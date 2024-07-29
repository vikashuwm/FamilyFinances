#include "Bank.h"
#include <random>
#include <sstream>
#include <iomanip>

std::shared_ptr<Account> Bank::open(const std::string& owner, const std::string& address,
                                    const Money& minimumBalance, const Money& initialBalance) {
    auto account = std::make_shared<Account>(owner, address, minimumBalance, initialBalance);
    accounts.push_back(account);
    return account;
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

std::string Bank::generatePassword(const std::string& owner, const std::string& accountId) {
    std::stringstream ss;
    ss << owner << accountId;

    std::string base = ss.str();
    
    // Generate a random number for additional complexity
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    int randomNumber = dis(gen);
    
    ss << randomNumber;

    // Simple hash-like conversion to ensure uniqueness
    std::hash<std::string> hash_fn;
    size_t hash = hash_fn(ss.str());
    
    std::stringstream passwordStream;
    passwordStream << std::hex << std::setw(8) << std::setfill('0') << hash;
    
    return passwordStream.str();
}

Bank::Iterator Bank::iterator() const {
    return Iterator(accounts);
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

std::vector<std::shared_ptr<Account>> Bank::getAccounts() const {
    return accounts;
}
