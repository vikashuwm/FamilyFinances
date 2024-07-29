// Bank.h
#ifndef BANK_H
#define BANK_H

#include <vector>
#include <memory>
#include <string>
#include "Account.h"
#include "Money.h"

class Bank {
public:
    std::shared_ptr<Account> open(const std::string& owner, const std::string& address,
                                  const Money& minimumBalance, const Money& initialBalance);

    std::shared_ptr<Account> findAccount(const std::string& accountId) const;
    
    std::string generatePassword(const std::string& owner, const std::string& accountId);

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
    std::vector<std::shared_ptr<Account>> getAccounts() const;


private:
    std::vector<std::shared_ptr<Account>> accounts;
};

#endif // BANK_H
