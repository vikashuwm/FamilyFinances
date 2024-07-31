#pragma once

#include <string>
#include <cstdint>

class Money {
private:
    int64_t cents;

public:
    Money(int64_t cents);
    Money(double amt);

    static Money fromCents(int64_t cents);
    static Money fromDollars(double dollars);

    Money negate() const;
    Money add(const Money& other) const;
    Money sub(const Money& other) const;
    std::string toString() const;
    int compareTo(const Money& other) const;
    double getDollars() const;
};