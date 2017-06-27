//pmd参考：https://github.com/mistydemeo/pmdmini
//midifile参考：https://github.com/craigsapp/midifile
#ifdef _DEBUG
#pragma comment(lib,"midifile\\visual-studio\\DebugLib\\midifile.lib")
#pragma comment(lib,"pmdplay\\Debug\\pmdplay.lib")
#else
#pragma comment(lib,"midifile\\visual-studio\\ReleaseLib\\midifile.lib")
#pragma comment(lib,"pmdplay\\Release\\pmdplay.lib")
#endif
#define _WINDOWS
#include"midifile\include\MidiFile.h"
#include"pmdplay\pmdwin\pmdwinimport.h"
#include<iostream>
#include<fstream>
#define MAX_PMD_PATCH 256
#define MAX_PMD_RHYTHM_NUM 11
#define PITCH_BEND_RANGE_HALFTONE 12
int g_patch[MAX_PMD_PATCH];
void InitPatch()
{
	std::ifstream pf("patch.txt", std::ios::in);
	for (int i = 0; i < MAX_PMD_PATCH; i++)g_patch[i] = i % 128;
	if (!pf)
	{
		printf("未找到 patch.txt, 程序将以PMD音色作为GM音色处理。"
			"建议你创建该文件并在每行写入PMD到GM的对应音色，否则转换出来的MIDI效果可能不理想。\n");
	}
	else
	{
		int i = 0;
		std::string input;
		while (i < MAX_PMD_PATCH)
		{
			std::getline(pf, input);
			if (input[0] == ';')continue;
			g_patch[i] = atoi(input.c_str());
			if (g_patch[i] > 127)printf("音色[%d]=%d, 超出最大值 127.\n", i, g_patch[i]);
			i++;
			if (pf.eof())break;
		}
		printf("读取了%d个音色。\n", i);
	}
}
struct ChannelNote
{
	int note = 0, last_note = 0;
	int volume = 0;//FM最大127，SSG最大15
	int patch = 0;
	int ch = 0;
	int pitchbend = 0;
	bool keyison = false;
	void UpdateChannel()
	{
		volume = getpartwork(ch)->volume;
		last_patch = patch;
		patch = getpartwork(ch)->voicenum;
		last_note = note;
		note = (getpartwork(ch)->onkai & 0xF) + ((getpartwork(ch)->onkai >> 4) * 12);
		last_key_on_count = key_on_count;
		key_on_count = getpartwork(ch)->keyon_flag;
		last_pitchbend = pitchbend;
		pitchbend = getpartwork(ch)->porta_num;//porta_num表示弯多少半音，每50为一个半音，正数表示向上弯，负数向下弯
		if (!lfo_found&&getpartwork(ch)->lfoswi)
		{
			lfo_found = true;
			printf("检测到 Ch %c 含有 LFO，请手动在转换后的 MIDI 中添加相应参数。（CC#1等）\n", 'A' + ch);
		}
	}
	bool IsOnNoteOn()
	{
		return (key_on_count != last_key_on_count) || (note < 128 && last_note >= 128);
	}
	bool IsOnNoteOff()
	{
		return (getpartwork(ch)->keyoff_flag != 0) || (note >= 128 && last_note < 128);
	}
	bool IsOnProgramChange()
	{
		return last_patch != patch;
	}
	bool IsOnPitchBend()
	{
		return last_pitchbend != pitchbend;
	}
	int GetVolumeInGM()
	{
		if (ch < 6)return volume;
		else return 127 * volume / 15;
	}
private:
	int last_key_on_count = 0;
	int last_patch = 0;
	int key_on_count = 0;
	int last_pitchbend = 0;
	bool lfo_found = false;
};
struct ChannelRhythm
{
	bool notes_on[MAX_PMD_RHYTHM_NUM] = { false }, last_notes_on[MAX_PMD_RHYTHM_NUM] = { false };
	bool rhythm_started = false;
	const int krhythmToGMNote[MAX_PMD_RHYTHM_NUM] = { 36,38,45,47,50,37,40,42,46,49,51 };
	int volume = 0;//最大63
	void UpdateChannel()
	{
		last_volume = volume;
		volume = getopenwork()->rhyvol;
		last_rhyaddr = rhyaddr;
		rhyaddr = getopenwork()->rhyadr;
		if (!rhythm_started&&getopenwork()->kshot_dat)
		{
			rhythm_started = true;
			printf("注意：由于 PMDWin 的限制，本程序无法保证转换出来的节奏完全正确。\n");
		}
		for (int i = 0; i < MAX_PMD_RHYTHM_NUM; i++)
		{
			last_notes_on[i] = notes_on[i];
			notes_on[i] = (getopenwork()->kshot_dat >> i) & 1;
		}
	}
	bool IsOnVolumeChange()
	{
		return (last_volume != volume) && rhythm_started;
	}
	bool IsOnRhythmUpdate()
	{
		return last_rhyaddr != rhyaddr;
	}
	int GetVolumeInGM()
	{
		return volume * 127 / 63;
	}
private:
	int last_volume = 0;
	uchar *rhyaddr = NULL, *last_rhyaddr = NULL;
};
struct Channels
{
	ChannelNote chnote[9];
	ChannelRhythm chrhy;
	Channels()
	{
		for (int i = 0; i < 9; i++)chnote[i].ch = i;
	}
	void Update()
	{
		for (int i = 0; i < 9; i++)chnote[i].UpdateChannel();
		chrhy.UpdateChannel();
		last_tempo = tempo;
		tempo = getopenwork()->tempo_48 * 2;
	}
	int tempo = 120;
	bool IsTempoChanged()
	{
		return last_tempo != tempo;
	}
private:
	int last_tempo = 120;
};
int Convert(const char *infile, const char *outfile)
{
	pmdwininit(".");
	setppsuse(false);
	setrhythmwithssgeffect(true);
	getopenwork()->effflag = 0;
	setfmcalc55k(true);
	setpcmrate(SOUND_44K);
	char musicfilepath[_MAX_PATH];
	strcpy(musicfilepath, infile);
	if (music_load(musicfilepath))
	{
		printf("加载文件 %s 时出错，请检查该文件是否是有效的 PMD 文件。\n", musicfilepath);
		return -1;
	}
	music_start();
	int tpq = getopenwork()->syousetu_lng / 4;
	MidiFile mf;
	Channels pmdch;
	char renbuf[SOUND_44K * 2 * 2];//一秒音频的内存长度
	mf.addTrack(10);
	mf.setTicksPerQuarterNote(tpq);
	for (int i = 0; i < 9; i++)
	{
		char tname[32];
		sprintf(tname, "PMD Ch %c", 'A' + i);
		mf.addTrackName(i + 1, 0, tname);
	}
	mf.addTrackName(10, 0, "PMD Rhythm");
	int _nowtick = getpos2();
	int lengthtick = 0, looplengthtick = 0;
	int offset_tick = 0;
	bool firstkey = true;
#define nowtick max(0,_nowtick-offset_tick)
	getlength2(musicfilepath, &lengthtick, &looplengthtick);
	for (int i = 6; i <= 8; i++)mf.addPatchChange(i + 1, 0, i, 80);
	for (int i = 0; i < 9; i++)
	{
		mf.addController(i + 1, 0, i, 0x65, 0);//设置RPN的高位为弯音
		mf.addController(i + 1, 0, i, 0x64, 0);//设置RPN的低位为弯音
		mf.addController(i + 1, 0, i, 0x06, PITCH_BEND_RANGE_HALFTONE);//将弯音范围设置为12个半音（即一个八度）
		mf.addController(i + 1, 0, i, 0x26, 0);//低位没用到，给0就行了
	}
	while (nowtick < lengthtick)
	{
		pmdch.Update();
		if (pmdch.IsTempoChanged())mf.addTempo(0, nowtick, pmdch.tempo);
		for (int i = 0; i < 9; i++)
		{
			if (pmdch.chnote[i].IsOnNoteOn())
			{
				if (firstkey)
				{
					firstkey = false;
					offset_tick = _nowtick;
				}
				if (pmdch.chnote[i].keyison)mf.addNoteOff(i + 1, nowtick, i, pmdch.chnote[i].last_note);
				pmdch.chnote[i].keyison = true;
				if (pmdch.chnote[i].note < 128)
					mf.addNoteOn(i + 1, nowtick, i, pmdch.chnote[i].note, pmdch.chnote[i].GetVolumeInGM());
			}
			if (pmdch.chnote[i].IsOnNoteOff())
			{
				pmdch.chnote[i].keyison = false;
				if (pmdch.chnote[i].last_note < 128)
					mf.addNoteOff(i + 1, nowtick, i, pmdch.chnote[i].last_note);
			}
			if (pmdch.chnote[i].IsOnProgramChange())mf.addPatchChange(i + 1, nowtick, i, g_patch[pmdch.chnote[i].patch]);
			if (pmdch.chnote[i].IsOnPitchBend())mf.addPitchBend(i + 1, nowtick, i, pmdch.chnote[i].pitchbend/(PITCH_BEND_RANGE_HALFTONE*50.0));
		}
		if (pmdch.chrhy.IsOnRhythmUpdate())
		{
			for (int i = 0; i < MAX_PMD_RHYTHM_NUM; i++)
			{
				if (pmdch.chrhy.last_notes_on[i])
					mf.addNoteOff(10, nowtick, 9, pmdch.chrhy.krhythmToGMNote[i]);
				if (pmdch.chrhy.notes_on[i])
					mf.addNoteOn(10, nowtick, 9, pmdch.chrhy.krhythmToGMNote[i], pmdch.chrhy.GetVolumeInGM());
			}
		}
		getpcmdata((short*)renbuf, min(SOUND_44K, SOUND_44K * 60 / tpq / pmdch.tempo));
		_nowtick = getpos2();
	}
	for (int i = 0; i < 9; i++)
		if (pmdch.chnote[i].keyison)
			mf.addNoteOff(i + 1, nowtick, i, pmdch.chnote[i].note);
	music_stop();
	mf.sortTracks();
	mf.write(outfile);
	return 0;
}
int main(int argc, char *argv[])
{
	char outfile[_MAX_PATH];
	const char *infile = argv[1];
	sprintf(outfile, "%s.mid", infile);
	switch (argc)
	{
	case 3:strcpy(outfile, argv[2]);
	case 2:
		break;
	default:
		printf("参数错误。\npmdconv <输入文件名> [输出文件名=<输入文件名>.mid]\n");
		return 1;
	}
	InitPatch();
	return Convert(infile, outfile);
}