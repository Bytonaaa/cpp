#include <stack>
#include <sstream>
#include <iomanip>
#include "format.h"

class FormatParser {
public:
    const char *next;

    FormatParser(const char *format) : next(format) { };

    void operator()(Format &fmt) {
        if (*next == '%') {
            next++;

            if (*next == '%') {
                fmt.str = "%";
                next++;
                return;
            }
            if (*next == '\0')
                throw std::invalid_argument("Unexpected end of format string");


            checkForFlags(fmt);

            if (isdigit(*next))
                fmt.width = readNumber();
            else if (*next == '*')
                next++, fmt.width = WP_READ;

            if (*next == '.') {
                next++;

                if (isdigit(*next))
                    fmt.precision = readNumber();
                else if (*next == '*')
                    next++, fmt.precision = WP_READ;
                else
                    //throw std::invalid_argument("Invalid format");
                    fmt.precision = 0;
            }

            checkForLength(fmt);
            checkForSpec(fmt);

        } else {
            while (*next != '%' && *next != '\0') {
                fmt.str.push_back(*next);
                next++;
            }
        }
    }

private:
    void checkForFlags(Format &fmt) {
        bool ok = true;
        while (ok) {
            switch (*next) {
                case '0':
                    fmt.zero = true;
                    break;
                case '+':
                    fmt.plus = true;
                    break;
                case '-':
                    fmt.minus = true;
                    break;
                case '#':
                    fmt.sharp = true;
                    break;

                case '\0':
                    throw std::invalid_argument("Invalid format: wrong flags");

                default:
                    ok = false;
                    next--;
            }
            next++;
        }
    }

    int readNumber() {
        int num = 0;
        while (isdigit(*next)) {
            num *= 10;
            if (num < 0)
                throw std::invalid_argument("Invalid format: overflow");

            num += *next - '0';
            next++;
        }
        return num;
    }

    void checkForLength(Format &fmt) {
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
                throw std::invalid_argument("Unexpected end of format");
            default:
                next--;
                break;
        }
        next++;
    }

    void checkForSpec(Format &fmt) {
        switch (*next) {
            case 'i':
            case 'd':
                fmt.spec = d;
                break;
            case 'u':
                fmt.spec = u;
                break;
            case 'o':
                fmt.spec = o;
                break;
            case 'x':
                fmt.spec = x;
                break;
            case 'X':
                fmt.spec = X;
                break;
            case 'f':
                fmt.spec = f;
            case 'F':
                fmt.spec = fmt.spec == f ? f : F;
                if (fmt.precision == -1)
                    fmt.precision = 6;
                break;
            case 'e':
                fmt.spec = e;
                break;
            case 'E':
                fmt.spec = E;
                break;
            case 'g':
                fmt.spec = g;
                break;
            case 'G':
                fmt.spec = G;
                break;
            case 'a':
                fmt.spec = a;
                break;
            case 'A':
                fmt.spec = A;
                break;
            case 'c':
                fmt.spec = c;
                break;
            case 's':
                fmt.spec = s;
                break;
            case 'p':
                fmt.spec = p;
                break;
            case 'n': //WTF is this???
                fmt.spec = n;
                break;
            case '\0':
                throw std::invalid_argument("Unexpected end of format");
            default:
                throw std::invalid_argument(format("Invalid format: wrong format specifier %%%c", *next));
        }
        next++;
    }
};

void parse(std::vector<Format> &fmt, const char *format) {
    FormatParser formatParser(format);
    while (*formatParser.next) {
        fmt.push_back(Format());
        formatParser(fmt.back());
    }
}

std::string commonFormatter(Format const *fmt, std::string str) {
    std::ostringstream result;

    result.setf(fmt->minus ? std::ios::left : std::ios::right);
    result.fill((fmt->zero && !fmt->minus) ? '0' : ' ');
    result.width(fmt->width);

    result << str;

    return result.str();
}

std::string sprintFloat(Format const *fmt, double d) {
    std::ostringstream result;
    if (fmt->precision != -1)
        result.precision(fmt->precision);
    if (fmt->sharp)
        result << std::showpoint;
    if (fmt->plus)
        result << std::showpos;

    switch (fmt->spec) {
        case F:
            result << std::uppercase;
        case f:
            result << std::fixed;
            break;

        case E:
            result << std::uppercase;
        case e:
            result << std::scientific;
            break;

        case G:
            result << std::uppercase;
        case g:
            //result << std::defaultfloat;
            break;

        case A:
            result << std::uppercase;
        case a:
            //result << std::hexfloat;
            break;

        default:
            break;
    }

    result << d;
    return result.str();
}

std::string sprintChar(Format const *fmt, char c) {
    std::string s;
    s.push_back(c);
    return s;
}

template<typename T>
std::string sprintHexOctDec(Format const *fmt, T arg) {
    std::ostringstream result;

    result.width(fmt->width);
    if (fmt->precision != -1)
        result.precision(fmt->precision);
    if (fmt->sharp)
        result << std::showbase;
    if (fmt->plus)
        result << std::showpos;

    switch (fmt->spec) {
        case d:
            result << std::dec;
            break;

        case X:
            result << std::uppercase;
        case x:
            result << std::hex;
            break;

        case o:
            result << std::oct;
            break;

        default:
            break;
    }

    result << arg;

    return result.str();
}


template<>
std::string sprint(Format const *fmt, int arg) {
    if (fmt->spec == d)
        return sprintHexOctDec(fmt, arg);
    else
        throw std::invalid_argument("Invalid argument: int found");
}

