#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
#include <vector>
#include "Money.h"
#include "Transaction.h"

class Account {
public:
    Account(const std::string& owner, const std::string& id, const Money& minimumBalance, const Money& initialBalance);
    
    std::string getOwner() const;
    std::string getID() const;
    Money getCurrent() const;
    Money getMinimum() const;
    std::string getEmail() const;
    std::string getPassword() const;
    bool isAdmin() const;

    void setEmail(const std::string& email);
    void setPassword(const std::string& password);
    void setIsAdmin(bool admin);

    void adjust(const Money& amount, bool force = false);
    void addTransaction(const Transaction& transaction);
    std::vector<Transaction> getLastTransactions(int count) const;
    std::string getUsername() const;
    void setUsername(const std::string& newUsername);

private:
    std::string username;
    std::string owner;
    std::string id;
    Money minimum;
    Money current;
    std::string email;
    std::string password;
    bool admin;
    std::vector<Transaction> transactions;
};

#endif // ACCOUNT_H