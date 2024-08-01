#ifndef FAMILYFINANCES_H
#define FAMILYFINANCES_H

#include <QMainWindow>
#include "Bank.h"
#include "LoginPage.h"
#include "AccountManager.h"
#include "TransactionManager.h"

class QFrame;

class FamilyFinances : public QMainWindow {
    Q_OBJECT

public:
    explicit FamilyFinances(QWidget *parent = nullptr);
    ~FamilyFinances();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onLoginSuccessful(const QString &username, bool isAdmin);
    void onLogoutRequested();

private:
    Bank *bank;
    LoginPage *loginPage;
    QWidget *bankWidget;
    AccountManager *accountManager;
    TransactionManager *transactionManager;
    QString currentUser;
    bool isAdminUser;

    bool initializeDatabase();
    void setupUI();
    void setUserAccess(const QString &username, bool isAdmin);
    QFrame* createStyledFrame();
};

#endif // FAMILYFINANCES_H