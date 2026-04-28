#ifndef DWG_PARSER_H
#define DWG_PARSER_H

extern "C" {
#include <dwg_api.h>
}
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class DwgParser {
    Dwg_Data dwg_;
    bool loaded_;
    std::string filepath_;
public:
    DwgParser();
    ~DwgParser();

    DwgParser(const DwgParser&) = delete;
    DwgParser& operator=(const DwgParser&) = delete;

    bool open(const std::string& filepath);
    json extract_all();
    std::string get_version() const;
};

#endif
