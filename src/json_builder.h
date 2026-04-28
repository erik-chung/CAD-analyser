#ifndef JSON_BUILDER_H
#define JSON_BUILDER_H

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class JsonBuilder {
    json result_;
public:
    JsonBuilder();
    void set_metadata(const std::string& filename, const std::string& version, int units);
    void add_layer(const json& layer);
    void add_block(const json& block);
    void add_text(const json& text);
    void add_line(const json& line);
    void add_circle(const json& circle);
    void add_arc(const json& arc);
    void add_polyline(const json& polyline);
    void add_dimension(const json& dim);
    json build() const;
};

#endif
