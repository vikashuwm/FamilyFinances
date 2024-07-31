// TransactionManager.h
#ifndef TRANSACTIONMANAGER_H
#define TRANSACTIONMANAGER_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include "Bank.h"

class TransactionManager : public QWidget {
    Q_OBJECT

public:
    explicit TransactionManager(Bank *bank, QWidget *parent = nullptr);
    void setUserAccess(const QString &username, bool isAdmin);
    void clearData();

public slots:
    void updateAccountList();

signals:
    void transactionCompleted();

private slots:
    void performTransaction();
    void showTransactionHistory();

private:
    void setupUI();
    void setupConnections();
    void updateTransactionHistory(const QString &accountId);

    Bank *bank;
    QLineEdit *sourceInput;
    QLineEdit *destInput;
    QLineEdit *amountInput;
    QPushButton *transferButton;
    QLabel *statusLabel;
    QTextEdit *transactionHistoryTextEdit;
    QString currentUser;
    bool isAdminUser;
    QString getUserAccountId(const QString &username);
};

#endif // TRANSACTIONMANAGER_H