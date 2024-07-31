#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H
#include <QWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include "Bank.h"

class AccountManager : public QWidget {
    Q_OBJECT

public:
    AccountManager(Bank *bank, QWidget *parent = nullptr);
    ~AccountManager();

    void setUserAccess(const QString &username, bool isAdmin);
    void clearData();

signals:
    void logoutRequested();

private slots:
    void showUserMenu();
    void showCreateAccountForm();
    void showAccountDetails(int row, int column);
    void logout();

private:
    void setupDatabase();
    void setupUI();
    void updateAccountList();
    void loadAccountsFromDatabase();
    void saveAccountToDatabase(const Account* account);
    void displayAccountDetails(const QString &accountId);
    QString generateUniqueAccountId();
    QDialog* setupAccountCreationDialog();

    Bank *bank;
    QTableWidget *accountTable;
    QTextEdit *accountDetailsTextEdit;
    QPushButton *userButton;
    QList<Account*> allAccounts;
    QString currentUser;
    bool isAdminUser;
};

#endif // ACCOUNTMANAGER_H