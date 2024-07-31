#include "TransactionManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>
#include <QDebug>
TransactionManager::TransactionManager(Bank *bank, QWidget *parent)
    : QWidget(parent), bank(bank), isAdminUser(false) {
    setupUI();
}


void TransactionManager::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);

    QGroupBox* transactionGroup = new QGroupBox(this);
    transactionGroup->setStyleSheet("QGroupBox { border: 1px solid #CCCCCC; border-radius: 10px; padding: 10px; background-color: rgba(255, 255, 255, 0.1); }");
    QVBoxLayout* transactionLayout = new QVBoxLayout(transactionGroup);

    QHBoxLayout* inputsLayout = new QHBoxLayout();
    QStringList inputLabels = {"Source Account", "Destination Account", "Amount"};
    QList<QLineEdit*> inputs = {sourceInput = new QLineEdit(), destInput = new QLineEdit(), amountInput = new QLineEdit()};

    for (int i = 0; i < inputLabels.size(); ++i) {
        QVBoxLayout* inputGroup = new QVBoxLayout();
        QLabel* label = new QLabel(inputLabels[i]);
        label->setStyleSheet("color: white;");
        inputGroup->addWidget(label);
        inputGroup->addWidget(inputs[i]);
        inputs[i]->setStyleSheet("background-color: rgba(255, 255, 255, 0.2); border: none; border-radius: 5px; padding: 5px; color: white;");
        inputsLayout->addLayout(inputGroup);

        if (i < inputLabels.size() - 1) {
            QLabel* arrow = new QLabel("→");
            arrow->setStyleSheet("color: white; font-size: 20px;");
            inputsLayout->addWidget(arrow);
        }
            }
    transactionLayout->addLayout(inputsLayout);

    QPushButton* transferButton = new QPushButton("TRANSFER", transactionGroup);
    transferButton->setStyleSheet("background-color: #28A745; color: white; border-radius: 5px; padding: 10px;");
    connect(transferButton, &QPushButton::clicked, this, &TransactionManager::performTransaction);
    transactionLayout->addWidget(transferButton);

    layout->addWidget(transactionGroup);

    statusLabel = new QLabel(this);
    statusLabel->setObjectName("statusLabel");
    statusLabel->setStyleSheet("color: white;");
    layout->addWidget(statusLabel);

    setLayout(layout);
}

void TransactionManager::setUserAccess(const QString &username, bool isAdmin) {
    currentUser = username;
    isAdminUser = isAdmin;

    if (!isAdminUser) {
        sourceInput->setText(username);
        sourceInput->setReadOnly(true);
    } else {
        sourceInput->setReadOnly(false);
    }
}

void TransactionManager::performTransaction() {
    QString sourceId = sourceInput->text();
    QString destId = destInput->text();
    QString amountStr = amountInput->text();

    if (sourceId.isEmpty() || destId.isEmpty() || amountStr.isEmpty()) {
        statusLabel->setText("Error: Please fill in all fields.");
        return;
    }

    bool conversionOk;
    double amount = amountStr.toDouble(&conversionOk);

    if (!conversionOk || amount <= 0) {
        statusLabel->setText("Error: Invalid amount. Please enter a positive number.");
        return;
    }

    try {
        auto sourceAccount = bank->findAccount(sourceId.toStdString());
        auto destAccount = bank->findAccount(destId.toStdString());

        if (!sourceAccount || !destAccount) {
            throw std::runtime_error("Invalid account ID");
        }

        if (!isAdminUser && sourceAccount->getOwner() != currentUser.toStdString()) {
            throw std::runtime_error("You can only transfer from your own account");
        }

        Money transferAmount = Money::fromDollars(amount);

        Transaction transaction(sourceAccount.get(), destAccount.get(), transferAmount);
        transaction.perform(false);  // Not forcing the transaction

        // Update the database
        QSqlQuery query;
        query.prepare("INSERT INTO transactions (account_id, amount, type, date) VALUES (:source_id, :amount, :type, :date)");
        query.bindValue(":source_id", QString::fromStdString(sourceAccount->getID()));
        query.bindValue(":amount", -amount);
        query.bindValue(":type", "TRANSFER");
        query.bindValue(":date", QDateTime::currentDateTime().toString(Qt::ISODate));
        if (!query.exec()) {
            qDebug() << "Error inserting source transaction:" << query.lastError().text();
            throw std::runtime_error("Database error: Failed to insert source transaction");
        }

        query.prepare("INSERT INTO transactions (account_id, amount, type, date) VALUES (:dest_id, :amount, :type, :date)");
        query.bindValue(":dest_id", QString::fromStdString(destAccount->getID()));
        query.bindValue(":amount", amount);
        query.bindValue(":type", "TRANSFER");
        query.bindValue(":date", QDateTime::currentDateTime().toString(Qt::ISODate));
        if (!query.exec()) {
            qDebug() << "Error inserting destination transaction:" << query.lastError().text();
            throw std::runtime_error("Database error: Failed to insert destination transaction");
        }

        // Update account balances in the database
        query.prepare("UPDATE accounts SET balance = balance - :amount WHERE id = :source_id");
        query.bindValue(":amount", amount);
        query.bindValue(":source_id", QString::fromStdString(sourceAccount->getID()));
        if (!query.exec()) {
            qDebug() << "Error updating source account balance:" << query.lastError().text();
            throw std::runtime_error("Database error: Failed to update source account balance");
        }

        query.prepare("UPDATE accounts SET balance = balance + :amount WHERE id = :dest_id");
        query.bindValue(":amount", amount);
        query.bindValue(":dest_id", QString::fromStdString(destAccount->getID()));
        if (!query.exec()) {
            qDebug() << "Error updating destination account balance:" << query.lastError().text();
            throw std::runtime_error("Database error: Failed to update destination account balance");
        }

        statusLabel->setText("Transaction completed successfully.");
    } catch (const std::exception& e) {
        statusLabel->setText(QString("Error: %1").arg(e.what()));
    }

    if (!isAdminUser) {
        sourceInput->clear();
    }
    destInput->clear();
    amountInput->clear();
}

void TransactionManager::clearData() {
    sourceInput->clear();
    destInput->clear();
    amountInput->clear();
    statusLabel->clear();
}