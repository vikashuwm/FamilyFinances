// AccountManager.h
#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include "Bank.h"
#include <QDebug>

class AccountManager : public QWidget {
    Q_OBJECT

public:
    AccountManager(Bank *bank, QWidget *parent = nullptr);
    void setUserAccess(const QString &username, bool isAdmin);
    void saveAllAccountsToFile();

private slots:
    void showCreateAccountForm();

private:
    void setupUI();
    void updateAccountList();
    QDialog* setupAccountCreationDialog();
    QString generateUniqueAccountId();

    Bank *bank;
    QTableWidget *accountTable;
    QPushButton *createAccountButton;
    QString currentUser;
    bool isAdminUser;
};

#endif // ACCOUNTMANAGER_H