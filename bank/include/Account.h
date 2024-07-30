#pragma once

#include <string>
#include <vector>
#include "Money.h"
#include "Transaction.h"

class Account {
public:
    Account(const std::string& owner, const std::string& id, const Money& min, const Money& initial);

    std::string getOwner() const;
    std::string getID() const;
    Money getCurrent() const;
    Money getMinimum() const;

    void adjust(const Money& amount, bool force = false);
    
    // New method to add a transaction
    void addTransaction(const Transaction& transaction);
    
    // New method to get the last 5 transactions
    std::vector<Transaction> getLastTransactions(int count = 5) const;

private:
    std::string owner;
    std::string id;
    Money minimum;
    Money current;
    std::vector<Transaction> transactions;  // New member to store transactions
};