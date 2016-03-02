class rational {
    int numerator;
    int denominator;

    int gcd(int a, int b) {
        while (a != b)
            if (a > b)
                a -= b;
            else
                b -= a;

        return a;
    }

public:
    rational(int num) {
        numerator = num;
        denominator = 1;
    }

    rational(int num, int denom) {
        int g = gcd(num, denom);

        numerator = num / g;
        denominator = denom / g;
    }

    int getNum() const {
        return numerator;
    }

    int getDenom() const {
        return denominator;
    }

    const rational operator+(const rational &thiz) {
        int a = getNum();
        int b = getDenom();
        int c = thiz.getNum();
        int d = thiz.getDenom();
        return rational(a * d + b * c, b * d);
    }

    const rational operator-(const rational &thiz) {
        int a = getNum();
        int b = getDenom();
        int c = thiz.getNum();
        int d = thiz.getDenom();
        return rational(a * d - b * c, b * d);
    }

    const rational operator*(const rational &b) {
        return rational(numerator * b.numerator, denominator * b.denominator);
    }

    const rational operator/(const rational &b) {
        return rational(numerator * b.denominator, denominator * b.numerator);
    }
};