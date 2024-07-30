#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include <QWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QJsonObject>
#include "Bank.h"

class AccountManager : public QWidget {
    Q_OBJECT

public:
    explicit AccountManager(Bank *bank, QWidget *parent = nullptr);
    void setUserAccess(const QString &username, bool isAdmin);
    void updateAccountList();
    void saveAllAccountsToFile();

private slots:
    void showCreateAccountForm();
    void showTransactionHistory(int row, int column);

private:
    Bank *bank;
    QString currentUser;
    bool isAdminUser;
    QJsonObject userAccounts;

    QTableWidget *accountTable;
    QTextEdit *transactionHistoryTextEdit;
    QPushButton *createAccountButton;
    static const QString ACCOUNT_FILE_PATH;
    static const QString USER_CONFIG_PATH;
    QMap<QString, QString> userCredentials; // Maps usernames to passwords
    QList<Account*> allAccounts; // List of all accounts

    void setupUI();
    void loadAccountsFromFile();
    void loadUserAccounts();
    void displayTransactionHistory(const QString &accountId);
    QString generateUniqueAccountId();
    QDialog* setupAccountCreationDialog();
    bool isUserAuthorizedForAccount(const QString& accountId);
    void displayUserInfo();
};

#endif // ACCOUNTMANAGER_H