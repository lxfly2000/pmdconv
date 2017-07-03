Add-Type -AssemblyName System.IO.Compression.FileSystem

function Unzip
{
	param([string]$zipfile, [string]$outpath)
	[System.IO.Compression.ZipFile]::ExtractToDirectory($zipfile, $outpath)
}

#更新midifile库到文件夹中
$webc = New-Object System.Net.WebClient
if(!(Test-Path "$PSScriptRoot\midifile.zip"))
{
	echo "下载 midifile..."
	$webc.DownloadFile("https://github.com/craigsapp/midifile/zipball/master","$PSScriptRoot\midifile.zip")
}
if(!(Test-Path "$PSScriptRoot\FMPMD_SDK*.zip"))
{
	echo "下载 FMPMD_SDK..."
	$webc.DownloadFile("http://c60.la.coocan.jp/download/FMPMD_SDK004.zip","$PSScriptRoot\FMPMD_SDK004.zip")
}

if(!(Get-ChildItem -Recurse -Path midifile))
{
	rd midifile
}
if(!(Test-Path "midifile"))
{
	Unzip "midifile.zip" "."
	move "*midifile-*" "midifile"
}
if(!(Test-Path "PMDWin"))
{
	Unzip "FMPMD_SDK004.zip" "."
	ren "PMDWin\PMDWinImort.h" "PMDWinImport.h"
}

#查找MSBuild
$path = &"${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
$path = join-path $path 'MSBuild\15.0\Bin\MSBuild.exe'

#构建Lib
&$path midifile\visual-studio\midifile.vcxproj "/p:Configuration=Debug;Platform=Win32;PlatformToolset=v141;WindowsTargetPlatformVersion=10.0.14393.0"
&$path midifile\visual-studio\midifile.vcxproj "/p:Configuration=Release;Platform=Win32;PlatformToolset=v141;WindowsTargetPlatformVersion=10.0.14393.0"

#构建主程序
&$path pmdconv.vcxproj "/p:Configuration=Release;Platform=Win32"