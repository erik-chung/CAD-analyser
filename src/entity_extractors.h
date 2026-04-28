#ifndef ENTITY_EXTRACTORS_H
#define ENTITY_EXTRACTORS_H

extern "C" {
#include <dwg_api.h>
}
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::string get_layer_name(Dwg_Object_Entity* ent);
int get_entity_color(Dwg_Object_Entity* ent);

json extract_layer(Dwg_Object* obj);
json extract_line(Dwg_Object* obj);
json extract_circle(Dwg_Object* obj);
json extract_arc(Dwg_Object* obj);
json extract_lwpolyline(Dwg_Object* obj);
json extract_polyline2d(Dwg_Object* obj);
json extract_text(Dwg_Object* obj);
json extract_mtext(Dwg_Object* obj);
json extract_insert(Dwg_Object* obj);
json extract_dimension(Dwg_Object* obj);

#endif
