//pmd参考：https://github.com/mistydemeo/pmdmini
//midifile参考：https://github.com/craigsapp/midifile
#ifdef _DEBUG
#pragma comment(lib,"midifile\\visual-studio\\DebugLib\\midifile.lib")
#pragma comment(lib,"pmdplay\\Debug\\pmdplay.lib")
#else
#pragma comment(lib,"midifile\\visual-studio\\ReleaseLib\\midifile.lib")
#pragma comment(lib,"pmdplay\\Release\\pmdplay.lib")
#endif
#include"midifile\include\MidiFile.h"
#include"pmdplay\pmdmini.h"
#include<iostream>
#include<fstream>
int Convert(const char *infile, const char *outfile)
{
	//TODO
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
	return Convert(infile, outfile);
}