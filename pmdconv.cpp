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
int patch[MAX_PMD_PATCH];
void InitPatch()
{
	std::ifstream pf("patch.txt", std::ios::in);
	for (int i = 0; i < MAX_PMD_PATCH; i++)patch[i] = i % 128;
	if (!pf)
	{
		printf("未找到 patch.txt, 程序将以PMD音色作为GM音色处理。"
			"建议你创建该文件并在每行写入PMD到GM的对应音色，否则转换出来的MIDI效果可能不理想。\n");
	}
	else
	{
		int i = 0;
		while (i < MAX_PMD_PATCH)
		{
			pf >> patch[i];
			if (patch[i] > 127)printf("音色[%d]=%d, 超出最大值 127.\n", i, patch[i]);
			if (pf.eof())break;
		}
		printf("读取了%d个音色。\n", i);
	}
}
int Convert(const char *infile, const char *outfile)
{
	pmdwininit(".");
	setpcmrate(SOUND_44K);
	const int buf_length = 8192;
	short buf[buf_length];
	//TODO
	//普通音符直接获取每通道QQ中的信息即可，节奏部分采用R通道音符，可以获取到音量，Tom需要参考K通道鼓点。
	printf("TODO:程序正在制作中。\n如需获取程序更新，请关注 https://github.com/lxfly2000/pmdconv.");
	music_stop();
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