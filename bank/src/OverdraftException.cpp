#include "OverdraftException.h"

OverdraftException::OverdraftException(const Account& account, const Money& amount)
    : std::runtime_error("Overdraft of " + account.getID() + " by " + amount.toString()),
      account(account), amount(amount) {
    if (amount.compareTo(Money::fromCents(0)) <= 0) {
        throw std::invalid_argument("Overdrawn amount must be positive");
    }
}

const Account& OverdraftException::getAccount() const { return account; }
Money OverdraftException::getAmount() const { return amount; }