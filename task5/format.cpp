#include <stack>
#include <sstream>
#include <iomanip>
#include "format.h"

namespace format_impl {
    std::regex regex("%([-+ #0]*)(\\d+|\\*)?(\\.)?(\\d+|\\*)?(hh|ll|[hljztL])?([diuoxXfFeEgAacspn%@])|[^%]+|%");
    std::regex_iterator<std::string::const_iterator> rend;

    void read_flags(Format &fmt, const std::string &str) {
        const char *next = str.c_str();

        while (true)
            switch (*(next++)) {
                case '0':
                    if (!fmt.minus)     //"-" flag discards "0" flag;
                        fmt.zero = true;
                    break;
                case '+':
                    fmt.plus = true;    //"+" flag discards " " flag;
                    fmt.space = false;
                    break;
                case '-':
                    fmt.minus = true;   //"-" flag discards "0" flag;
                    fmt.zero = false;
                    break;
                case '#':
                    fmt.sharp = true;
                    break;
                case ' ':
                    if (!fmt.plus)      //"+" flag discards " " flag;
                        fmt.space = true;
                    break;

                default:
                    goto end;
            }

        end: return;
    }

/*
 * if nullptr is given in fmt, it just generates the string with the given character
 */
    std::string sprintChar(Format const *fmt, char c) {
        std::string internal;
        std::string result;
        result.push_back(c);

        if (fmt != nullptr) {
            while (fmt->width > (internal.size() + 1))
                internal.push_back(fmt->zero ? '0' : ' ');

            if (fmt->minus)
                result = result + internal;
            else
                result = internal + result;
        }

        return result;
    }

//TODO: precision
    std::string sprintHexFloat(Format const *fmt, double d) {
        const char upperCase[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'P'};
        const char lowerCase[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'p'};
        const char *alpha = fmt->type == A ? upperCase : lowerCase;

        std::string result = fmt->type == A ? "0X" : "0x";
        std::string num;
        std::string hex;
        union {
            double f;
            struct {
                uint64_t m:52;
                uint_fast32_t p:11;
                uint_fast32_t s:1;
            };
        } Float;

        Float.f = d;
        int p = Float.p - 1023;

        const char *sign;
        if (fmt->plus && d >= 0) {
            sign = "+";
        } else if (fmt->space && d >= 0) {
            sign = " ";
        } else if (d < 0) {
            sign = "-";
        } else {
            sign = "";
        }
        result = sign + result;

        //??????? Some operations on denormal floats
        if (Float.p == 0 && Float.m != 0) {
            while ((Float.m & ((long long) 1 << 51)) == 0) {
                p--;
                Float.m <<= 1;
            }
            Float.m <<= 1;

            num = "1";
        } else {
            if (Float.m == 0 && Float.p == 0) {
                p = 0;
                num = "0";
            }
            else
                num = "1";
        }

        while (Float.m) {
            char c = alpha[Float.m & 0xF];
            if (c != '0' || hex.size())
                hex = sprintChar(nullptr, c) + hex;
            Float.m >>= 4;
        }

        //correcting precision
        while (fmt->precision > hex.size() && fmt->precision != DEFAULT_PRECISION) {
            hex += "0";
        }

        if (hex.size()) {
            result += num + "." + hex + sprintChar(nullptr, alpha[16]) + format("%+d", p);
        } else if (fmt->sharp) {
            result += num + "." + sprintChar(nullptr, alpha[16]) + format("%+d", p);
        } else {
            result += num + sprintChar(nullptr, alpha[16]) + format("%+d", p);
        }

        return result;
    }

    std::string sprintFloat(Format const *fmt, double d) {
        std::string result;
        std::string internal;
        std::ostringstream number;

        switch (fmt->type) {
            case F:
                number << std::uppercase;
            case f:
                number << std::fixed;
                break;

            case E:
                number << std::uppercase;
            case e:
                number << std::scientific;
                break;

            case G:
                number << std::uppercase;
            case g:
                //number << std::defaultfloat;  //TODO: Does not work on gcc, but works on clang
                break;

            case A:
            case a:
                return sprintHexFloat(fmt, d);

            default:
                throw std::logic_error("Logic error at sprintFloat()");
                break;
        }

        const char *signStr;

        //check sign
        if (fmt->plus && d > 0) {
            signStr = "+";
        } else if (fmt->space && d > 0) {
            signStr = " ";
        } else if (d < 0) {
            signStr = "-";
        } else {
            signStr = "";
        }

        //removing sign
        if (d < 0)
            d = -d;

        if (fmt->precision != DEFAULT_PRECISION)
            number.precision(fmt->precision);

        if (fmt->sharp)
            number << std::showpoint;

        number << d;
        result = number.str();

        size_t signLen = *signStr == '\0' ? 0 : 1;  //strlen(signStr);

        //correcting width
        while (fmt->width > (result.size() + internal.size() + signLen)) {
            const char c = (fmt->zero ? '0' : ' ');
            internal.push_back(c);
        }

        if (fmt->minus)
            result = signStr + result + internal;
        else if (fmt->zero)
            result = signStr + internal + result;
        else
            result = internal + signStr + result;

        return result;
    }

