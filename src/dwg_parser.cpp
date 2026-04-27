#include "dwg_parser.h"
#include "entity_extractors.h"
#include "json_builder.h"
#include <iostream>
#include <cstring>

#include <filesystem>
namespace fs = std::filesystem;

static std::string filename_from_path(const std::string& path) {
    return fs::path(path).filename().string();
}

DwgParser::DwgParser() : loaded_(false) {
    std::memset(&dwg_, 0, sizeof(dwg_));
}

DwgParser::~DwgParser() {
    if (loaded_) {
        dwg_free(&dwg_);
    }
}

bool DwgParser::open(const std::string& filepath) {
    if (loaded_) {
        dwg_free(&dwg_);
        std::memset(&dwg_, 0, sizeof(dwg_));
        loaded_ = false;
    }
    filepath_ = filepath;
    int error = dwg_read_file(filepath.c_str(), &dwg_);
    if (error >= DWG_ERR_CRITICAL) {
        std::cerr << "Error: failed to read DWG file: " << filepath
                  << " (error code: " << error << ")" << std::endl;
        return false;
    }
    if (error > 0) {
        std::cerr << "Warning: non-critical errors reading " << filepath
                  << " (code: " << error << ")" << std::endl;
    }
    loaded_ = true;
    return true;
}

std::string DwgParser::get_version() const {
    if (!loaded_) return "unknown";
    switch (dwg_.header.version) {
    case R_2004: return "AC1018";
    case R_2007: return "AC1021";
    case R_2010: return "AC1024";
    case R_2013: return "AC1027";
    case R_2018: return "AC1032";
    case R_14:   return "AC1014";
    case R_2000: return "AC1015";
    default:     return "unknown";
    }
}

json DwgParser::extract_all() {
    if (!loaded_) return json::object();

    JsonBuilder builder;
    builder.set_metadata(
        filename_from_path(filepath_),
        get_version(),
        static_cast<int>(dwg_.header_vars.INSUNITS)
    );

    // Pass 1: Extract table objects (layers, styles, etc.)
    for (BITCODE_BL i = 0; i < dwg_.num_objects; i++) {
        Dwg_Object* obj = &dwg_.object[i];
        if (obj->supertype != DWG_SUPERTYPE_OBJECT) continue;

        if (obj->type == DWG_TYPE_LAYER) {
            json layer = extract_layer(obj);
            if (!layer.is_null())
                builder.add_layer(layer);
        }
    }

    // Pass 2: Extract entities
    for (BITCODE_BL i = 0; i < dwg_.num_objects; i++) {
        Dwg_Object* obj = &dwg_.object[i];
        if (obj->supertype != DWG_SUPERTYPE_ENTITY) continue;

        json entity;
        switch (obj->fixedtype) {
        case DWG_TYPE_LINE:
            entity = extract_line(obj);
            if (!entity.is_null()) builder.add_line(entity);
            break;

        case DWG_TYPE_CIRCLE:
            entity = extract_circle(obj);
            if (!entity.is_null()) builder.add_circle(entity);
            break;

        case DWG_TYPE_ARC:
            entity = extract_arc(obj);
            if (!entity.is_null()) builder.add_arc(entity);
            break;

        case DWG_TYPE_LWPOLYLINE:
            entity = extract_lwpolyline(obj);
            if (!entity.is_null()) builder.add_polyline(entity);
            break;

        case DWG_TYPE_POLYLINE_2D:
            entity = extract_polyline2d(obj);
            if (!entity.is_null()) builder.add_polyline(entity);
            break;

        case DWG_TYPE_TEXT:
            entity = extract_text(obj);
            if (!entity.is_null()) builder.add_text(entity);
            break;

        case DWG_TYPE_MTEXT:
            entity = extract_mtext(obj);
            if (!entity.is_null()) builder.add_text(entity);
            break;

        case DWG_TYPE_INSERT:
            entity = extract_insert(obj);
            if (!entity.is_null()) builder.add_block(entity);
            break;

        case DWG_TYPE_DIMENSION_LINEAR:
        case DWG_TYPE_DIMENSION_ALIGNED:
        case DWG_TYPE_DIMENSION_ANG2LN:
        case DWG_TYPE_DIMENSION_ANG3PT:
        case DWG_TYPE_DIMENSION_RADIUS:
        case DWG_TYPE_DIMENSION_DIAMETER:
        case DWG_TYPE_DIMENSION_ORDINATE:
            entity = extract_dimension(obj);
            if (!entity.is_null()) builder.add_dimension(entity);
            break;

        default:
            break;
        }
    }

    return builder.build();
}
