// FamilyFinances.h
#ifndef FAMILYFINANCES_H
#define FAMILYFINANCES_H

#include <QMainWindow>
#include "Bank.h"
#include "LoginPage.h"
#include "AccountManager.h"
#include "TransactionManager.h"

class FamilyFinances : public QMainWindow {
    Q_OBJECT

public:
    FamilyFinances(QWidget *parent = nullptr);
    ~FamilyFinances();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onLoginSuccessful(const QString &username, bool isAdmin);

private:
    void setupUI();
    void setUserAccess(const QString &username, bool isAdmin);
    void setupDatabase();

    Bank *bank;
    LoginPage *loginPage;
    AccountManager *accountManager;
    TransactionManager *transactionManager;
    QWidget *bankWidget;
    QString currentUser;
    bool isAdminUser;
};

#endif // FAMILYFINANCES_H