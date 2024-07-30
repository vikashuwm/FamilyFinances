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
#include <QJsonDocument>
#include <QJsonObject>

const QString AccountManager::ACCOUNT_FILE_PATH = "/Users/vikashkumar/FamilyFinances/resources/user_accounts.cfg";
const QString AccountManager::USER_CONFIG_PATH = "/Users/vikashkumar/FamilyFinances/resources/user.cfg";

AccountManager::AccountManager(Bank *bank, QWidget *parent)
    : QWidget(parent), bank(bank), isAdminUser(false) {
    setupUI();
    loadAccountsFromFile();
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

void AccountManager::loadUserAccounts() {
    QFile file(USER_CONFIG_PATH);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open user.cfg:" << file.errorString();
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(data));
    userAccounts = doc.object();

    file.close();
}

bool AccountManager::isUserAuthorizedForAccount(const QString& accountId) {
    if (isAdminUser) return true;
    
    // For regular users, check if their username matches the account owner
    for (const auto& account : allAccounts) {
        if (QString::fromStdString(account->getID()) == accountId) {
            return QString::fromStdString(account->getOwner()).toLower() == currentUser.toLower();
        }
    }
    return false;
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

void AccountManager::setUserAccess(const QString &username, bool isAdmin) {
    currentUser = username;
    isAdminUser = isAdmin;
    createAccountButton->setVisible(isAdmin);
    accountTable->setVisible(isAdmin);
    
    qDebug() << "Setting user access for:" << username << "isAdmin:" << isAdmin;

    if (isAdmin) {
        loadUserAccounts();
        updateAccountList();
    } else {
        displayUserInfo();
    }

    // Ensure the layout adjusts to show/hide widgets
    layout()->activate();
}

void AccountManager::showCreateAccountForm() {
    QDialog* dialog = setupAccountCreationDialog();
    if (dialog->exec() == QDialog::Accepted) {
        loadAccountsFromFile();
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

        std::shared_ptr<Account> accountPtr = bank->open(owner, accountId.toStdString(), minBalance, initial);
        if (accountPtr) {
            allAccounts.append(accountPtr.get());  // Add the raw pointer to allAccounts
        } else {
            QMessageBox::warning(dialog, "Account Creation Error", "Failed to create the account.");
            return;
        }

        QString password = QString::fromStdString(bank->generatePassword(owner, accountId.toStdString()));

        // Save the new account information to the account file
        saveAllAccountsToFile();

        // Save the user credentials to the user config file
        QFile userFile(USER_CONFIG_PATH);
        if (userFile.open(QIODevice::ReadWrite)) {
            QByteArray userData = userFile.readAll();
            QJsonDocument doc = QJsonDocument::fromJson(userData);
            QJsonObject userJson = doc.object();

            userJson[accountId] = password;

            userFile.seek(0);
            userFile.write(QJsonDocument(userJson).toJson());
            userFile.close();
        } else {
            QMessageBox::warning(dialog, "File Error", "Failed to save user credentials.");
        }

        dialog->accept();
        
        // Refresh the account list
        loadAccountsFromFile();
        updateAccountList();
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
    QFile file(ACCOUNT_FILE_PATH);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file:" << file.errorString();
        return;
    }

    QTextStream in(&file);
    QString line;
    bool foundAccount = false;
    QString transactionHistory;

    while (!in.atEnd()) {
        line = in.readLine();
        if (line.startsWith("Account ID: " + accountId)) {
            foundAccount = true;
            transactionHistory = "Transaction History for Account " + accountId + ":\n\n";
        } else if (foundAccount && line.startsWith("Last")) {
            while (!in.atEnd() && !line.isEmpty()) {
                transactionHistory += line + "\n";
                line = in.readLine();
            }
            break;
        }
    }

    file.close();

    if (foundAccount) {
        transactionHistoryTextEdit->setPlainText(transactionHistory);
    } else {
        transactionHistoryTextEdit->setPlainText("No transaction history found for Account " + accountId);
    }
}

QString AccountManager::generateUniqueAccountId() {
    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString("yyyyMMddHHmmsszzz");  // Format: YearMonthDayHourMinuteSecondMillisecond
    return timestamp;
}

void AccountManager::loadAccountsFromFile() {
    QFile file(ACCOUNT_FILE_PATH);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file:" << file.errorString();
        return;
    }

    QTextStream in(&file);
    QString accountId, owner, balanceStr;
    Money balance, minBalance;

    allAccounts.clear();

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith("Account ID: ")) {
            accountId = line.mid(12).trimmed();
        } else if (line.startsWith("Owner: ")) {
            owner = line.mid(7).trimmed();
        } else if (line.startsWith("Balance: ")) {
            balanceStr = line.mid(9).trimmed();
            balanceStr.remove("$").remove(",");
            bool ok;
            double balanceValue = balanceStr.toDouble(&ok);
            if (ok) {
                balance = Money::fromDollars(balanceValue);
                minBalance = Money::fromDollars(0);  // Assuming minimum balance is 0
                allAccounts.append(new Account(owner.toStdString(), accountId.toStdString(), minBalance, balance));
            } else {
                qDebug() << "Failed to parse balance:" << balanceStr;
            }
        }
    }

    file.close();
}