template<>
std::string sprint(Format const *fmt, unsigned int arg) {
    if (fmt->spec == u || fmt->spec == x || fmt->spec == X || fmt->spec == o)
        return sprintHexOctDec(fmt, arg);
    else
        throw std::invalid_argument("Invalid argument: unsigned int found");
}


template<>
std::string sprint(Format const *fmt, double arg) {
    if (fmt->spec == f || fmt->spec == F || fmt->spec == e || fmt->spec == E ||
        fmt->spec == g || fmt->spec == G || fmt->spec == a || fmt->spec == A)
        return sprintFloat(fmt, arg);
    else
        throw std::invalid_argument("Invalid argument: double found");
}

template<>
std::string sprint(Format const *fmt, float arg) {
    if (fmt->spec == f || fmt->spec == F || fmt->spec == e || fmt->spec == E ||
        fmt->spec == g || fmt->spec == G || fmt->spec == a || fmt->spec == A)
        return sprintFloat(fmt, arg);
    else
        throw std::invalid_argument("Invalid argument: float found");
}


template<>
std::string sprint<std::string>(Format const *fmt, std::string arg) {
    if (fmt->spec == s)
        return arg;
    else
        throw std::invalid_argument("Invalid argument: std::string found");
}

template<>
std::string sprint(Format const *fmt, const char *arg) {
    if (arg == NULL)
        //throw std::invalid_argument("Invalid argument: null pointer found");
        return std::string("(null)");

    if (fmt->spec == s)
        return sprint<std::string>(fmt, arg);
    else
        throw std::invalid_argument("Invalid argument: char* found");
}

template<>
std::string sprint(Format const *fmt, char *arg) {
    return sprint(fmt, (const char *) arg);
}


template<>
std::string sprint(Format const *fmt, char arg) {
    if (fmt->spec == c)
        return sprintChar(fmt, arg);
    else if (fmt->spec == d && fmt->length == hh)
        return sprintHexOctDec(fmt, arg);
    else
        throw std::invalid_argument("Invalid argument: char found");
}

std::string sprint(Format const *fmt, unsigned char arg) {
    if (fmt->spec == c)
        return sprintChar(fmt, arg);
    else if ((fmt->spec == u || fmt->spec == o || fmt->spec == x || fmt->spec == X) && fmt->length == hh)
        return sprintHexOctDec(fmt, arg);
    else
        throw std::invalid_argument("Invalid argument: unsigned char found");
}


template<>
std::string sprint(Format const *fmt, short int arg) {
    if (fmt->spec == d && fmt->length == hh)
        return sprintHexOctDec(fmt, arg);
    else
        throw std::invalid_argument("Invalid argument: short int found");
}

std::string sprint(Format const *fmt, unsigned short int arg) {
    if ((fmt->spec == u || fmt->spec == o || fmt->spec == x || fmt->spec == X) && fmt->length == h)
        return sprintHexOctDec(fmt, arg);
    else
        throw std::invalid_argument("Invalid argument: unsigned short int found");
}


template<>
std::string sprint(Format const *fmt, long int arg) {
    if (fmt->spec == d && fmt->length == l)
        return sprintHexOctDec(fmt, arg);
    else
        throw std::invalid_argument("Invalid argument: long int found");
}

template<>
std::string sprint(Format const *fmt, unsigned long int arg) {
    if ((fmt->spec == u || fmt->spec == o || fmt->spec == x || fmt->spec == X) && fmt->length == l)
        return sprintHexOctDec(fmt, arg);
    else
        throw std::invalid_argument("Invalid argument: unsigned long int found");
}


template<>
std::string sprint(Format const *fmt, long long int arg) {
    if (fmt->spec == d && fmt->length == ll)
        return sprintHexOctDec(fmt, arg);
    else
        throw std::invalid_argument("Invalid argument: long long int found");
}

template<>
std::string sprint(Format const *fmt, unsigned long long int arg) {
    if ((fmt->spec == u || fmt->spec == o || fmt->spec == x || fmt->spec == X) && fmt->length == ll)
        return sprintHexOctDec(fmt, arg);
    else
        throw std::invalid_argument("Invalid argument: unsigned long long int found");
}


template<>
std::string sprint(Format const *fmt, int *arg) {
    if (fmt->spec == n) {
        *arg = 0;   //TODO: %n
        return "";
    }
    else
        throw std::invalid_argument("Invalid argument: int* found");
}


template<>
std::string sprint(Format const *fmt, void *arg) {
    if (fmt->spec == p)
        return sprintHexOctDec(fmt, arg);
    else
        throw std::invalid_argument("Invalid argument: int* found");
}


template<>
std::string sprint(Format const *fmt, std::nullptr_t arg) {
    throw std::invalid_argument("Invalid argument: nullptr found");
}

std::string format(std::string const &format) {
    std::string str;
    std::vector<Format> fmt;

    parse(fmt, format.c_str());
    gen(fmt.data(), fmt.size(), str);

    return str;
}

void gen(Format *fmt, unsigned long size, std::string &str) {
    if (size) {
        if (fmt->str.size()) {
            str += fmt->str;
            gen(fmt + 1, size - 1, str);
        } else
            throw std::out_of_range("I need more arguments!!1!");//TODO
    }
}

template<>
int checkForInt<int>(int arg) {
    return arg;
}

template<>
int checkForInt<size_t>(size_t arg) {
    return (int) arg;
}

template<>
int checkForInt<unsigned int>(unsigned int arg) {
    return (int) arg;
}