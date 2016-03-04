#include "rational.h"
//num = 0 and denom = 0 work like NaN

rational::rational(int num) {
    numerator = num;
    denominator = 1;
}

rational::rational(int num, int denom) {
    if (denom < 0) {
        num = -num;
        denom = -denom;
    }
    
    int g = denom ? denom : 1;
    if (num)
        int g = gcd(abs(num), denom);

    numerator = num / g;
    denominator = denom / g;
}

int rational::getNum() const {
    return numerator;
}

int rational::getDenom() const {
    return denominator;
}

rational rational::operator+(const rational &thiz) const {
    int a = getNum();
    int b = getDenom();
    int c = thiz.getNum();
    int d = thiz.getDenom();
    return rational(a * d + b * c, b * d);
}

rational rational::operator-(const rational &thiz) const {
    int a = getNum();
    int b = getDenom();
    int c = thiz.getNum();
    int d = thiz.getDenom();
    return rational(a * d - b * c, b * d);
}

rational rational::operator*(const rational &b) const {
    return rational(numerator * b.numerator, denominator * b.denominator);
}

rational rational::operator/(const rational &b) const {
    return rational(numerator * b.denominator, denominator * b.numerator);
}

int rational::gcd(int a, int b) {
    while (a != b)
        if (a > b)
            a -= b;
        else
            b -= a;

    return a;
}
