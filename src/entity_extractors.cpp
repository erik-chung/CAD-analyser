#include "entity_extractors.h"
#include "text_utils.h"
#include <cstring>

static json point3d(double x, double y, double z) {
    return json::array({x, y, z});
}

static json point2d(double x, double y) {
    return json::array({x, y});
}

static std::string safe_str(const char* s) {
    return s ? gbk_to_utf8(s) : "";
}

std::string get_layer_name(Dwg_Object_Entity* ent) {
    if (!ent || !ent->layer || !ent->layer->obj)
        return "0";
    Dwg_Object* layer_obj = ent->layer->obj;
    if (layer_obj->type != DWG_TYPE_LAYER)
        return "0";
    Dwg_Object_LAYER* layer = layer_obj->tio.object->tio.LAYER;
    if (!layer || !layer->name)
        return "0";
    return safe_str(layer->name);
}

int get_entity_color(Dwg_Object_Entity* ent) {
    if (!ent) return 256;
    return ent->color.index;
}

json extract_layer(Dwg_Object* obj) {
    if (!obj || obj->type != DWG_TYPE_LAYER) return nullptr;
    Dwg_Object_LAYER* layer = obj->tio.object->tio.LAYER;
    if (!layer) return nullptr;
    json j;
    j["name"] = safe_str(layer->name);
    j["color"] = layer->color.index;
    j["on"] = !layer->off;
    j["frozen"] = layer->frozen != 0;
    j["linetype"] = "";
    if (layer->ltype && layer->ltype->obj) {
        Dwg_Object* lt_obj = layer->ltype->obj;
        if (lt_obj->type == DWG_TYPE_LTYPE) {
            Dwg_Object_LTYPE* ltype = lt_obj->tio.object->tio.LTYPE;
            if (ltype && ltype->name)
                j["linetype"] = safe_str(ltype->name);
        }
    }
    return j;
}

json extract_line(Dwg_Object* obj) {
    if (!obj) return nullptr;
    Dwg_Entity_LINE* line = obj->tio.entity->tio.LINE;
    if (!line) return nullptr;
    json j;
    j["start"] = point3d(line->start.x, line->start.y, line->start.z);
    j["end"] = point3d(line->end.x, line->end.y, line->end.z);
    j["layer"] = get_layer_name(obj->tio.entity);
    j["color"] = get_entity_color(obj->tio.entity);
    return j;
}

json extract_circle(Dwg_Object* obj) {
    if (!obj) return nullptr;
    Dwg_Entity_CIRCLE* circle = obj->tio.entity->tio.CIRCLE;
    if (!circle) return nullptr;
    json j;
    j["center"] = point3d(circle->center.x, circle->center.y, circle->center.z);
    j["radius"] = circle->radius;
    j["layer"] = get_layer_name(obj->tio.entity);
    j["color"] = get_entity_color(obj->tio.entity);
    return j;
}

json extract_arc(Dwg_Object* obj) {
    if (!obj) return nullptr;
    Dwg_Entity_ARC* arc = obj->tio.entity->tio.ARC;
    if (!arc) return nullptr;
    json j;
    j["center"] = point3d(arc->center.x, arc->center.y, arc->center.z);
    j["radius"] = arc->radius;
    j["start_angle"] = arc->start_angle;
    j["end_angle"] = arc->end_angle;
    j["layer"] = get_layer_name(obj->tio.entity);
    j["color"] = get_entity_color(obj->tio.entity);
    return j;
}

json extract_lwpolyline(Dwg_Object* obj) {
    if (!obj) return nullptr;
    Dwg_Entity_LWPOLYLINE* pline = obj->tio.entity->tio.LWPOLYLINE;
    if (!pline) return nullptr;
    json j;
    j["closed"] = (pline->flag & 512) != 0;
    j["layer"] = get_layer_name(obj->tio.entity);
    j["color"] = get_entity_color(obj->tio.entity);
    json pts = json::array();
    for (BITCODE_BL i = 0; i < pline->num_points; i++) {
        pts.push_back(point2d(pline->points[i].x, pline->points[i].y));
    }
    j["points"] = pts;
    return j;
}

