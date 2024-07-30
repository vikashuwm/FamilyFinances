// AccountManager.cpp
#include "AccountManager.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDateTime>

AccountManager::AccountManager(Bank *bank, QWidget *parent)
    : QWidget(parent), bank(bank), isAdminUser(false) {
    setupUI();
}

void AccountManager::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);

    createAccountButton = new QPushButton("Create Account", this);
    createAccountButton->setObjectName("createAccountButton");
    createAccountButton->setStyleSheet("background-color: #007BFF; color: white; border-radius: 15px; padding: 10px 20px;");
    connect(createAccountButton, &QPushButton::clicked, this, &AccountManager::showCreateAccountForm);
    layout->addWidget(createAccountButton, 0, Qt::AlignRight);

    accountTable = new QTableWidget(this);
    accountTable->setObjectName("accountTable");
    accountTable->setColumnCount(3);
    accountTable->setHorizontalHeaderLabels({"Account ID", "Owner", "Balance"});
    accountTable->horizontalHeader()->setStretchLastSection(true);
    accountTable->setStyleSheet("QTableWidget { border: 1px solid #CCCCCC; border-radius: 10px; }"
                                "QHeaderView::section { background-color: #333333; color: white; padding: 5px; }");
    layout->addWidget(accountTable);

    setLayout(layout);
}

void AccountManager::setUserAccess(const QString &username, bool isAdmin) {
    currentUser = username;
    isAdminUser = isAdmin;
    createAccountButton->setVisible(isAdmin);
    updateAccountList();
}

void AccountManager::showCreateAccountForm() {
    QDialog* dialog = setupAccountCreationDialog();
    dialog->exec();
    delete dialog;
}

void AccountManager::updateAccountList() {
    accountTable->clearContents();
    accountTable->setRowCount(0);

    const auto& accounts = bank->getAccounts();
    for (const auto& account : accounts) {
        int row = accountTable->rowCount();
        accountTable->insertRow(row);

        QTableWidgetItem* idItem = new QTableWidgetItem(QString::fromStdString(account->getID()));
        QTableWidgetItem* ownerItem = new QTableWidgetItem(QString::fromStdString(account->getOwner()));
        QTableWidgetItem* balanceItem = new QTableWidgetItem(QString::fromStdString(account->getCurrent().toString()));

        accountTable->setItem(row, 0, idItem);
        accountTable->setItem(row, 1, ownerItem);
        accountTable->setItem(row, 2, balanceItem);
    }
}

QDialog* AccountManager::setupAccountCreationDialog() {
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Create Account");
    dialog->setObjectName("createAccountDialog");
    dialog->setFixedSize(400, 400);

    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 20, 20, 20);

    QLabel* titleLabel = new QLabel("Create New Account", dialog);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    QStringList placeholders = {"First Name", "Last Name", "Email Address", "Initial Balance"};
    QList<QLineEdit*> inputs;
    for (const QString& placeholder : placeholders) {
        QLineEdit* input = new QLineEdit(dialog);
        input->setPlaceholderText(placeholder);
        input->setObjectName(placeholder.toLower().replace(" ", "") + "Input");
        layout->addWidget(input);
        inputs.append(input);
    }

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* createButton = new QPushButton("Create", dialog);
    createButton->setObjectName("createButton");
    QPushButton* cancelButton = new QPushButton("Cancel", dialog);
    cancelButton->setObjectName("cancelButton");

    buttonLayout->addWidget(createButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    connect(createButton, &QPushButton::clicked, [dialog, inputs, this]() {
        QString firstName = inputs[0]->text().trimmed();
        QString lastName = inputs[1]->text().trimmed();
        QString email = inputs[2]->text().trimmed();
        QString initialBalanceStr = inputs[3]->text().trimmed();

        if (firstName.isEmpty() || lastName.isEmpty() || email.isEmpty() || initialBalanceStr.isEmpty()) {
            QMessageBox::warning(dialog, "Input Error", "Please fill in all fields.");
            return;
        }

        bool ok;
        double initialBalance = initialBalanceStr.toDouble(&ok);
        if (!ok || initialBalance <= 0) {
            QMessageBox::warning(dialog, "Input Error", "Please enter a valid initial balance.");
            return;
        }

        QString accountId = generateUniqueAccountId();
        std::string owner = (firstName + " " + lastName).toStdString();
        Money minBalance = Money::fromDollars(0);  // Set minimum balance as needed
        Money initial = Money::fromDollars(initialBalance);

        auto account = bank->open(owner, accountId.toStdString(), minBalance, initial);
        QString password = QString::fromStdString(bank->generatePassword(owner, accountId.toStdString()));

        QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/user_accounts.cfg");
        if (file.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&file);
            out << "Account ID: " << accountId << "\n";
            out << "Owner: " << QString::fromStdString(owner) << "\n";
            out << "Balance: " << QString::fromStdString(initial.toString()) << "\n";
            out << "Password: " << password << "\n\n";
            file.close();
        } else {
            QMessageBox::warning(dialog, "File Error", "Failed to save account details.");
        }

        dialog->accept();
        updateAccountList();
    });

    connect(cancelButton, &QPushButton::clicked, dialog, &QDialog::reject);

    return dialog;
}

QString AccountManager::generateUniqueAccountId() {
    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString("yyyyMMddHHmmsszzz");  // Format the date and time
    return timestamp;
}

void AccountManager::saveAllAccountsToFile() {
    QString filePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/user_accounts.cfg";
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file:" << file.errorString();
        return;
    }

    QTextStream out(&file);

    for (const auto& account : bank->getAccounts()) {
        out << "Account ID: " << QString::fromStdString(account->getID()) << "\n";
        out << "Owner: " << QString::fromStdString(account->getOwner()) << "\n";
        out << "Balance: " << QString::fromStdString(account->getCurrent().toString()) << "\n";
        
        // Get last 5 transactions for this account
        const auto transactions = account->getLastTransactions(5);
        int transactionCount = transactions.size();
        
        out << "Last " << transactionCount << " Transactions:\n";
        for (const auto& transaction : transactions) {
            out << "  - Date: " << QString::fromStdString(transaction.getDate()) << "\n";
            out << "    Amount: " << QString::fromStdString(transaction.getAmount().toString()) << "\n";
            out << "    Type: " << (transaction.getType() == Transaction::Type::DEPOSIT ? "Deposit" : 
                                    (transaction.getType() == Transaction::Type::WITHDRAWAL ? "Withdrawal" : "Transfer")) << "\n";
        }
        out << "\n";
    }

    file.close();
}