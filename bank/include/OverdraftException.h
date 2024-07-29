#ifndef OVERDRAFT_EXCEPTION_H
#define OVERDRAFT_EXCEPTION_H

#include <stdexcept>
#include "Account.h"
#include "Money.h"

class OverdraftException : public std::runtime_error {
private:
    Account account;
    Money amount;

public:
    OverdraftException(const Account& account, const Money& amount);

    const Account& getAccount() const;
    Money getAmount() const;
};

#endif // OVERDRAFT_EXCEPTION_H