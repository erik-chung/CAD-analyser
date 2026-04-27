#include "dwg_parser.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;
using json = nlohmann::json;

static void print_usage(const char* prog) {
    std::cerr << "Usage:\n"
              << "  " << prog << " <input.dwg> [-o output.json]\n"
              << "  " << prog << " --dir <directory> [-o output_dir]\n"
              << "\nOptions:\n"
              << "  -o          Output file or directory (default: stdout / ./output/)\n"
              << "  --dir       Process all .dwg files in directory\n"
              << "  --pretty    Pretty-print JSON output (default)\n"
              << "  --compact   Compact JSON output\n"
              << "  -h, --help  Show this help\n";
}

static bool ends_with_dwg(const std::string& s) {
    if (s.size() < 4) return false;
    std::string ext = s.substr(s.size() - 4);
    return ext == ".dwg" || ext == ".DWG";
}

static bool process_file(const std::string& input, const std::string& output, bool pretty) {
    DwgParser parser;
    if (!parser.open(input)) {
        std::cerr << "Failed to open: " << input << std::endl;
        return false;
    }

    json result = parser.extract_all();

    // Stats summary to stderr
    std::cerr << "Parsed: " << input << "\n"
              << "  Layers: " << result["layers"].size()
              << "  Blocks: " << result["blocks"].size()
              << "  Texts: " << result["texts"].size()
              << "  Lines: " << result["geometry"]["lines"].size()
              << "  Circles: " << result["geometry"]["circles"].size()
              << "  Arcs: " << result["geometry"]["arcs"].size()
              << "  Polylines: " << result["geometry"]["polylines"].size()
              << "  Dimensions: " << result["dimensions"].size()
              << std::endl;

    if (output.empty()) {
        std::cout << (pretty ? result.dump(2) : result.dump()) << std::endl;
    } else {
        std::ofstream ofs(output);
        if (!ofs) {
            std::cerr << "Error: cannot write to " << output << std::endl;
            return false;
        }
        ofs << (pretty ? result.dump(2) : result.dump()) << std::endl;
        std::cerr << "Output written to: " << output << std::endl;
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string input;
    std::string output;
    bool dir_mode = false;
    bool pretty = true;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--dir") {
            dir_mode = true;
            if (i + 1 < argc) input = argv[++i];
        } else if (arg == "-o") {
            if (i + 1 < argc) output = argv[++i];
        } else if (arg == "--compact") {
            pretty = false;
        } else if (arg == "--pretty") {
            pretty = true;
        } else if (input.empty()) {
            input = arg;
        }
    }

    if (input.empty()) {
        std::cerr << "Error: no input specified\n";
        print_usage(argv[0]);
        return 1;
    }

    if (dir_mode) {
        if (!fs::is_directory(input)) {
            std::cerr << "Error: " << input << " is not a directory\n";
            return 1;
        }
        std::string out_dir = output.empty() ? "./output" : output;
        fs::create_directories(out_dir);

        int success = 0, fail = 0;
        for (auto& entry : fs::directory_iterator(input)) {
            std::string path = entry.path().string();
            if (!ends_with_dwg(path)) continue;

            std::string out_name = entry.path().stem().string() + ".json";
            std::string out_path = (fs::path(out_dir) / out_name).string();

            if (process_file(path, out_path, pretty))
                success++;
            else
                fail++;
        }
        std::cerr << "\nDone: " << success << " succeeded, " << fail << " failed\n";
        return fail > 0 ? 1 : 0;
    } else {
        if (!fs::exists(input)) {
            std::cerr << "Error: file not found: " << input << "\n";
            return 1;
        }
        return process_file(input, output, pretty) ? 0 : 1;
    }
}
