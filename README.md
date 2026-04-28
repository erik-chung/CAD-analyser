# CAD-analyser

基于 C++ 和 [LibreDWG](https://www.gnu.org/software/libredwg/) 的 DWG 文件解析工具，可从 AutoCAD DWG 文件中提取图层、图块、文字标注、几何数据和尺寸标注，输出为结构化 JSON。

## 快速开始

`bin/` 目录包含预编译的 Windows x64 可执行文件及所有运行时依赖，可直接在 cmd 或 PowerShell 中运行：

```cmd
cd bin
cad-analyser.exe input.dwg -o output.json
```

### 运行时文件说明

| 文件 | 说明 |
|------|------|
| `cad-analyser.exe` | 主程序 |
| `libredwg.dll` | LibreDWG DWG 解析库 |
| `libstdc++-6.dll` | GCC C++ 标准库 |
| `libgcc_s_seh-1.dll` | GCC 异常处理运行时 |
| `libwinpthread-1.dll` | POSIX 线程库 |
| `libiconv-2.dll` | 字符编码转换库 |

## 使用方式

解析单个 DWG 文件：

```cmd
cad-analyser.exe input.dwg -o output.json
```

批量解析目录下所有 DWG 文件：

```cmd
cad-analyser.exe --dir .\drawings\ -o .\output\
```

命令行参数：

| 参数 | 说明 |
|------|------|
| `-o <路径>` | 输出文件（单文件模式）或输出目录（目录模式） |
| `--dir <路径>` | 批量处理指定目录下所有 .dwg 文件 |
| `--compact` | 紧凑 JSON 输出（无缩进） |
| `--pretty` | 格式化 JSON 输出（默认） |
| `-h, --help` | 显示帮助 |

## 从源码编译

### 环境要求

**MSYS2 MinGW64** 环境，安装以下依赖：

```bash
pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-gcc mingw-w64-x86_64-ninja mingw-w64-x86_64-libredwg
```

[nlohmann/json](https://github.com/nlohmann/json) 头文件已内嵌在 `include/` 目录中，无需额外安装。

### 编译

```bash
mkdir build && cd build
cmake .. -G Ninja
ninja
```

编译完成后，需将以下 DLL 复制到可执行文件同目录：
- `/mingw64/bin/libstdc++-6.dll`
- `/mingw64/bin/libwinpthread-1.dll`

## JSON 输出格式

```json
{
  "metadata": {
    "filename": "01平面布置图.dwg",
    "version": "AC1018",
    "units": 4
  },
  "layers": [
    { "name": "A-WALL", "color": 7, "linetype": "Continuous", "on": true, "frozen": false }
  ],
  "blocks": [
    {
      "name": "CAMERA-PTZ",
      "insertion_point": [100.0, 200.0, 0.0],
      "rotation": 0.0,
      "scale": [1.0, 1.0, 1.0],
      "layer": "S-EQUIP",
      "attributes": [{ "tag": "NUM", "value": "C01" }]
    }
  ],
  "texts": [
    {
      "type": "TEXT",
      "content": "入口",
      "position": [50.0, 60.0, 0.0],
      "height": 2.5,
      "rotation": 0.0,
      "layer": "A-ANNO",
      "style": "Standard"
    }
  ],
  "geometry": {
    "lines": [{ "start": [0,0,0], "end": [100,0,0], "layer": "A-WALL", "color": 256 }],
    "circles": [{ "center": [50,50,0], "radius": 10.0, "layer": "A-SYMB", "color": 1 }],
    "arcs": [{ "center": [0,0,0], "radius": 5.0, "start_angle": 0.0, "end_angle": 90.0, "layer": "A-WALL" }],
    "polylines": [{ "points": [[0,0],[10,0],[10,10]], "closed": true, "layer": "A-WALL" }]
  },
  "dimensions": [
    { "type": "linear", "measurement": 3500.0, "def_point": [100,200,0], "layer": "A-DIM" }
  ]
}
```

## 支持的实体类型

| 类别 | 实体类型 |
|------|----------|
| 几何图形 | LINE、CIRCLE、ARC、LWPOLYLINE、POLYLINE_2D |
| 文字标注 | TEXT、MTEXT（自动去除 MTEXT 格式控制码） |
| 图块引用 | INSERT（含属性 ATTRIB 提取） |
| 尺寸标注 | LINEAR、ALIGNED、ANGULAR、RADIUS、DIAMETER、ORDINATE |
| 图层表 | LAYER（含线型、颜色、开关、冻结状态） |

## 中文支持

- **GBK 文本**：AC1018（AutoCAD 2004）格式使用 GBK（CP936）编码存储中文文本，本工具自动转换为 UTF-8 输出
- **中文路径**：完整支持中文文件名和目录名（通过 Windows Unicode API）
- **MTEXT 格式清理**：自动去除字体、颜色、对齐、段落等格式控制码，输出纯文本

## 测试结果

使用 4 个真实银行安防图纸 DWG 文件测试：

| 文件 | 图层 | 图块 | 文字 | 线段 | 圆 | 弧 | 折线 | 尺寸 |
|------|------|------|------|------|---|---|------|------|
| 01平面布置图.dwg | 141 | 1184 | 2085 | 3611 | 98 | 203 | 775 | 239 |
| 02立面图.dwg | 124 | 1957 | 1566 | 18233 | 1015 | 184 | 1882 | 484 |
| 03详图.dwg | 256 | 2857 | 2947 | 25193 | 793 | 2110 | 5668 | 955 |
| 04水电图.dwg | 66 | 485 | 1305 | 1573 | 93 | 135 | 773 | 44 |

## 许可证

GPL-3.0（因依赖 LibreDWG）
