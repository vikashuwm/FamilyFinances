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
#include <QLabel>

TransactionManager::TransactionManager(Bank *bank, QWidget *parent)
    : QWidget(parent), bank(bank), isAdminUser(false) {
    setupUI();
    setupConnections();
}

void TransactionManager::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Transaction form
    QGroupBox* transactionGroup = new QGroupBox(this);
    transactionGroup->setStyleSheet(
        "QGroupBox {"
        "    border: 1px solid #E0E0E0;"
        "    border-radius: 8px;"
        "    background-color: white;"
        "    padding: 15px;"
        "}"
    );
    QVBoxLayout* transactionLayout = new QVBoxLayout(transactionGroup);

    QStringList inputLabels = {"Source Account", "Destination Account", "Amount"};
    QList<QLineEdit*> inputs = {sourceInput = new QLineEdit(), destInput = new QLineEdit(), amountInput = new QLineEdit()};

    for (int i = 0; i < inputLabels.size(); ++i) {
        QLabel* label = new QLabel(inputLabels[i], this);
        label->setStyleSheet("color: #34495E; font-weight: bold; margin-top: 10px;");
        transactionLayout->addWidget(label);

        inputs[i]->setStyleSheet(
            "QLineEdit {"
            "    border: 1px solid #BDC3C7;"
            "    border-radius: 4px;"
            "    padding: 8px;"
            "    background-color: #ECF0F1;"
            "    color: #2C3E50;"
            "}"
            "QLineEdit:focus {"
            "    border-color: #3498DB;"
            "}"
        );
        transactionLayout->addWidget(inputs[i]);
    }

    transferButton = new QPushButton("Transfer", this);
    transferButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #3498DB;"
        "    color: white;"
        "    border: none;"
        "    padding: 10px;"
        "    border-radius: 4px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2980B9;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #2473A6;"
        "}"
    );
    transactionLayout->addWidget(transferButton);

    mainLayout->addWidget(transactionGroup);

    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet("color: #2C3E50; margin-top: 10px;");
    mainLayout->addWidget(statusLabel);

    setLayout(mainLayout);

    // Set overall widget style
    this->setStyleSheet(
        "QWidget {"
        "    font-size: 14px;"
        "    color: #2C3E50;"
        "}"
    );
}

void TransactionManager::setupConnections() {
    connect(transferButton, &QPushButton::clicked, this, &TransactionManager::performTransaction);
}

void TransactionManager::setUserAccess(const QString &username, bool isAdmin) {
    currentUser = username;
    isAdminUser = isAdmin;

    if (!isAdminUser) {
        QString userAccountId = getUserAccountId(username);
        sourceInput->setText(userAccountId);
        sourceInput->setReadOnly(true);
    } else {
        sourceInput->clear();
        sourceInput->setReadOnly(false);
    }
    
    destInput->clear();
    amountInput->clear();
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

    QSqlDatabase::database().transaction();

    QSqlQuery query;
    
    // Check source account
    query.prepare("SELECT balance FROM accounts WHERE id = :id");
    query.bindValue(":id", sourceId);
    if (!query.exec() || !query.next()) {
        QSqlDatabase::database().rollback();
        statusLabel->setText("Error: Invalid source account ID.");
        return;
    }
    double sourceBalance = query.value("balance").toDouble();
    
    // Check destination account
    query.prepare("SELECT id FROM accounts WHERE id = :id");
    query.bindValue(":id", destId);
    if (!query.exec() || !query.next()) {
        QSqlDatabase::database().rollback();
        statusLabel->setText("Error: Invalid destination account ID.");
        return;
    }
    
    // Check for sufficient funds
    if (sourceBalance < amount) {
        QSqlDatabase::database().rollback();
        statusLabel->setText("Error: Insufficient funds in source account.");
        return;
    }
    
    // Update source account
    query.prepare("UPDATE accounts SET balance = balance - :amount WHERE id = :id");
    query.bindValue(":amount", amount);
    query.bindValue(":id", sourceId);
    if (!query.exec()) {
        QSqlDatabase::database().rollback();
        statusLabel->setText("Error: Failed to update source account.");
        return;
    }
    
    // Update destination account
    query.prepare("UPDATE accounts SET balance = balance + :amount WHERE id = :id");
    query.bindValue(":amount", amount);
    query.bindValue(":id", destId);
    if (!query.exec()) {
        QSqlDatabase::database().rollback();
        statusLabel->setText("Error: Failed to update destination account.");
        return;
    }
    
    // Record transaction for source account
    query.prepare("INSERT INTO transactions (account_id, amount, type, date) VALUES (:source_id, :amount, :type, :date)");
    query.bindValue(":source_id", sourceId);
    query.bindValue(":amount", -amount);
    query.bindValue(":type", "TRANSFER");
    query.bindValue(":date", QDateTime::currentDateTime().toString(Qt::ISODate));
    if (!query.exec()) {
        QSqlDatabase::database().rollback();
        qDebug() << "Error inserting source transaction:" << query.lastError().text();
        statusLabel->setText("Error: Failed to record source transaction.");
        return;
    }
    
    // Record transaction for destination account
    query.prepare("INSERT INTO transactions (account_id, amount, type, date) VALUES (:dest_id, :amount, :type, :date)");
    query.bindValue(":dest_id", destId);
    query.bindValue(":amount", amount);
    query.bindValue(":type", "TRANSFER");
    query.bindValue(":date", QDateTime::currentDateTime().toString(Qt::ISODate));
    if (!query.exec()) {
        QSqlDatabase::database().rollback();
        qDebug() << "Error inserting destination transaction:" << query.lastError().text();
        statusLabel->setText("Error: Failed to record destination transaction.");
        return;
    }

    if (QSqlDatabase::database().commit()) {
        statusLabel->setText("Transaction completed successfully.");
        
        // Clear inputs
        if (!isAdminUser) {
            destInput->clear();
        } else {
            sourceInput->clear();
            destInput->clear();
        }
        amountInput->clear();
        
        // Emit the signal to notify that a transaction has been completed
        emit transactionCompleted();
    } else {
        QSqlDatabase::database().rollback();
        statusLabel->setText("Error: Transaction failed. Please try again.");
    }
}

QString TransactionManager::getUserAccountId(const QString &username) {
    QSqlQuery query;
    query.prepare("SELECT id FROM accounts WHERE username = :username");
    query.bindValue(":username", username);
    
    if (query.exec() && query.next()) {
        return query.value("id").toString();
    }
    return "";
}

void TransactionManager::clearData() {
    sourceInput->clear();
    destInput->clear();
    amountInput->clear();
    statusLabel->clear();
}