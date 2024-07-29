#include "Money.h"
#include <cmath>
#include <stdexcept>
#include <iomanip>
#include <sstream>

Money::Money(int64_t cents) : cents(cents) {
    if (cents == INT64_MIN) throw std::overflow_error("too small");
}

Money::Money(double amt) {
    if (amt > 92233720368547758.07 || amt < -92233720368547758.07) {
        throw std::overflow_error("amount is not in range");
    }
    this->cents = static_cast<int64_t>(std::round(amt * 100));
}

Money Money::fromCents(int64_t cents) {
    return Money(cents);
}

Money Money::fromDollars(double dollars) {
    return Money(dollars);
}

Money Money::negate() const {
    return Money::fromCents(-cents);
}

Money Money::add(const Money& other) const {
    int64_t result;
    if (__builtin_add_overflow(cents, other.cents, &result)) {
        throw std::overflow_error("overflow or underflow");
    }
    return Money::fromCents(result);
}

Money Money::sub(const Money& other) const {
    int64_t result;
    if (__builtin_sub_overflow(cents, other.cents, &result)) {
        throw std::overflow_error("overflow or underflow");
    }
    return Money::fromCents(result);
}

std::string Money::toString() const {
    std::ostringstream oss;
    oss << (cents < 0 ? "($" : "$")
        << std::abs(cents / 100) << '.'
        << std::setw(2) << std::setfill('0') << std::abs(cents % 100)
        << (cents < 0 ? ")" : "");
    return oss.str();
}

int Money::compareTo(const Money& other) const {
    return (cents > other.cents) - (cents < other.cents);
}