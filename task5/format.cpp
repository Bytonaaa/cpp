#include <stack>
#include <sstream>
#include <iomanip>
#include "format.h"

namespace format_impl {
    std::regex regex("%([-+ #0]*)(\\d+|\\*)?(\\.)?(\\d+|\\*)?(hh|ll|[hljztL])?([diuoxXfFeEgGAacspn%@])|[^%]+|%");
    std::regex_iterator<std::string::const_iterator> rend;

    template<> bool check_type(FormatSpec len, FormatSpec type, int &) { return (len == def && (type == d || type == i)); }
    template<> bool check_type(FormatSpec len, FormatSpec type, signed char &) { return (len == hh && (type == d || type == i)); }
    template<> bool check_type(FormatSpec len, FormatSpec type, short int &) { return (len == h && (type == d || type == i)); }
    template<> bool check_type(FormatSpec len, FormatSpec type, long int &) { return (len == l && (type == d || type == i)); }
    template<> bool check_type(FormatSpec len, FormatSpec type, long long int &) { return (len == ll && (type == d || type == i)); }
    //template<> bool check_type(FormatSpec len, FormatSpec type, intmax_t &) { return (len == j && (type == d || type == i)); }
    //template<> bool check_type(FormatSpec len, FormatSpec type, size_t &) { return (len == z && (type == d || type == i || type == u || type == o || type == x || type == X)); }
    //template<> bool check_type(FormatSpec len, FormatSpec type, ptrdiff_t &) { return (len == t && (type == d || type == i || type == u || type == o || type == x || type == X)); }

    template<> bool check_type(FormatSpec len, FormatSpec type, unsigned int &) { return (len == def && (type == u || type == o || type == x || type == X)); }
    template<> bool check_type(FormatSpec len, FormatSpec type, unsigned char &) { return (len == hh && (type == u || type == o || type == x || type == X)); }
    template<> bool check_type(FormatSpec len, FormatSpec type, unsigned short int &) { return (len == h && (type == u || type == o || type == x || type == X)); }
    template<> bool check_type(FormatSpec len, FormatSpec type, unsigned long int &) { return (len == l && (type == u || type == o || type == x || type == X)); }
    template<> bool check_type(FormatSpec len, FormatSpec type, unsigned long long int &) { return (len == ll && (type == u || type == o || type == x || type == X)); }
    //template<> bool check_type(FormatSpec len, FormatSpec type, uintmax_t &) { return (len == j && (type == u || type == o || type == x || type == X)); }

    template<> bool check_type(FormatSpec len, FormatSpec type, float &) { return (len == def && (type == f || type == F || type == e || type == E || type == g || type == G || type == a || type == A)); }
    template<> bool check_type(FormatSpec len, FormatSpec type, double &) { return (len == def && (type == f || type == F || type == e || type == E || type == g || type == G || type == a || type == A)); }
    template<> bool check_type(FormatSpec len, FormatSpec type, long double &) { return (len == L && (type == f || type == F || type == e || type == E || type == g || type == G || type == a || type == A)); }

    template<> bool check_type(FormatSpec len, FormatSpec type, char &) { return (len == def && (type == c)); }
    //template<> bool check_type(FormatSpec len, FormatSpec type, wint_t &) { return (len == l && (type == c)); }

    template<> bool check_type(FormatSpec len, FormatSpec type, char *&) { return (len == def && (type == s)); }
    template<> bool check_type(FormatSpec len, FormatSpec type, wchar_t *&) { return (len == l && (type == s)); }

    template<> bool check_type(FormatSpec len, FormatSpec type, void *&) { return (len == def && (type == p)); }

    void format_implementation(std::string &s, std::regex_iterator<std::string::const_iterator> &rit, std::string &str) {
        if (rit == rend)
            return;

        if ((*rit)[RIT_STRING] == "%")
            throw std::invalid_argument("Invalid format");

        if ((*rit)[RIT_SPECIFIER] == "") {
            str.append((*rit)[RIT_STRING]);
        } else if ((*rit)[RIT_SPECIFIER] == "%") {
            str.push_back('%');
        } else {
            throw std::out_of_range("Too few arguments");
        }
        format_implementation(s, ++rit, str);
    }

    const char *demangle(const char *mangledName) {
#ifdef __GNUC__
        int status = -1;
        return abi::__cxa_demangle(mangledName, NULL, NULL, NULL);
#else
        return mangledName;
#endif
    }
}