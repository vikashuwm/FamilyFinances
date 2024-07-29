#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "Money.h"
#include <string>

class Account {
private:
    std::string owner;
    std::string id;
    Money minimum;
    Money current;

public:
    Account(const std::string& owner, const std::string& id, const Money& min, const Money& initial);

    std::string getOwner() const;
    std::string getID() const;
    Money getCurrent() const;

    void adjust(const Money& amount, bool force);
};

#endif // ACCOUNT_H