json extract_polyline2d(Dwg_Object* obj) {
    if (!obj) return nullptr;
    Dwg_Entity_POLYLINE_2D* pline = obj->tio.entity->tio.POLYLINE_2D;
    if (!pline) return nullptr;
    json j;
    j["closed"] = (pline->flag & 1) != 0;
    j["layer"] = get_layer_name(obj->tio.entity);
    j["color"] = get_entity_color(obj->tio.entity);
    // POLYLINE_2D vertices are separate VERTEX_2D entities linked via owned handles
    // We collect them during traversal in DwgParser
    j["points"] = json::array();
    return j;
}

json extract_text(Dwg_Object* obj) {
    if (!obj) return nullptr;
    Dwg_Entity_TEXT* text = obj->tio.entity->tio.TEXT;
    if (!text) return nullptr;
    json j;
    j["type"] = "TEXT";
    j["content"] = safe_str(text->text_value);
    j["position"] = point3d(text->ins_pt.x, text->ins_pt.y, 0.0);
    j["height"] = text->height;
    j["rotation"] = text->rotation;
    j["layer"] = get_layer_name(obj->tio.entity);
    // Text style
    j["style"] = "";
    if (text->style && text->style->obj) {
        Dwg_Object* st_obj = text->style->obj;
        if (st_obj->type == DWG_TYPE_STYLE) {
            Dwg_Object_STYLE* style = st_obj->tio.object->tio.STYLE;
            if (style && style->name)
                j["style"] = safe_str(style->name);
        }
    }
    return j;
}

json extract_mtext(Dwg_Object* obj) {
    if (!obj) return nullptr;
    Dwg_Entity_MTEXT* mtext = obj->tio.entity->tio.MTEXT;
    if (!mtext) return nullptr;
    json j;
    j["type"] = "MTEXT";
    std::string raw = safe_str(mtext->text);
    j["content"] = sanitize_mtext(raw);
    j["raw_content"] = raw;
    j["position"] = point3d(mtext->ins_pt.x, mtext->ins_pt.y, mtext->ins_pt.z);
    j["height"] = mtext->text_height;
    j["x_axis_dir"] = point3d(mtext->x_axis_dir.x, mtext->x_axis_dir.y, mtext->x_axis_dir.z);
    j["layer"] = get_layer_name(obj->tio.entity);
    return j;
}

json extract_insert(Dwg_Object* obj) {
    if (!obj) return nullptr;
    Dwg_Entity_INSERT* insert = obj->tio.entity->tio.INSERT;
    if (!insert) return nullptr;
    json j;
    // Block name from block_header
    j["name"] = "";
    if (insert->block_header && insert->block_header->obj) {
        Dwg_Object* bh_obj = insert->block_header->obj;
        if (bh_obj->type == DWG_TYPE_BLOCK_HEADER) {
            Dwg_Object_BLOCK_HEADER* bh = bh_obj->tio.object->tio.BLOCK_HEADER;
            if (bh && bh->name)
                j["name"] = safe_str(bh->name);
        }
    }
    j["insertion_point"] = point3d(insert->ins_pt.x, insert->ins_pt.y, insert->ins_pt.z);
    j["rotation"] = insert->rotation;
    j["scale"] = json::array({insert->scale.x, insert->scale.y, insert->scale.z});
    j["layer"] = get_layer_name(obj->tio.entity);

    // Extract block attributes
    json attrs = json::array();
    if (insert->has_attribs && insert->num_owned > 0) {
        for (BITCODE_BL i = 0; i < insert->num_owned; i++) {
            if (!insert->attribs || !insert->attribs[i] || !insert->attribs[i]->obj)
                continue;
            Dwg_Object* att_obj = insert->attribs[i]->obj;
            if (att_obj->fixedtype != DWG_TYPE_ATTRIB)
                continue;
            Dwg_Entity_ATTRIB* attrib = att_obj->tio.entity->tio.ATTRIB;
            if (!attrib) continue;
            json attr;
            attr["tag"] = safe_str(attrib->tag);
            attr["value"] = safe_str(attrib->text_value);
            attrs.push_back(attr);
        }
    }
    j["attributes"] = attrs;
    return j;
}

