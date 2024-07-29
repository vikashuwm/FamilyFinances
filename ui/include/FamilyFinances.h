#ifndef FAMILYFINANCES_H
#define FAMILYFINANCES_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>
#include <QDialog>
#include <QVector>
#include <QSharedPointer>

class Bank;
class LoginPage;
class Transaction;
class Account;

class FamilyFinances : public QMainWindow
{
    Q_OBJECT

public:
    explicit FamilyFinances(QWidget *parent = nullptr);
    ~FamilyFinances();

private slots:
    void onLoginSuccessful(const QString &username, bool isAdmin);
    void performTransaction();
    void showCreateAccountForm();

private:
    void setupUI();
    QDialog* setupAccountCreationDialog();
    QString generateUniqueAccountId();
    void updateAccountList();
    void setUserAccess(const QString &username, bool isAdmin);
    bool saveAccountToFile(const QString& accountId, const QString& password, 
                           const QString& owner, const QString& email);

    Bank *bank;
    LoginPage *loginPage;
    QWidget *bankWidget;

    QTableWidget *accountTable;
    QLineEdit *sourceInput;
    QLineEdit *destInput;
    QLineEdit *amountInput;
    QPushButton *transferButton;
    QPushButton *createAccountButton;
    QLabel *statusLabel;

    QString currentUser;
    bool isAdminUser;
};

#endif // FAMILYFINANCES_H
