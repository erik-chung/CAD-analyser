#include "json_builder.h"

JsonBuilder::JsonBuilder() {
    result_["metadata"] = json::object();
    result_["layers"] = json::array();
    result_["blocks"] = json::array();
    result_["texts"] = json::array();
    result_["geometry"]["lines"] = json::array();
    result_["geometry"]["circles"] = json::array();
    result_["geometry"]["arcs"] = json::array();
    result_["geometry"]["polylines"] = json::array();
    result_["dimensions"] = json::array();
}

void JsonBuilder::set_metadata(const std::string& filename, const std::string& version, int units) {
    result_["metadata"]["filename"] = filename;
    result_["metadata"]["version"] = version;
    result_["metadata"]["units"] = units;
}

void JsonBuilder::add_layer(const json& layer) { result_["layers"].push_back(layer); }
void JsonBuilder::add_block(const json& block) { result_["blocks"].push_back(block); }
void JsonBuilder::add_text(const json& text) { result_["texts"].push_back(text); }
void JsonBuilder::add_line(const json& line) { result_["geometry"]["lines"].push_back(line); }
void JsonBuilder::add_circle(const json& circle) { result_["geometry"]["circles"].push_back(circle); }
void JsonBuilder::add_arc(const json& arc) { result_["geometry"]["arcs"].push_back(arc); }
void JsonBuilder::add_polyline(const json& polyline) { result_["geometry"]["polylines"].push_back(polyline); }
void JsonBuilder::add_dimension(const json& dim) { result_["dimensions"].push_back(dim); }

json JsonBuilder::build() const { return result_; }