json extract_dimension(Dwg_Object* obj) {
    if (!obj) return nullptr;
    Dwg_Object_Entity* ent = obj->tio.entity;
    if (!ent) return nullptr;

    json j;
    j["layer"] = get_layer_name(ent);
    j["color"] = get_entity_color(ent);

    switch (obj->fixedtype) {
    case DWG_TYPE_DIMENSION_LINEAR: {
        Dwg_Entity_DIMENSION_LINEAR* dim = ent->tio.DIMENSION_LINEAR;
        if (!dim) return nullptr;
        j["type"] = "linear";
        j["measurement"] = dim->act_measurement;
        j["def_point"] = point3d(dim->def_pt.x, dim->def_pt.y, dim->def_pt.z);
        j["text_midpoint"] = point2d(dim->text_midpt.x, dim->text_midpt.y);
        j["user_text"] = safe_str(dim->user_text);
        j["rotation"] = dim->dim_rotation;
        break;
    }
    case DWG_TYPE_DIMENSION_ALIGNED: {
        Dwg_Entity_DIMENSION_ALIGNED* dim = ent->tio.DIMENSION_ALIGNED;
        if (!dim) return nullptr;
        j["type"] = "aligned";
        j["measurement"] = dim->act_measurement;
        j["def_point"] = point3d(dim->def_pt.x, dim->def_pt.y, dim->def_pt.z);
        j["text_midpoint"] = point2d(dim->text_midpt.x, dim->text_midpt.y);
        j["user_text"] = safe_str(dim->user_text);
        break;
    }
    case DWG_TYPE_DIMENSION_ANG2LN: {
        Dwg_Entity_DIMENSION_ANG2LN* dim = ent->tio.DIMENSION_ANG2LN;
        if (!dim) return nullptr;
        j["type"] = "angular_2line";
        j["measurement"] = dim->act_measurement;
        j["def_point"] = point3d(dim->def_pt.x, dim->def_pt.y, dim->def_pt.z);
        j["user_text"] = safe_str(dim->user_text);
        break;
    }
    case DWG_TYPE_DIMENSION_ANG3PT: {
        Dwg_Entity_DIMENSION_ANG3PT* dim = ent->tio.DIMENSION_ANG3PT;
        if (!dim) return nullptr;
        j["type"] = "angular_3point";
        j["measurement"] = dim->act_measurement;
        j["def_point"] = point3d(dim->def_pt.x, dim->def_pt.y, dim->def_pt.z);
        j["user_text"] = safe_str(dim->user_text);
        break;
    }
    case DWG_TYPE_DIMENSION_RADIUS: {
        Dwg_Entity_DIMENSION_RADIUS* dim = ent->tio.DIMENSION_RADIUS;
        if (!dim) return nullptr;
        j["type"] = "radius";
        j["measurement"] = dim->act_measurement;
        j["def_point"] = point3d(dim->def_pt.x, dim->def_pt.y, dim->def_pt.z);
        j["leader_len"] = dim->leader_len;
        j["user_text"] = safe_str(dim->user_text);
        break;
    }
    case DWG_TYPE_DIMENSION_DIAMETER: {
        Dwg_Entity_DIMENSION_DIAMETER* dim = ent->tio.DIMENSION_DIAMETER;
        if (!dim) return nullptr;
        j["type"] = "diameter";
        j["measurement"] = dim->act_measurement;
        j["def_point"] = point3d(dim->def_pt.x, dim->def_pt.y, dim->def_pt.z);
        j["leader_len"] = dim->leader_len;
        j["user_text"] = safe_str(dim->user_text);
        break;
    }
    case DWG_TYPE_DIMENSION_ORDINATE: {
        Dwg_Entity_DIMENSION_ORDINATE* dim = ent->tio.DIMENSION_ORDINATE;
        if (!dim) return nullptr;
        j["type"] = "ordinate";
        j["measurement"] = dim->act_measurement;
        j["def_point"] = point3d(dim->def_pt.x, dim->def_pt.y, dim->def_pt.z);
        j["user_text"] = safe_str(dim->user_text);
        break;
    }
    default:
        return nullptr;
    }
    return j;
}
