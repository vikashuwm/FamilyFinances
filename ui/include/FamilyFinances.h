#pragma once

#include <QMainWindow>
#include <QCloseEvent>
#include "Bank.h"
#include "AccountManager.h"
#include "TransactionManager.h"
#include "LoginPage.h"

class FamilyFinances : public QMainWindow {
    Q_OBJECT

public:
    FamilyFinances(QWidget *parent = nullptr);
    ~FamilyFinances();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool initializeDatabase();

private slots:
    void onLoginSuccessful(const QString &username, bool isAdmin);
    void onLogoutRequested();

private:
    void setupDatabase();
    void setupUI();
    void setUserAccess(const QString &username, bool isAdmin);

    Bank *bank;
    AccountManager *accountManager;
    TransactionManager *transactionManager;
    LoginPage *loginPage;
    QWidget *bankWidget;

    QString currentUser;
    bool isAdminUser;
};