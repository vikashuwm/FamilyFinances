#include "AccountManager.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

AccountManager::AccountManager(Bank *bank, QWidget *parent)
    : QWidget(parent), bank(bank), isAdminUser(false) {
    setupDatabase();
    setupUI();
    loadAccountsFromDatabase();
}

AccountManager::~AccountManager() {
    // Clean up any resources if needed
}

void AccountManager::setupDatabase() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/Users/vikashkumar/FamilyFinances/familyfinances.db");
    
    if (!db.open()) {
        qDebug() << "Error: connection with database failed";
    } else {
        qDebug() << "Database: connection ok";
    }

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS accounts ("
               "id TEXT PRIMARY KEY, "
               "owner TEXT, "
               "balance REAL, "
               "minimum_balance REAL)");

    query.exec("CREATE TABLE IF NOT EXISTS transactions ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "account_id TEXT, "
               "amount REAL, "
               "type TEXT, "
               "date TEXT, "
               "FOREIGN KEY (account_id) REFERENCES accounts(id))");

    query.exec("CREATE TABLE IF NOT EXISTS users ("
               "username TEXT PRIMARY KEY, "
               "password TEXT, "
               "is_admin BOOLEAN)");
}

void AccountManager::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    createAccountButton = new QPushButton("Create Account", this);
    createAccountButton->setObjectName("createAccountButton");
    createAccountButton->setStyleSheet("background-color: #007BFF; color: white; border-radius: 15px; padding: 10px 20px;");
    connect(createAccountButton, &QPushButton::clicked, this, &AccountManager::showCreateAccountForm);
    mainLayout->addWidget(createAccountButton, 0, Qt::AlignRight);

    accountTable = new QTableWidget(this);
    accountTable->setObjectName("accountTable");
    accountTable->setColumnCount(3);
    accountTable->setHorizontalHeaderLabels({"Account ID", "Owner", "Balance"});
    accountTable->horizontalHeader()->setStretchLastSection(true);
    accountTable->setStyleSheet("QTableWidget { border: 1px solid #CCCCCC; border-radius: 10px; }"
                                "QHeaderView::section { background-color: #333333; color: white; padding: 5px; }");
    connect(accountTable, &QTableWidget::cellClicked, this, &AccountManager::showTransactionHistory);
    
    int rowHeight = accountTable->verticalHeader()->defaultSectionSize();
    int headerHeight = accountTable->horizontalHeader()->height();
    int tableHeight = (rowHeight * 10) + headerHeight;
    accountTable->setMinimumHeight(tableHeight);
    
    mainLayout->addWidget(accountTable);

    transactionHistoryTextEdit = new QTextEdit(this);
    transactionHistoryTextEdit->setReadOnly(true);
    transactionHistoryTextEdit->setStyleSheet("QTextEdit { border: 1px solid #CCCCCC; border-radius: 10px; padding: 10px; }");
    transactionHistoryTextEdit->setMinimumHeight(200);
    mainLayout->addWidget(transactionHistoryTextEdit);

    setLayout(mainLayout);

    int minWidth = 600;
    int minHeight = tableHeight + createAccountButton->height() + transactionHistoryTextEdit->minimumHeight() + 60;
    setMinimumSize(minWidth, minHeight);
}

void AccountManager::loadAccountsFromDatabase() {
    allAccounts.clear();
    QSqlQuery query("SELECT * FROM accounts");
    while (query.next()) {
        QString id = query.value("id").toString();
        std::string owner = query.value("owner").toString().toStdString();
        double balance = query.value("balance").toDouble();
        double minBalance = query.value("minimum_balance").toDouble();

        Money current = Money::fromDollars(balance);
        Money minimum = Money::fromDollars(minBalance);

        Account* account = new Account(owner, id.toStdString(), minimum, current);
        allAccounts.append(account);
    }
    updateAccountList();
}

