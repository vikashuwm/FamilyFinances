#ifndef TRANSACTIONMANAGER_H
#define TRANSACTIONMANAGER_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "Bank.h"

class TransactionManager : public QWidget {
    Q_OBJECT

public:
    explicit TransactionManager(Bank *bank, QWidget *parent = nullptr);
    void setUserAccess(const QString &username, bool isAdmin);
    void clearData();

signals:
    void transactionCompleted();

private slots:
    void performTransaction();

private:
    Bank *bank;
    QLineEdit *sourceInput;
    QLineEdit *destInput;
    QLineEdit *amountInput;
    QPushButton *transferButton;
    QLabel *statusLabel;
    QString currentUser;
    bool isAdminUser;

    void setupUI();
    void setupConnections();
    QString getUserAccountId(const QString &username);
};

#endif // TRANSACTIONMANAGER_H