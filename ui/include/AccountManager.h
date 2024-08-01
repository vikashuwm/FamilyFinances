#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include <QWidget>
#include <QList>
#include <QString>
#include "Bank.h"
#include "Account.h"

class QTableWidget;
class QTextEdit;
class QPushButton;
class QDialog;
class QLabel;
class QListWidget;

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
public slots:
    QPushButton* getUserButton() { return userButton; }

private:
    Bank *bank;
    QList<Account*> allAccounts;
    QTableWidget *accountTable;
    QPushButton *userButton;
    QString currentUser;
    bool isAdminUser;

    // Widgets for user mode
    QWidget *userViewWidget;
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