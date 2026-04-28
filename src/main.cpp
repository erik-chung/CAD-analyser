#include "dwg_parser.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

namespace fs = std::filesystem;
using json = nlohmann::json;

#ifdef _WIN32
static std::string wide_to_utf8(const std::wstring& wide) {
    if (wide.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string u(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, &u[0], len, nullptr, nullptr);
    if (!u.empty() && u.back() == '\0') u.pop_back();
    return u;
}

static std::wstring utf8_to_wide(const std::string& utf8) {
    if (utf8.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    std::wstring w(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &w[0], len);
    if (!w.empty() && w.back() == L'\0') w.pop_back();
    return w;
}

static std::string acp_to_utf8(const char* s) {
    if (!s || !s[0]) return {};
    int wlen = MultiByteToWideChar(CP_ACP, 0, s, -1, nullptr, 0);
    std::wstring w(wlen, 0);
    MultiByteToWideChar(CP_ACP, 0, s, -1, &w[0], wlen);
    return wide_to_utf8(w);
}
#endif

static void print_usage(const char* prog) {
    std::cerr << "Usage:\n"
              << "  " << prog << " <input.dwg> [-o output.json]\n"
              << "  " << prog << " --dir <directory> [-o output_dir]\n"
              << "\nOptions:\n"
              << "  -o          Output file or directory\n"
              << "  --dir       Process all .dwg files in directory\n"
              << "  --pretty    Pretty-print JSON output (default)\n"
              << "  --compact   Compact JSON output\n"
              << "  -h, --help  Show this help\n";
}

static bool ends_with_dwg(const std::wstring& s) {
    if (s.size() < 4) return false;
    std::wstring ext = s.substr(s.size() - 4);
    return ext == L".dwg" || ext == L".DWG";
}

static bool process_file(const fs::path& input_path, const fs::path& output_path, bool pretty) {
    std::string input_str = wide_to_utf8(input_path.wstring());
    DwgParser parser;
    if (!parser.open(input_str)) {
        std::cerr << "Failed to open: " << input_str << std::endl;
        return false;
    }

    json result = parser.extract_all();

    std::cerr << "Parsed: " << input_str << "\n"
              << "  Layers: " << result["layers"].size()
              << "  Blocks: " << result["blocks"].size()
              << "  Texts: " << result["texts"].size()
              << "  Lines: " << result["geometry"]["lines"].size()
              << "  Circles: " << result["geometry"]["circles"].size()
              << "  Arcs: " << result["geometry"]["arcs"].size()
              << "  Polylines: " << result["geometry"]["polylines"].size()
              << "  Dimensions: " << result["dimensions"].size()
              << std::endl;

    if (output_path.empty()) {
        std::cout << (pretty ? result.dump(2) : result.dump()) << std::endl;
    } else {
        std::ofstream ofs(output_path.wstring().c_str());
        if (!ofs) {
            std::cerr << "Error: cannot write to " << wide_to_utf8(output_path.wstring()) << std::endl;
            return false;
        }
        ofs << (pretty ? result.dump(2) : result.dump()) << std::endl;
        std::cerr << "Output written to: " << wide_to_utf8(output_path.wstring()) << std::endl;
    }
    return true;
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // Get arguments as UTF-8 strings, handling both Windows codepage and UTF-8 inputs
    std::vector<std::string> args;
#ifdef _WIN32
    {
        int wargc;
        LPWSTR* wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
        if (wargv) {
            for (int i = 0; i < wargc; i++)
                args.push_back(wide_to_utf8(wargv[i]));
            LocalFree(wargv);
        }
    }
#else
    for (int i = 0; i < argc; i++)
        args.push_back(argv[i]);
#endif

    if (args.size() < 2) {
        print_usage(args.empty() ? "cad-analyser" : args[0].c_str());
        return 1;
    }

    std::string input;
    std::string output;
    bool dir_mode = false;
    bool pretty = true;

    for (size_t i = 1; i < args.size(); i++) {
        const std::string& arg = args[i];
        if (arg == "-h" || arg == "--help") {
            print_usage(args[0].c_str());
            return 0;
        } else if (arg == "--dir") {
            dir_mode = true;
            if (i + 1 < args.size()) input = args[++i];
        } else if (arg == "-o") {
            if (i + 1 < args.size()) output = args[++i];
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
        print_usage(args[0].c_str());
        return 1;
    }

    fs::path input_path(utf8_to_wide(input));

    if (dir_mode) {
        if (!fs::is_directory(input_path)) {
            std::cerr << "Error: not a directory: " << input << "\n";
            return 1;
        }
        fs::path out_dir = output.empty() ? fs::path(L"./output") : fs::path(utf8_to_wide(output));
        fs::create_directories(out_dir);

        int success = 0, fail = 0;
        for (auto& entry : fs::directory_iterator(input_path)) {
            if (!ends_with_dwg(entry.path().wstring())) continue;
            fs::path out_path = out_dir / (entry.path().stem().wstring() + L".json");
            if (process_file(entry.path(), out_path, pretty))
                success++;
            else
                fail++;
        }
        std::cerr << "\nDone: " << success << " succeeded, " << fail << " failed\n";
        return fail > 0 ? 1 : 0;
    } else {
        if (!fs::exists(input_path)) {
            std::cerr << "Error: file not found: " << input << "\n";
            return 1;
        }
        fs::path out_path = output.empty() ? fs::path() : fs::path(utf8_to_wide(output));
        return process_file(input_path, out_path, pretty) ? 0 : 1;
    }
}