    std::string sprintStr(Format const *fmt, std::string str) {
        std::string internal;

        while (fmt->width > (str.size() + internal.size()))
            internal.push_back(' ');//fmt->zero ? '0' : ' '

        if (fmt->minus)
            return str + internal;
        else
            return internal + str;
    }

    template<typename T>
    std::string sprintHex(Format const *fmt, T arg) {
        const char upperCase[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
        const char lowerCase[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
        const char *alpha = fmt->type == X ? upperCase : lowerCase;

        typename std::make_unsigned<T>::type uarg = (typename std::make_unsigned<T>::type) arg;
        std::string result;
        std::string internal;

        const bool zero = fmt->precision == DEFAULT_PRECISION && fmt->zero;

        while (uarg) {   //TODO: O(n^2) is very bad
            const unsigned char digit = (unsigned char) (uarg & 0xF);
            result = sprintChar(nullptr, alpha[digit]) + result;
            uarg >>= 4;
        }

        if (result.size() == 0)
            result = "0";

        if (fmt->precision != DEFAULT_PRECISION) {
            while (fmt->precision > (result.size()))   //TODO: O(n^2)!!!
                result = "0" + result;
        }

        if (fmt->sharp)
            result = (fmt->type == X ? "0X" : "0x") + result;

        while (fmt->width > (result.size() + internal.size())) {
            const char c = (zero ? '0' : ' ');
            internal.push_back(c);
        }

        if (fmt->minus)
            result = result + internal;
        else
            result = internal + result;

        return result;
    }

    template<typename T>
    std::string sprintOct(Format const *fmt, T arg) {
        typename std::make_unsigned<T>::type uarg = (typename std::make_unsigned<T>::type) arg;
        std::string result;
        std::string internal;

        const bool zero = fmt->precision == DEFAULT_PRECISION && fmt->zero;

        while (uarg) {   //TODO: O(n^2) is very bad
            const unsigned char digit = (unsigned char) (uarg & 0x7);
            result = sprintChar(nullptr, digit + (char) 0x30) + result;
            uarg >>= 3;
        }

        if (result.size() == 0)
            result = "0";

        if (fmt->precision != DEFAULT_PRECISION) {
            while (fmt->precision > (result.size() + (fmt->sharp ? 1 : 0)))   //TODO: O(n^2)!!!
                result = "0" + result;
        }

        if (fmt->sharp)
            result = "0" + result;

        while (fmt->width > (result.size() + internal.size())) {
            const char c = (zero ? '0' : ' ');
            internal.push_back(c);
        }

        if (fmt->minus)
            result = result + internal;
        else
            result = internal + result;

        return result;
    }

    template<typename T>
    std::string sprintDec(Format const *fmt, T arg) {
        std::string result;
        std::string internal;
        const char *signStr;

        const bool zero = fmt->precision == DEFAULT_PRECISION && fmt->zero;

        if (arg < 0) {
            signStr = "-";
        } else if ((fmt->plus || fmt->space) && fmt->type == d) {
            signStr = (fmt->plus ? "+" : " ");
        } else {
            signStr = "";
        }

        while (arg) {   //TODO: O(n^2) is very bad
            char symbol = (arg % 10);   //abs(arg % 10) throws warnings on unsigned types
            if (symbol < 0)
                symbol = -symbol + '0';
            else
                symbol += '0';

            result = sprintChar(nullptr, symbol) + result;
            arg /= 10;
        }

        if (result.size() == 0)
            result = "0";

        size_t signLength = *signStr == '\0' ? 0
                                             : 1;//auto signLength = strlen(signStr); gcc throws here a warning for absolutely fucking no reason
        if (fmt->precision != DEFAULT_PRECISION) {
            while (fmt->precision > (result.size() + signLength))   //TODO: O(n^2)!!!
                result = "0" + result;
        }

        while (fmt->width > (result.size() + internal.size() + signLength)) {
            const char c = (zero ? '0' : ' ');
            internal.push_back(c);
        }

        if (fmt->minus)
            result = signStr + result + internal;
        else if (zero)
            result = signStr + internal + result;
        else
            result = internal + signStr + result;

        return result;
    }

    std::string sprintPointer(Format const *fmt, void *arg) {
        std::string str((size_t) snprintf(NULL, 0, fmt->fmt.c_str(), arg), '\0');
        snprintf(const_cast<char *>(str.data()), str.size() + 1, fmt->fmt.c_str(), arg);
        return str;
    }

    template <typename T>
    std::string sprintUnsigned(Format const *fmt, T arg) {
        switch (fmt->type) {
            case u:
                return sprintDec(fmt, arg);
            case o:
                return sprintOct(fmt, arg);
            case x:
            case X:
                return sprintHex(fmt, arg);

            default:
                throw std::logic_error("Logic error at sprintUnsigned()");
        }
    }

    template <> std::string sprint(Format const *fmt, int arg) { return sprintDec(fmt, arg); }
    template <> std::string sprint(Format const *fmt, unsigned int arg) { return sprintUnsigned(fmt, arg); }
    template <> std::string sprint(Format const *fmt, double arg) { return sprintFloat(fmt, arg); }
    template <> std::string sprint(Format const *fmt, float arg) { return sprintFloat(fmt, arg); }
    template <> std::string sprint(Format const *fmt, unsigned short int arg) { return sprintUnsigned(fmt, arg); }
    template <> std::string sprint(Format const *fmt, long int arg) { return sprintDec(fmt, arg); }
    template <> std::string sprint(Format const *fmt, unsigned long long int arg) { return sprintUnsigned(fmt, arg); }
    template <> std::string sprint(Format const *fmt, long long int arg) { return sprintDec(fmt, arg); }
    template <> std::string sprint(Format const *fmt, unsigned char arg) { return (fmt->type == c) ? sprintChar(fmt, arg) : sprintUnsigned(fmt, arg); }
    template <> std::string sprint(Format const *fmt, char arg) { return (fmt->type == c) ? sprintChar(fmt, arg) : sprintDec(fmt, arg); }
    template <> std::string sprint(Format const *fmt, char *arg) { return (arg == NULL) ? "(nil)" : sprintStr(fmt, arg); }
    template <> std::string sprint(Format const *fmt, std::string arg) { return sprintStr(fmt, arg); }
    template <> std::string sprint(Format const *fmt, short int arg) { return sprintDec(fmt, arg); }
    template <> std::string sprint(Format const *fmt, unsigned long int arg) { return sprintUnsigned(fmt, arg); }

    template<>
    std::string sprint(Format const *fmt, signed int *arg) {
        if (fmt->type == n) {
            *arg = 0;   //TODO: %n
            return "";
        } else if (fmt->type == p) {
            return sprintPointer(fmt, arg);
        }

        throw std::invalid_argument("Invalid argument: int* found");
    }

    template<>
    std::string sprint(Format const *fmt, std::nullptr_t arg) {
        if (fmt->type == p)
            return "(nil)";
        if (fmt->type == automatic)
            return "nullptr";

        throw std::invalid_argument("Invalid argument: nullptr found");
    }

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

    template<> bool check_type(FormatSpec len, FormatSpec type, double &) { return (len == def && (type == f || type == F || type == e || type == E || type == g || type == G || type == a || type == A)); }
    template<> bool check_type(FormatSpec len, FormatSpec type, long double &) { return (len == L && (type == f || type == F || type == e || type == E || type == g || type == G || type == a || type == A)); }

    template<> bool check_type(FormatSpec len, FormatSpec type, char &) { return (len == def && (type == c)); }
    //template<> bool check_type(FormatSpec len, FormatSpec type, wint_t &) { return (len == l && (type == c)); }

    template<> bool check_type(FormatSpec len, FormatSpec type, char *&) { return (len == def && (type == s)); }
    template<> bool check_type(FormatSpec len, FormatSpec type, wchar_t *&) { return (len == l && (type == s)); }

    template<> bool check_type(FormatSpec len, FormatSpec type, void *&) { return (len == def && (type == p)); }

    const char *demangle(const char *mangledName) {
#ifdef __GNUC__
        int status = -1;
        return abi::__cxa_demangle(mangledName, NULL, NULL, &status);
#else
#error "Demangling is not supported on your compiler"
#endif
    }
}