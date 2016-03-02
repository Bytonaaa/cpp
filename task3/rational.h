#ifndef TASK3_RATIONAL_H
#define TASK3_RATIONAL_H

class rational {
    int numerator;
    int denominator;

    int gcd(int a, int b);

public:
    rational(int num);
    rational(int num, int denom);

    int getNum() const;
    int getDenom() const;

    const rational operator+(const rational &thiz);
    const rational operator-(const rational &thiz);
    const rational operator*(const rational &b);
    const rational operator/(const rational &b);
};

#endif //TASK3_RATIONAL_H
