#include "text_utils.h"
#include <cstring>
#include <regex>

#ifdef _WIN32
#include <windows.h>

static std::string win_convert(const char* src, unsigned int from_cp, unsigned int to_cp) {
    if (!src || !src[0]) return "";
    int wlen = MultiByteToWideChar(from_cp, 0, src, -1, nullptr, 0);
    if (wlen <= 0) return src;
    std::wstring wstr(wlen, 0);
    MultiByteToWideChar(from_cp, 0, src, -1, &wstr[0], wlen);
    int ulen = WideCharToMultiByte(to_cp, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (ulen <= 0) return src;
    std::string utf8(ulen, 0);
    WideCharToMultiByte(to_cp, 0, wstr.c_str(), -1, &utf8[0], ulen, nullptr, nullptr);
    if (!utf8.empty() && utf8.back() == '\0') utf8.pop_back();
    return utf8;
}

std::string gbk_to_utf8(const char* gbk_str) {
    return win_convert(gbk_str, 936, CP_UTF8);
}

std::string to_utf8(const char* str, unsigned int codepage) {
    if (codepage == CP_UTF8 || codepage == 65001) return str ? str : "";
    return win_convert(str, codepage, CP_UTF8);
}

#else
#include <iconv.h>

static std::string iconv_convert(const char* src, const char* from_enc) {
    if (!src || !src[0]) return "";
    iconv_t cd = iconv_open("UTF-8", from_enc);
    if (cd == (iconv_t)-1) return src;
    size_t inlen = std::strlen(src);
    size_t outlen = inlen * 4;
    std::string result(outlen, 0);
    char* inbuf = const_cast<char*>(src);
    char* outbuf = &result[0];
    size_t outleft = outlen;
    size_t inleft = inlen;
    iconv(cd, &inbuf, &inleft, &outbuf, &outleft);
    iconv_close(cd);
    result.resize(outlen - outleft);
    return result;
}

std::string gbk_to_utf8(const char* gbk_str) {
    return iconv_convert(gbk_str, "GBK");
}

std::string to_utf8(const char* str, unsigned int codepage) {
    if (codepage == 65001) return str ? str : "";
    if (codepage == 936 || codepage == 0) return gbk_to_utf8(str);
    return str ? str : "";
}
#endif

std::string sanitize_mtext(const std::string& raw) {
    std::string result = raw;
    result = std::regex_replace(result, std::regex(R"(\{\\f[^;]*;([^}]*)\})"), "$1");
    result = std::regex_replace(result, std::regex(R"(\\f[^;]*;)"), "");
    result = std::regex_replace(result, std::regex(R"(\\P)"), "\n");
    result = std::regex_replace(result, std::regex(R"(\\A\d+;)"), "");
    result = std::regex_replace(result, std::regex(R"(\\C\d+;)"), "");
    result = std::regex_replace(result, std::regex(R"(\\p[^;]*;)"), "");
    result = std::regex_replace(result, std::regex(R"(\\[WHSTQLOo~][^;]*;?)"), "");
    std::regex u_re(R"(\\U\+([0-9A-Fa-f]{4}))");
    std::string tmp;
    std::sregex_iterator it(result.begin(), result.end(), u_re), end;
    size_t last = 0;
    for (; it != end; ++it) {
        tmp += result.substr(last, it->position() - last);
        unsigned int cp = std::stoul((*it)[1].str(), nullptr, 16);
        if (cp < 0x80) {
            tmp += static_cast<char>(cp);
        } else if (cp < 0x800) {
            tmp += static_cast<char>(0xC0 | (cp >> 6));
            tmp += static_cast<char>(0x80 | (cp & 0x3F));
        } else {
            tmp += static_cast<char>(0xE0 | (cp >> 12));
            tmp += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            tmp += static_cast<char>(0x80 | (cp & 0x3F));
        }
        last = it->position() + it->length();
    }
    if (!tmp.empty() || last > 0) {
        tmp += result.substr(last);
        result = tmp;
    }
    result = std::regex_replace(result, std::regex(R"([\{\}])"), "");
    return result;
}
