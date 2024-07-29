#include "Transaction.h"
#include "OverdraftException.h"
#include <stdexcept>

Transaction::Transaction(Account* source, Account* destination, const Money& amount)
    : Transaction("", source, destination, amount) {}

Transaction::Transaction(const std::string& memo, Account* source, Account* destination, const Money& amount)
    : memo(memo), source(source), destination(destination), amount(amount) {
    if (source == nullptr && destination == nullptr) {
        throw std::invalid_argument("Both source and destination cannot be null");
    }
    if (amount.compareTo(Money::fromCents(0)) <= 0) {
        throw std::invalid_argument("Transaction amount should be positive");
    }
    if (source != nullptr && destination != nullptr && source == destination) {
        throw std::invalid_argument("Source and destination accounts cannot be the same");
    }
}

Money Transaction::perform(bool force) {
    if (source == nullptr) {
        destination->adjust(amount, force);
        return amount.negate();
    } else if (destination == nullptr) {
        source->adjust(amount.negate(), force);
        return amount;
    } else {
        source->adjust(amount.negate(), force);
        try {
            destination->adjust(amount, force);
        } catch (const std::exception& e) {
            // Rollback the source adjustment if destination adjustment fails
            source->adjust(amount, true);
            throw;
        }
        return Money::fromCents(0);
    }
}

std::string Transaction::toString() const {
    std::string result;
    result += (source == nullptr) ? "DEPOSIT" : (destination == nullptr) ? "WITHDRAWAL" : "TRANSFER";
    if (source != nullptr) result += " from " + source->getID();
    if (destination != nullptr) result += " to " + destination->getID();
    result += ": " + amount.toString();
    if (!memo.empty()) result += " " + memo;
    return result;
}