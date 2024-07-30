#ifndef FAMILYFINANCES_H
#define FAMILYFINANCES_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QCloseEvent>  // Add this include
#include <QGroupBox>    // Add this include
#include <QUuid>

class Bank;
class LoginPage;

class FamilyFinances : public QMainWindow {
    Q_OBJECT

public:
    FamilyFinances(QWidget *parent = nullptr);
    ~FamilyFinances();

protected:
    void closeEvent(QCloseEvent *event) override;  // Declare the closeEvent method

private slots:
    void onLoginSuccessful(const QString &username, bool isAdmin);
    void performTransaction();
    void showCreateAccountForm();

private:
    void setupUI();
    void updateAccountList();
    QDialog* setupAccountCreationDialog();
    void saveAllAccountsToFile();  // Declare the saveAllAccountsToFile method
    void setUserAccess(const QString &username, bool isAdmin);
    QString generateUniqueAccountId();

    Bank *bank;
    LoginPage *loginPage;
    QWidget *bankWidget;
    QString currentUser;
    bool isAdminUser;

    QTableWidget *accountTable;
    QPushButton *createAccountButton;
    QLineEdit *sourceInput;
    QLineEdit *destInput;
    QLineEdit *amountInput;
    QLabel *statusLabel;
};

#endif // FAMILYFINANCES_H
