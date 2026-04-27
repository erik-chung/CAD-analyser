# CAD-analyser

DWG file parser that extracts layers, blocks, text annotations, geometry, and dimensions into structured JSON. Built with C++ and [LibreDWG](https://www.gnu.org/software/libredwg/).

## Prerequisites

**MSYS2 MinGW64** environment with:

```bash
pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-gcc mingw-w64-x86_64-pkg-config mingw-w64-x86_64-libredwg
```

The [nlohmann/json](https://github.com/nlohmann/json) header is vendored in `include/`.

## Build

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

## Usage

Parse a single DWG file:

```bash
./cad-analyser input.dwg -o output.json
```

Parse all DWG files in a directory:

```bash
./cad-analyser --dir ./drawings/ -o ./output/
```

Options:

| Flag | Description |
|------|-------------|
| `-o <path>` | Output file (single mode) or directory (dir mode) |
| `--dir <path>` | Process all .dwg files in directory |
| `--compact` | Compact JSON (no indentation) |
| `--pretty` | Pretty-print JSON (default) |

## JSON Output Schema

```json
{
  "metadata": { "filename": "...", "version": "AC1018", "units": 4 },
  "layers": [{ "name": "...", "color": 7, "linetype": "Continuous", "on": true, "frozen": false }],
  "blocks": [{ "name": "...", "insertion_point": [x,y,z], "rotation": 0, "scale": [1,1,1], "layer": "...", "attributes": [] }],
  "texts": [{ "type": "TEXT|MTEXT", "content": "...", "position": [x,y,z], "height": 2.5, "layer": "..." }],
  "geometry": {
    "lines": [{ "start": [x,y,z], "end": [x,y,z], "layer": "...", "color": 256 }],
    "circles": [{ "center": [x,y,z], "radius": 10.0, "layer": "...", "color": 1 }],
    "arcs": [{ "center": [x,y,z], "radius": 5.0, "start_angle": 0, "end_angle": 90, "layer": "..." }],
    "polylines": [{ "points": [[x,y],...], "closed": true, "layer": "..." }]
  },
  "dimensions": [{ "type": "linear", "measurement": 3500.0, "def_point": [x,y,z], "layer": "..." }]
}
```

## Supported Entities

- **Geometry**: LINE, CIRCLE, ARC, LWPOLYLINE, POLYLINE_2D
- **Text**: TEXT, MTEXT (with formatting code removal)
- **Blocks**: INSERT (with ATTRIB extraction)
- **Dimensions**: LINEAR, ALIGNED, ANGULAR, RADIUS, DIAMETER, ORDINATE
- **Tables**: LAYER (with linetype, color, on/frozen status)

## License

GPL-3.0 (due to LibreDWG dependency)
