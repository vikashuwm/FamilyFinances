#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QTableWidget>
#include "Bank.h"
#include "Account.h"

class QDialog;

class AccountManager : public QWidget {
    Q_OBJECT

public:
    explicit AccountManager(Bank *bank, QWidget *parent = nullptr);
    ~AccountManager();

    void setUserAccess(const QString &username, bool isAdmin);
    void clearData();

public slots:
    void onTransactionCompleted();

signals:
    void logoutRequested();

private slots:
    void showUserMenu();
    void logout();
    void showCreateAccountForm();
    void showAccountDetails(int row, int column);

private:
    Bank *bank;
    QList<Account*> allAccounts;
    QTableWidget *accountTable;
    QTextEdit *accountDetailsTextEdit;
    QPushButton *userButton;
    QString currentUser;
    bool isAdminUser;

    // Widgets for user mode
    QWidget *accountDetailsWidget;
    QLabel *accountNameLabel;
    QLabel *accountIdLabel;
    QLabel *accountBalanceLabel;
    QLabel *accountEmailLabel;
    QLabel *transactionHistoryLabel;
    QListWidget *transactionList;

    void setupUI();
    void loadAccountsFromDatabase();
    void updateAccountList();
    void displayAccountDetails(const QString &accountId);
    void saveAccountToDatabase(const Account* account);
    QString generateUniqueAccountId();
    QDialog* setupAccountCreationDialog();
    std::string getCurrentUserAccountId();
};

#endif // ACCOUNTMANAGER_H