void AccountManager::saveAccountToDatabase(const Account* account) {
    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO accounts (id, owner, balance, minimum_balance) "
                  "VALUES (:id, :owner, :balance, :minimum_balance)");
    query.bindValue(":id", QString::fromStdString(account->getID()));
    query.bindValue(":owner", QString::fromStdString(account->getOwner()));
    query.bindValue(":balance", QString::fromStdString(account->getCurrent().toString()));
    query.bindValue(":minimum_balance", QString::fromStdString(account->getMinimum().toString()));

    if (!query.exec()) {
        qDebug() << "Error saving account:" << query.lastError().text();
    }
}

void AccountManager::updateAccountList() {
    accountTable->clearContents();
    accountTable->setRowCount(0);

    for (const auto& account : allAccounts) {
        QString accountId = QString::fromStdString(account->getID());
        QString owner = QString::fromStdString(account->getOwner());
        
        if (isAdminUser || (owner.toLower() == currentUser.toLower())) {
            int row = accountTable->rowCount();
            accountTable->insertRow(row);

            QTableWidgetItem* idItem = new QTableWidgetItem(accountId);
            QTableWidgetItem* ownerItem = new QTableWidgetItem(owner);
            QTableWidgetItem* balanceItem = new QTableWidgetItem(QString::fromStdString(account->getCurrent().toString()));

            accountTable->setItem(row, 0, idItem);
            accountTable->setItem(row, 1, ownerItem);
            accountTable->setItem(row, 2, balanceItem);
        }
    }
}

void AccountManager::showCreateAccountForm() {
    QDialog* dialog = setupAccountCreationDialog();
    if (dialog->exec() == QDialog::Accepted) {
        loadAccountsFromDatabase();
        updateAccountList();
    }
    delete dialog;
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
        Money minBalance = Money::fromDollars(0);
        Money initial = Money::fromDollars(initialBalance);

        Account* newAccount = new Account(owner, accountId.toStdString(), minBalance, initial);
        allAccounts.append(newAccount);
        saveAccountToDatabase(newAccount);

        QString password = QString::fromStdString(bank->generatePassword(owner, accountId.toStdString()));

        // Save the user credentials to the database
        QSqlQuery query;
        query.prepare("INSERT INTO users (username, password, is_admin) VALUES (:username, :password, :is_admin)");
        query.bindValue(":username", email);
        query.bindValue(":password", password);
        query.bindValue(":is_admin", false);

        if (!query.exec()) {
            QMessageBox::warning(dialog, "Database Error", "Failed to save user credentials.");
        }

        dialog->accept();
    });

    connect(cancelButton, &QPushButton::clicked, dialog, &QDialog::reject);

    return dialog;
}

void AccountManager::showTransactionHistory(int row, int column) {
    Q_UNUSED(column);
    QString accountId = accountTable->item(row, 0)->text();
    displayTransactionHistory(accountId);
}

void AccountManager::displayTransactionHistory(const QString &accountId) {
    QSqlQuery query;
    query.prepare("SELECT * FROM transactions WHERE account_id = :account_id ORDER BY date DESC LIMIT 5");
    query.bindValue(":account_id", accountId);

    if (query.exec()) {
        QString transactionHistory = "Transaction History for Account " + accountId + ":\n\n";
        while (query.next()) {
            QString date = query.value("date").toString();
            QString amount = query.value("amount").toString();
            QString type = query.value("type").toString();

            transactionHistory += QString("Date: %1\nAmount: $%2\nType: %3\n\n")
                                      .arg(date)
                                      .arg(amount)
                                      .arg(type);
        }
        transactionHistoryTextEdit->setPlainText(transactionHistory);
    } else {
        transactionHistoryTextEdit->setPlainText("No transaction history found for Account " + accountId);
    }
}

QString AccountManager::generateUniqueAccountId() {
    QDateTime now = QDateTime::currentDateTime();
    return now.toString("yyyyMMddHHmmsszzz");  // Format: YearMonthDayHourMinuteSecondMillisecond
}

void AccountManager::setUserAccess(const QString &username, bool isAdmin) {
    currentUser = username;
    isAdminUser = isAdmin;
    createAccountButton->setVisible(isAdmin);
    updateAccountList();
}