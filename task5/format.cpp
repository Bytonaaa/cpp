#include <stack>
#include <sstream>
#include <iomanip>
#include "format.h"

/*
 * FormatParser parses a format string into tokens which are understandable for functions below
 */

namespace formatImpl {

    class FormatParser {
    public:
        const char *const formatString;
        const char *next;

        FormatParser(const char *format) : next(format), formatString(format) { };

        void nextToken(Format &fmt) {
            if (*next == '%') {
                next++;

                //check for "%%"
                if (*next == '%') {
                    fmt.str = "%";
                    next++;
                    return;
                }
                if (*next == '\0')
                    throw std::invalid_argument("Invalid format: unexpected end of string");

                readFlags(fmt);

                //reading width
                if (isdigit(*next))
                    fmt.width = readNumber();
                else if (*next == '*')
                    next++, fmt.width = WP_READ;

                //reading precision
                if (*next == '.') {
                    next++;

                    if (isdigit(*next))
                        fmt.precision = readNumber();
                    else if (*next == '*')
                        next++, fmt.precision = WP_READ;
                    else
                        fmt.precision = 0;  //"%.d" is equal to "%.0d"
                }

                readLength(fmt);
                readType(fmt);

            } else {
                //reading string
                while (*next != '%' && *next != '\0') {
                    fmt.str.push_back(*next);
                    next++;
                }
            }
        }

    private:
        void readFlags(Format &fmt) {
            bool ok = true;
            while (ok) {
                switch (*next) {
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

                    case '\0':
                        throw std::invalid_argument("Invalid format: unexpected end of string");

                    default:
                        ok = false;
                        next--;
                }
                next++;
            }
        }

        //parses integer
        int readNumber() {
            int num = 0;
            while (isdigit(*next)) {
                num *= 10;
                if (num < 0)
                    throw std::invalid_argument("Invalid format: overflow while parsing number");

                num += *next - '0';
                next++;
            }
            return num;
        }

        void readLength(Format &fmt) {
            switch (*next) {
                case 'l':
                    if (next[1] == 'l')
                        next++, fmt.length = ll;
                    else
                        fmt.length = l;
                    break;
                case 'h':
                    if (next[1] == 'h')
                        next++, fmt.length = hh;
                    else
                        fmt.length = h;
                    break;
                case 'j':
                    fmt.length = f;
                    break;
                case 'z':
                    fmt.length = z;
                    break;
                case 't':
                    fmt.length = t;
                    break;
                case 'L':
                    fmt.length = L;

                case '\0':
                    throw std::invalid_argument("Invalid format: unexpected end of format");
                default:
                    next--;
                    break;
            }
            next++;
        }

        void readType(Format &fmt) {
            switch (*next) {
                case 'i':
                case 'd':
                    fmt.type = d;
                    break;
                case 'u':
                    fmt.type = u;
                    break;
                case 'o':
                    fmt.type = o;
                    break;
                case 'x':
                    fmt.type = x;
                    break;
                case 'X':
                    fmt.type = X;
                    break;
                case 'f':
                    fmt.type = f;
                case 'F':
                    fmt.type = fmt.type == f ? f : F;
                    if (fmt.precision == -1)
                        fmt.precision = 6;  //default prescision for floats
                    break;
                case 'e':
                    fmt.type = e;
                    break;
                case 'E':
                    fmt.type = E;
                    break;
                case 'g':
                    fmt.type = g;
                    break;
                case 'G':
                    fmt.type = G;
                    break;
                case 'a':
                    fmt.type = a;
                    break;
                case 'A':
                    fmt.type = A;
                    break;
                case 'c':
                    fmt.type = c;
                    break;
                case 's':
                    fmt.type = s;
                    break;
                case 'p':
                    fmt.type = p;
                    break;
                case 'n': //WTF is this???
                    fmt.type = n;
                    break;
                case '@':
                    fmt.type = automatic;
                    break;
                case '\0':
                    throw std::invalid_argument("Unexpected end of format");
                default:
                    throw std::invalid_argument(
                            format("Invalid format: wrong format specifier %%%c in format \"%s\"", *next,
                                   formatString));
            }
            next++;
        }
    };

    void parseFormatString(std::vector<Format> &fmt, const char *format) {
        FormatParser formatParser(format);
        while (*formatParser.next) {
            fmt.push_back(Format());
            formatParser.nextToken(fmt.back());
        }
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
        std::ostringstream result;

        if (fmt->sharp)
            result << std::showbase;

        result << arg;

        return result.str();
    }

    template<>
    std::string sprint(Format const *fmt, int arg) {
        if (fmt->type == d)
            return sprintDec(fmt, arg);

        throw std::invalid_argument("Invalid argument: int found");
    }

    template<>
    std::string sprint(Format const *fmt, unsigned int arg) {
        if (fmt->type == u)// || fmt->type == x || fmt->type == X || fmt->type == o
            return sprintDec(fmt, arg);
        else if (fmt->type == o)
            return sprintOct(fmt, arg);
        else if (fmt->type == X || fmt->type == x)
            return sprintHex(fmt, arg);

        throw std::invalid_argument("Invalid argument: unsigned int found");
    }


