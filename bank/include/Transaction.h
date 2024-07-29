#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include "Account.h"
#include "Money.h"

class Transaction {
private:
    std::string memo;
    Account* source;
    Account* destination;
    Money amount;

public:
    Transaction(Account* source, Account* destination, const Money& amount);
    Transaction(const std::string& memo, Account* source, Account* destination, const Money& amount);

    Money perform(bool force);
    std::string toString() const;
};

#endif // TRANSACTION_H