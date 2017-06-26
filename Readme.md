# pmdconv
PMD 到 MIDI 的转换工具。

## 使用方法
`pmdconv <输入：PMD文件> [输出：MIDI文件]`

## 编译
请用 VS2017 编译，本程序需要 [midifile](https://github.com/craigsapp/midifile) 和 [pmdplay](https://github.com/lxfly2000/pmdplay) 来工作，可通过运行 `fetchlib.ps1` 获得，或者直接运行该脚本完成全部构建过程。

### 提示
使用本程序前请先在程序目录中创建一个 `patch.txt` 文件，在里面写上 PMD 到 GM 的音色转换，一共 256 行，每行写对应的 GM 音色号即可。

## 注意
* 本程序转换的 MIDI 中的节奏是根据 PMD 节奏得到的，而非 YM2608 的节奏。
* 由于 PMDWin 的限制，本程序无法完全准确转换节奏部分，你可能需要根据原 PMD 文件手动修改转换后的 MIDI 文件。