    template<>
    std::string sprint(Format const *fmt, double arg) {
        if (fmt->type == f || fmt->type == F || fmt->type == e || fmt->type == E ||
            fmt->type == g || fmt->type == G || fmt->type == a || fmt->type == A)
            return sprintFloat(fmt, arg);
        else if (fmt->type == automatic)
            return sprintAuto<double>(fmt, arg);

        throw std::invalid_argument("Invalid argument: double found");
    }

    template<>
    std::string sprint(Format const *fmt, float arg) {
        if (fmt->type == f || fmt->type == F || fmt->type == e || fmt->type == E ||
            fmt->type == g || fmt->type == G || fmt->type == a || fmt->type == A)
            return sprintFloat(fmt, arg);

        throw std::invalid_argument("Invalid argument: float found");
    }


    template<>
    std::string sprint<std::string>(Format const *fmt, std::string arg) {
        if (fmt->type == s)
            return sprintStr(fmt, arg);

        throw std::invalid_argument("Invalid argument: std::string found");
    }

    template<>
    std::string sprint(Format const *fmt, const char *arg) {
        if (arg == NULL)
            //throw std::invalid_argument("Invalid argument: null pointer found");
            return "(nil)";

        if (fmt->type == s)
            return sprintStr(fmt, arg);

        throw std::invalid_argument("Invalid argument: char* found");
    }

    template<>
    std::string sprint(Format const *fmt, char *arg) {
        return sprint(fmt, (const char *) arg);
    }


    template<>
    std::string sprint(Format const *fmt, char arg) {
        if (fmt->type == c)
            return sprintChar(fmt, arg);
        else if (fmt->type == d && fmt->length == hh)
            return sprintDec(fmt, arg);

        throw std::invalid_argument("Invalid argument: char found");
    }

    std::string sprint(Format const *fmt, unsigned char arg) {
        if (fmt->type == c)
            return sprintChar(fmt, arg);
        else if (fmt->length == hh) {
            if (fmt->type == u)
                return sprintDec(fmt, arg);
            else if (fmt->type == o)
                return sprintOct(fmt, arg);
            else if (fmt->type == X || fmt->type == x)
                return sprintHex(fmt, arg);
        }

        throw std::invalid_argument("Invalid argument: unsigned char found");
    }


    template<>
    std::string sprint(Format const *fmt, short int arg) {
        if (fmt->type == d && fmt->length == hh)
            return sprintDec(fmt, arg);

        throw std::invalid_argument("Invalid argument: short int found");
    }

    std::string sprint(Format const *fmt, unsigned short int arg) {
        if (fmt->length == h) {
            if (fmt->type == u)
                return sprintDec(fmt, arg);
            else if (fmt->type == o)
                return sprintOct(fmt, arg);
            else if (fmt->type == X || fmt->type == x)
                return sprintHex(fmt, arg);
        }

        throw std::invalid_argument("Invalid argument: unsigned short int found");
    }


    template<>
    std::string sprint(Format const *fmt, long int arg) {
        if (fmt->type == d && fmt->length == l)
            return sprintDec(fmt, arg);

        throw std::invalid_argument("Invalid argument: long int found");
    }

    template<>
    std::string sprint(Format const *fmt, unsigned long int arg) {
        if (fmt->length == l) {
            if (fmt->type == u)
                return sprintDec(fmt, arg);
            else if (fmt->type == o)
                return sprintOct(fmt, arg);
            else if (fmt->type == X || fmt->type == x)
                return sprintHex(fmt, arg);
        }

        throw std::invalid_argument("Invalid argument: unsigned long int found");
    }


    template<>
    std::string sprint(Format const *fmt, long long int arg) {
        if (fmt->type == d && fmt->length == ll)
            return sprintDec(fmt, arg);

        throw std::invalid_argument("Invalid argument: long long int found");
    }

    template<>
    std::string sprint(Format const *fmt, unsigned long long int arg) {
        if (fmt->length == ll) {
            if (fmt->type == u)
                return sprintDec(fmt, arg);
            else if (fmt->type == o)
                return sprintOct(fmt, arg);
            else if (fmt->type == X || fmt->type == x)
                return sprintHex(fmt, arg);
        }

        throw std::invalid_argument("Invalid argument: unsigned long long int found");
    }


    template<>
    std::string sprint(Format const *fmt, signed int *arg) {
        if (fmt->type == n) {
            *arg = 0;   //TODO: %n
            return "";
        } else if (fmt->type == p) {
            return sprintPointer(fmt, arg);
        } else if (fmt->type == automatic) {
            //TODO: return sprintAuto(fmt, arg);
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

    void formatImplementation(Format *fmt, unsigned long size, std::string &str) {
        if (size) {
            if (fmt->str.size()) {
                str += fmt->str;
                formatImplementation(fmt + 1, size - 1, str);
            } else
                throw std::out_of_range("Too few arguments");
        }
    }
}