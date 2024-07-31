// TransactionManager.h
#ifndef TRANSACTIONMANAGER_H
#define TRANSACTIONMANAGER_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include "Bank.h"

class TransactionManager : public QWidget {
    Q_OBJECT

public:
    TransactionManager(Bank *bank, QWidget *parent = nullptr);
    void setUserAccess(const QString &username, bool isAdmin);
    void clearData();

private slots:
    void performTransaction();

private:
    void setupUI();

    Bank *bank;
    QLineEdit *sourceInput;
    QLineEdit *destInput;
    QLineEdit *amountInput;
    QLabel *statusLabel;
    QString currentUser;
    bool isAdminUser;
};
#endif // TRANSACTIONMANAGER_H
