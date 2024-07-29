#ifndef FAMILYFINANCES_H
#define FAMILYFINANCES_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QComboBox>
#include <QIntValidator>

class Bank;
class LoginPage;

class FamilyFinances : public QMainWindow {
    Q_OBJECT

public:
    FamilyFinances(QWidget *parent = nullptr);
    ~FamilyFinances();

private slots:
    void onLoginSuccessful(const QString &username, bool isAdmin);
    void createAccount();
    void performTransaction();
    void updateAccountList();
    void showTransferInfo() {
        QMessageBox::information(this, "Bank Transfer Information",
            "A bank transfer is a method of electronically moving money from one bank account to another. "
            "It can be between accounts at the same bank or different banks. Transfers are typically used "
            "for sending money to friends or family, paying bills, or moving funds between your own accounts.");
    }

private:
    void setupUI();
    void setUserAccess(const QString &username, bool isAdmin);
    void setupAccountCreation(QVBoxLayout *mainLayout);

    Bank *bank;
    bool isAdminUser;
    int nextAccountId;
    QString currentUser;

    LoginPage *loginPage;
    QWidget *bankWidget;

    // Account creation fields
    QLineEdit *firstNameInput;
    QLineEdit *lastNameInput;
    QLineEdit *emailInput;
    QLineEdit *ageInput;
    QComboBox *sexInput;

    QLineEdit *ownerInput;
    QLineEdit *initialBalanceInput;
    QLineEdit *sourceInput;
    QLineEdit *destInput;
    QLineEdit *amountInput;
    QPushButton *createButton;
    QPushButton *transferButton;
    QTableWidget *accountTable;
    QLabel *statusLabel;
    QWidget *accountCreationWidget; // New widget for account creation section
};

#endif // FAMILYFINANCES_H
