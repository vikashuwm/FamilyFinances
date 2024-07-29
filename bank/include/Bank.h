#pragma once

#include <memory>
#include <vector>
#include <string>
#include "Account.h"

class Bank {
public:
    Bank() = default;
    ~Bank() = default;

    std::shared_ptr<Account> open(const std::string& owner, const std::string& address,
                                  const Money& minimumBalance, const Money& initialBalance);
    
    class Iterator {
    public:
        Iterator(const std::vector<std::shared_ptr<Account>>& accounts);
        bool hasNext() const;
        std::shared_ptr<Account> next();
    private:
        const std::vector<std::shared_ptr<Account>>& accounts;
        size_t currentIndex;
    };

    Iterator iterator() const;

    std::shared_ptr<Account> findAccount(const std::string& accountId) const;

private:
    std::vector<std::shared_ptr<Account>> accounts;
};