#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H

#include <string>

std::string gbk_to_utf8(const char* gbk_str);
std::string to_utf8(const char* str, unsigned int codepage);
std::string sanitize_mtext(const std::string& raw);

#endif
