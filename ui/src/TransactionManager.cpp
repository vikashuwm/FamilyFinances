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
    setupConnections();
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

    transferButton = new QPushButton("TRANSFER", transactionGroup);
    transferButton->setStyleSheet("background-color: #28A745; color: white; border-radius: 5px; padding: 10px;");
    transactionLayout->addWidget(transferButton);

    layout->addWidget(transactionGroup);

    statusLabel = new QLabel(this);
    statusLabel->setObjectName("statusLabel");
    statusLabel->setStyleSheet("color: white;");
    layout->addWidget(statusLabel);

    transactionHistoryTextEdit = new QTextEdit(this);
    transactionHistoryTextEdit->setReadOnly(true);
    transactionHistoryTextEdit->setStyleSheet("background-color: rgba(255, 255, 255, 0.2); border: none; border-radius: 5px; padding: 5px; color: white;");
    layout->addWidget(transactionHistoryTextEdit);

    QPushButton* historyButton = new QPushButton("Show Transaction History", this);
    historyButton->setStyleSheet("background-color: #007BFF; color: white; border-radius: 5px; padding: 10px;");
    layout->addWidget(historyButton);

    setLayout(layout);
}

void TransactionManager::setupConnections() {
    connect(transferButton, &QPushButton::clicked, this, &TransactionManager::performTransaction);
    connect(findChild<QPushButton*>("Show Transaction History"), &QPushButton::clicked, this, &TransactionManager::showTransactionHistory);
}

void TransactionManager::setUserAccess(const QString &username, bool isAdmin) {
    currentUser = username;
    isAdminUser = isAdmin;

    if (!isAdminUser) {
        // Pre-fill the source account with the user's account ID
        QString userAccountId = getUserAccountId(username);
        sourceInput->setText(userAccountId);
        sourceInput->setReadOnly(true);
        updateTransactionHistory(userAccountId);
    } else {
        sourceInput->clear();
        sourceInput->setReadOnly(false);
    }
    
    destInput->clear();
    amountInput->clear();
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

    QSqlQuery query;
    query.prepare("SELECT * FROM accounts WHERE id = :id");
    
    // Check source account
    query.bindValue(":id", sourceId);
    if (!query.exec() || !query.next()) {
        statusLabel->setText("Error: Invalid source account ID.");
        return;
    }
    double sourceBalance = query.value("balance").toDouble();
    
    // Check destination account
    query.bindValue(":id", destId);
    if (!query.exec() || !query.next()) {
        statusLabel->setText("Error: Invalid destination account ID.");
        return;
    }
    
    // Perform transaction
    if (sourceBalance < amount) {
        statusLabel->setText("Error: Insufficient funds in source account.");
        return;
    }
    
    // Update source account
    query.prepare("UPDATE accounts SET balance = balance - :amount WHERE id = :id");
    query.bindValue(":amount", amount);
    query.bindValue(":id", sourceId);
    if (!query.exec()) {
        statusLabel->setText("Error: Failed to update source account.");
        return;
    }
    
    // Update destination account
    query.prepare("UPDATE accounts SET balance = balance + :amount WHERE id = :id");
    query.bindValue(":amount", amount);
    query.bindValue(":id", destId);
    if (!query.exec()) {
        statusLabel->setText("Error: Failed to update destination account.");
        return;
    }
    
    // Record transaction
    query.prepare("INSERT INTO transactions (account_id, amount, type, date) VALUES (:source_id, :amount, :type, :date)");
    query.bindValue(":source_id", sourceId);
    query.bindValue(":amount", -amount);
    query.bindValue(":type", "TRANSFER");
    query.bindValue(":date", QDateTime::currentDateTime().toString(Qt::ISODate));
    if (!query.exec()) {
        qDebug() << "Error inserting source transaction:" << query.lastError().text();
    }
    
    query.prepare("INSERT INTO transactions (account_id, amount, type, date) VALUES (:dest_id, :amount, :type, :date)");
    query.bindValue(":dest_id", destId);
    query.bindValue(":amount", amount);
    query.bindValue(":type", "TRANSFER");
    query.bindValue(":date", QDateTime::currentDateTime().toString(Qt::ISODate));
    if (!query.exec()) {
        qDebug() << "Error inserting destination transaction:" << query.lastError().text();
    }

    statusLabel->setText("Transaction completed successfully.");
    
    // Clear inputs
    if (!isAdminUser) {
        sourceInput->clear();
    }
    destInput->clear();
    amountInput->clear();
    
    // Update account list
    emit transactionCompleted();
}

void TransactionManager::clearData() {
    sourceInput->clear();
    destInput->clear();
    amountInput->clear();
    statusLabel->clear();
    transactionHistoryTextEdit->clear();
}

void TransactionManager::updateAccountList() {
    // This function should update the account list in the UI
    // You might want to emit a signal to notify AccountManager to update its list
    emit transactionCompleted();
}

void TransactionManager::showTransactionHistory() {
    QString accountId = sourceInput->text();
    if (accountId.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a source account ID to view transaction history.");
        return;
    }
    updateTransactionHistory(accountId);
}

void TransactionManager::updateTransactionHistory(const QString &accountId) {
    QSqlQuery query;
    query.prepare("SELECT * FROM transactions WHERE account_id = :account_id ORDER BY date DESC LIMIT 10");
    query.bindValue(":account_id", accountId);

    if (query.exec()) {
        QString history = QString("Transaction History for Account %1:\n\n").arg(accountId);
        while (query.next()) {
            QString date = query.value("date").toString();
            double amount = query.value("amount").toDouble();
            QString type = query.value("type").toString();

            history += QString("%1 | %2 | $%3\n")
                           .arg(date)
                           .arg(type)
                           .arg(qAbs(amount), 0, 'f', 2);
        }
        transactionHistoryTextEdit->setText(history);
    } else {
        qDebug() << "Error fetching transaction history:" << query.lastError().text();
        transactionHistoryTextEdit->setText("Error fetching transaction history.");
    }
}