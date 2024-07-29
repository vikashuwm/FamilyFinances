#ifndef MONEY_H
#define MONEY_H

#include <string>
#include <cstdint>

class Money {
private:
    int64_t cents;

    explicit Money(int64_t cents);

public:
    Money() : cents(0) {}
    explicit Money(double amt);

    Money negate() const;
    Money add(const Money& other) const;
    Money sub(const Money& other) const;

    std::string toString() const;
    int compareTo(const Money& other) const;

    static Money fromCents(int64_t cents);
    static Money fromDollars(double dollars);
};

#endif // MONEY_H