void AccountManager::saveAllAccountsToFile() {
    QFile file(ACCOUNT_FILE_PATH);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "Failed to open file for reading and writing:" << file.errorString();
        return;
    }

    // Read existing accounts
    QTextStream in(&file);
    QMap<QString, QString> existingAccounts;
    QString currentAccountId;
    QString accountData;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith("Account ID: ")) {
            if (!currentAccountId.isEmpty()) {
                existingAccounts[currentAccountId] = accountData;
                accountData.clear();
            }
            currentAccountId = line.mid(12).trimmed();
        }
        accountData += line + "\n";
    }
    if (!currentAccountId.isEmpty()) {
        existingAccounts[currentAccountId] = accountData;
    }

    // Rewind file and clear it
    file.seek(0);
    file.resize(0);

    QTextStream out(&file);

    // Write all accounts, updating or adding new ones
    for (const auto& account : bank->getAccounts()) {
        QString accountId = QString::fromStdString(account->getID());
        if (existingAccounts.contains(accountId)) {
            out << existingAccounts[accountId];
        } else {
            out << "Account ID: " << accountId << "\n";
            out << "Owner: " << QString::fromStdString(account->getOwner()) << "\n";
            out << "Balance: " << QString::fromStdString(account->getCurrent().toString()) << "\n";
            
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
        existingAccounts.remove(accountId);
    }

    // Write any remaining existing accounts that weren't in bank->getAccounts()
    for (const auto& accountData : existingAccounts) {
        out << accountData;
    }

    file.close();
}

void AccountManager::displayUserInfo() {
    QFile file(ACCOUNT_FILE_PATH);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file:" << file.errorString();
        transactionHistoryTextEdit->setPlainText("Error: Failed to open account file.");
        return;
    }

    QTextStream in(&file);
    QString accountInfo;
    bool foundAccount = false;
    QString currentLine;

    while (!in.atEnd()) {
        currentLine = in.readLine();
        if (currentLine.startsWith("Account ID: ") && currentLine.mid(12).trimmed() == currentUser) {
            // If currentUser matches Account ID
            foundAccount = true;
        } else if (currentLine.startsWith("Owner: ") && currentLine.mid(7).trimmed().toLower() == currentUser.toLower()) {
            // If currentUser matches Owner name
            foundAccount = true;
            in.seek(in.pos() - currentLine.length() - 1);  // Go back one line to include Account ID
        }

        if (foundAccount) {
            accountInfo = "Your Account Information:\n\n";
            // Read account information (Account ID, Owner, Balance, and Transactions)
            for (int i = 0; i < 5 && !in.atEnd(); ++i) {
                accountInfo += in.readLine() + "\n";
            }
            break;
        }
    }

    file.close();

    if (foundAccount) {
        transactionHistoryTextEdit->setPlainText(accountInfo);
    } else {
        transactionHistoryTextEdit->setPlainText("No account information found for " + currentUser);
    }
}