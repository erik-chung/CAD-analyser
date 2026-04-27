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
    // Strip MTEXT formatting codes: {\fArial|...;text}, \P (newline), \A1; etc.
    std::string result = raw;
    // Remove {\fFontName|b0|i0|c134|p49; ... } font wrappers - keep inner text
    std::regex font_re(R"(\{\\f[^;]*;([^}]*)\})");
    result = std::regex_replace(result, font_re, "$1");
    // Remove \P (paragraph/newline markers) -> newline
    std::regex newline_re(R"(\\P)");
    result = std::regex_replace(result, newline_re, "\n");
    // Remove \A alignment codes
    std::regex align_re(R"(\\A\d+;)");
    result = std::regex_replace(result, align_re, "");
    // Remove remaining backslash codes like \W, \H, \T, \Q, \L, \O, \~
    std::regex code_re(R"(\\[WHSTQLOo~][^;]*;?)");
    result = std::regex_replace(result, code_re, "");
    // Remove stray braces
    std::regex brace_re(R"([\{\}])");
    result = std::regex_replace(result, brace_re, "");
    return result;
}
