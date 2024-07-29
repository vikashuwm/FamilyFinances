#include "Account.h"
#include "OverdraftException.h"
#include <stdexcept>

Account::Account(const std::string& owner, const std::string& id, const Money& min, const Money& initial)
    : owner(owner), id(id), minimum(min), current(initial) {
    if (owner.empty() || id.empty() || id.length() < 4) {
        throw std::invalid_argument("Invalid account parameters");
    }
}

std::string Account::getOwner() const { return owner; }
std::string Account::getID() const { return id; }
Money Account::getCurrent() const { return current; }

void Account::adjust(const Money& amount, bool force) {
    Money newBalance = current.add(amount);
    if (!force && newBalance.compareTo(minimum) < 0 && amount.compareTo(Money::fromCents(0)) < 0) {
        throw OverdraftException(*this, minimum.sub(newBalance));
    }
    current = newBalance;
}