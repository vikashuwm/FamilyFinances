#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include <QWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVector>
#include <memory>
#include "Bank.h"
#include "Account.h"

class AccountManager : public QWidget {
    Q_OBJECT

public:
    explicit AccountManager(Bank *bank, QWidget *parent = nullptr);
    ~AccountManager();

    void setUserAccess(const QString &username, bool isAdmin);
    void updateAccountList();

private slots:
    void showCreateAccountForm();
    void showTransactionHistory(int row, int column);

private:
    void setupDatabase();
    void setupUI();
    void loadAccountsFromDatabase();
    void saveAccountToDatabase(const Account* account);
    void displayTransactionHistory(const QString &accountId);
    QDialog* setupAccountCreationDialog();
    QString generateUniqueAccountId();

    Bank *bank;
    QVector<Account*> allAccounts;
    QString currentUser;
    bool isAdminUser;

    QTableWidget *accountTable;
    QTextEdit *transactionHistoryTextEdit;
    QPushButton *createAccountButton;
};

#endif // ACCOUNTMANAGER_H