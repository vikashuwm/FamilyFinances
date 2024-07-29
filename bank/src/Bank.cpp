#include "Bank.h"
#include <stdexcept>
#include <algorithm>

std::shared_ptr<Account> Bank::open(const std::string& owner, const std::string& address,
                                    const Money& minimumBalance, const Money& initialBalance) {
    auto account = std::make_shared<Account>(owner, address, minimumBalance, initialBalance);
    accounts.push_back(account);
    return account;
}

Bank::Iterator Bank::iterator() const {
    return Iterator(accounts);
}

std::shared_ptr<Account> Bank::findAccount(const std::string& accountId) const {
    auto it = std::find_if(accounts.begin(), accounts.end(),
                           [&accountId](const std::shared_ptr<Account>& account) {
                               return account->getID() == accountId;
                           });
    if (it != accounts.end()) {
        return *it;
    }
    return nullptr;
}

Bank::Iterator::Iterator(const std::vector<std::shared_ptr<Account>>& accounts)
    : accounts(accounts), currentIndex(0) {}

bool Bank::Iterator::hasNext() const {
    return currentIndex < accounts.size();
}

std::shared_ptr<Account> Bank::Iterator::next() {
    if (!hasNext()) {
        throw std::out_of_range("No more accounts");
    }
    return accounts[currentIndex++];
}