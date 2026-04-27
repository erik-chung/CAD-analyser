# CAD-analyser

基于 C++ 和 [LibreDWG](https://www.gnu.org/software/libredwg/) 的 DWG 文件解析工具，可从 AutoCAD DWG 文件中提取图层、图块、文字标注、几何数据和尺寸标注，输出为结构化 JSON。

## 环境要求

**MSYS2 MinGW64** 环境，安装以下依赖：

```bash
pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-gcc mingw-w64-x86_64-pkg-config mingw-w64-x86_64-libredwg
```

[nlohmann/json](https://github.com/nlohmann/json) 头文件已内嵌在 `include/` 目录中，无需额外安装。

## 编译

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

## 使用方式

解析单个 DWG 文件：

```bash
./cad-analyser input.dwg -o output.json
```

批量解析目录下所有 DWG 文件：

```bash
./cad-analyser --dir ./drawings/ -o ./output/
```

命令行参数：

| 参数 | 说明 |
|------|------|
| `-o <路径>` | 输出文件（单文件模式）或输出目录（目录模式） |
| `--dir <路径>` | 批量处理指定目录下所有 .dwg 文件 |
| `--compact` | 紧凑 JSON 输出（无缩进） |
| `--pretty` | 格式化 JSON 输出（默认） |

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

AC1018（AutoCAD 2004）格式的 DWG 文件使用 GBK（CP936）编码存储中文文本。本工具自动将 GBK 编码转换为 UTF-8 输出，确保 JSON 中的中文内容正确显示。

## 许可证

GPL-3.0（因依赖 LibreDWG）
