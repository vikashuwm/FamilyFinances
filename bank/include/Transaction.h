#pragma once

#include <string>
#include "Money.h"
#include <chrono>

class Account;

class Transaction {
public:
    enum class Type { DEPOSIT, WITHDRAWAL, TRANSFER };

    Transaction(Account* source, Account* destination, const Money& amount);
    Transaction(const std::string& memo, Account* source, Account* destination, const Money& amount);

    Money perform(bool force = false);
    std::string toString() const;

    // New methods to get transaction details
    std::string getDate() const;
    Money getAmount() const;
    Type getType() const;

private:
    std::string memo;
    Account* source;
    Account* destination;
    Money amount;
    std::chrono::system_clock::time_point timestamp;  // New member to store transaction time
};