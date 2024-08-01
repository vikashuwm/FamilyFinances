#include "AccountManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
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
#include <QMenu>
#include <QGroupBox>
#include <QTableWidget>
#include <QListWidget>

AccountManager::AccountManager(Bank *bank, QWidget *parent)
    : QWidget(parent), bank(bank), isAdminUser(false) {
    setupUI();
    loadAccountsFromDatabase();
}

AccountManager::~AccountManager() {
    qDeleteAll(allAccounts);
}

void AccountManager::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // User button setup
    userButton = new QPushButton(this);
    userButton->setObjectName("userButton");
    userButton->setText("User ▼");
    userButton->setCursor(Qt::PointingHandCursor);
    userButton->setStyleSheet(
        "QPushButton#userButton {"
        "    color: white;"
        "    background-color: #007BFF;"
        "    border: none;"
        "    border-radius: 4px;"
        "    padding: 8px 12px;"
        "    font-size: 14px;"
        "}"
        "QPushButton#userButton:hover {"
        "    background-color: #0056b3;"
        "}"
    );

    connect(userButton, &QPushButton::clicked, this, &AccountManager::showUserMenu);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->addStretch();
    headerLayout->addWidget(userButton);
    mainLayout->addLayout(headerLayout);

    // Admin View: Account Table
    accountTable = new QTableWidget(this);
    accountTable->setObjectName("accountTable");
    accountTable->setColumnCount(3);
    accountTable->setHorizontalHeaderLabels({"Account ID", "Owner", "Balance"});
    accountTable->horizontalHeader()->setStretchLastSection(true);
    accountTable->setStyleSheet(
        "QTableWidget { border: 1px solid #CCCCCC; border-radius: 4px; }"
        "QHeaderView::section { background-color: #333333; color: white; padding: 5px; }"
        "QTableWidget::item { padding: 5px; }"
    );
    connect(accountTable, &QTableWidget::cellClicked, this, &AccountManager::showAccountDetails);
    
    mainLayout->addWidget(accountTable);

    // User View: Account Details and Transactions
    userViewWidget = new QWidget(this);
    QVBoxLayout *userViewLayout = new QVBoxLayout(userViewWidget);

    // User Info Box
    QGroupBox *userInfoBox = new QGroupBox("User Information", userViewWidget);
    QVBoxLayout *userInfoLayout = new QVBoxLayout(userInfoBox);

    accountNameLabel = new QLabel(userInfoBox);
    accountIdLabel = new QLabel(userInfoBox);
    accountBalanceLabel = new QLabel(userInfoBox);
    accountEmailLabel = new QLabel(userInfoBox);

    userInfoLayout->addWidget(accountNameLabel);
    userInfoLayout->addWidget(accountIdLabel);
    userInfoLayout->addWidget(accountBalanceLabel);
    userInfoLayout->addWidget(accountEmailLabel);

    userViewLayout->addWidget(userInfoBox);

    // Transaction History Box
    QGroupBox *transactionBox = new QGroupBox("Recent Transactions", userViewWidget);
    QVBoxLayout *transactionLayout = new QVBoxLayout(transactionBox);

    transactionList = new QListWidget(transactionBox);
    transactionList->setStyleSheet(
        "QListWidget { border: 1px solid #CCCCCC; border-radius: 4px; }"
        "QListWidget::item { padding: 5px; }"
    );
    transactionLayout->addWidget(transactionList);

    userViewLayout->addWidget(transactionBox);

    mainLayout->addWidget(userViewWidget);

    // Set the layout for the main widget
    setLayout(mainLayout);

    // Initially hide both views
    accountTable->hide();
    userViewWidget->hide();
}

void AccountManager::showUserMenu() {
    QMenu *menu = new QMenu(this);
    menu->setStyleSheet("QMenu {"
                        "    background-color: #f8f9fa;"
                        "    border: 2px solid #007BFF;"
                        "    border-radius: 10px;"
                        "    padding: 5px;"
                        "}"
                        "QMenu::item {"
                        "    padding: 8px 20px;"
                        "    border-radius: 5px;"
                        "}"
                        "QMenu::item:selected {"
                        "    background-color: #007BFF;"
                        "    color: white;"
                        "}");

    if (isAdminUser) {
        QAction *addAccountAction = new QAction("Add/Update Account", this); // same username then update will be performed.
        connect(addAccountAction, &QAction::triggered, this, &AccountManager::showCreateAccountForm);
        menu->addAction(addAccountAction);
    }

    QAction *logoutAction = new QAction("Logout", this);
    connect(logoutAction, &QAction::triggered, this, &AccountManager::logout);
    menu->addAction(logoutAction);

    menu->popup(userButton->mapToGlobal(userButton->rect().bottomLeft()));

    connect(menu, &QMenu::aboutToHide, menu, &QObject::deleteLater);
}


void AccountManager::logout() {
    emit logoutRequested();
}

void AccountManager::loadAccountsFromDatabase() {
    allAccounts.clear();
    QSqlQuery query("SELECT * FROM accounts");
    while (query.next()) {
        QString id = query.value("id").toString();
        std::string owner = query.value("owner").toString().toStdString();
        std::string username = query.value("username").toString().toStdString();
        QString email = query.value("email").toString();
        QString password = query.value("password").toString();
        double balance = query.value("balance").toDouble();
        bool isAdmin = query.value("is_admin").toBool();

        Money current = Money::fromDollars(balance);
        Money minimum = Money::fromDollars(0);

        Account* account = new Account(owner, id.toStdString(), minimum, current);
        account->setUsername(username);
        account->setEmail(email.toStdString());
        account->setPassword(password.toStdString());
        account->setIsAdmin(isAdmin);
        allAccounts.append(account);
    }
    updateAccountList();
}

void AccountManager::updateAccountList() {
    accountTable->clearContents();
    accountTable->setRowCount(0);

    for (const auto& account : allAccounts) {
        QString accountId = QString::fromStdString(account->getID());
        QString owner = QString::fromStdString(account->getOwner());
        
        if (account->isAdmin()) {
            continue;
        }

        if (isAdminUser || (owner.toLower() == currentUser.toLower())) {
            int row = accountTable->rowCount();
            accountTable->insertRow(row);

            QTableWidgetItem* idItem = new QTableWidgetItem(accountId);
            QTableWidgetItem* ownerItem = new QTableWidgetItem(owner);
            QTableWidgetItem* balanceItem = new QTableWidgetItem(QString::number(account->getCurrent().getDollars(), 'f', 2));

            accountTable->setItem(row, 0, idItem);
            accountTable->setItem(row, 1, ownerItem);
            accountTable->setItem(row, 2, balanceItem);
        }
    }
}

void AccountManager::setUserAccess(const QString &username, bool isAdmin) {
    currentUser = username;
    isAdminUser = isAdmin;
    userButton->setText(username);
    loadAccountsFromDatabase();
    
    if (isAdmin) {
        accountTable->show();
        userViewWidget->hide();
        updateAccountList();
    } else {
        accountTable->hide();
        userViewWidget->show();
        displayAccountDetails(QString::fromStdString(getCurrentUserAccountId()));
    }
}

std::string AccountManager::getCurrentUserAccountId() {
    for (const auto& account : allAccounts) {
        if (QString::fromStdString(account->getUsername()).toLower() == currentUser.toLower()) {
            return account->getID();
        }
    }
    return "";
}

void AccountManager::displayAccountDetails(const QString &accountId) {
    QSqlQuery accountQuery;
    accountQuery.prepare("SELECT * FROM accounts WHERE id = :account_id");
    accountQuery.bindValue(":account_id", accountId);

    if (accountQuery.exec() && accountQuery.next()) {
        QString owner = accountQuery.value("owner").toString();
        QString email = accountQuery.value("email").toString();
        double balance = accountQuery.value("balance").toDouble();

        accountNameLabel->setText(owner);
        accountIdLabel->setText("Account ID: " + accountId);
        accountBalanceLabel->setText(QString("Current Balance: $%1").arg(balance, 0, 'f', 2));
        accountEmailLabel->setText("Email: " + email);

        QSqlQuery transactionQuery;
        transactionQuery.prepare("SELECT * FROM transactions WHERE account_id = :account_id ORDER BY date DESC LIMIT 10");
        transactionQuery.bindValue(":account_id", accountId);

        transactionList->clear();
        if (transactionQuery.exec()) {
            while (transactionQuery.next()) {
                QString date = transactionQuery.value("date").toString();
                double amount = transactionQuery.value("amount").toDouble();
                QString type = transactionQuery.value("type").toString();

                QString transactionText = QString("%1 | %2 | $%3")
                                              .arg(date)
                                              .arg(type)
                                              .arg(qAbs(amount), 0, 'f', 2);
                QListWidgetItem *item = new QListWidgetItem(transactionText);
                
                if (amount < 0) {
                    item->setForeground(Qt::red);
                    transactionText = "- " + transactionText;
                } else {
                    item->setForeground(Qt::darkGreen);
                    transactionText = "+ " + transactionText;
                }
                
                item->setText(transactionText);
                transactionList->addItem(item);
            }
        }

        if (isAdminUser) {
            userViewWidget->show();
        }
    } else {
        accountNameLabel->setText("No account found");
        accountIdLabel->clear();
        accountBalanceLabel->clear();
        accountEmailLabel->clear();
        transactionList->clear();
    }
}

void AccountManager::showAccountDetails(int row, int column) {
    Q_UNUSED(column);
    QString accountId = accountTable->item(row, 0)->text();
    displayAccountDetails(accountId);
}

void AccountManager::showCreateAccountForm() {
    if (isAdminUser) {
        QDialog* dialog = setupAccountCreationDialog();
        if (dialog->exec() == QDialog::Accepted) {
            loadAccountsFromDatabase();
            updateAccountList();
        }
        delete dialog;
    } else {
        QMessageBox::warning(this, "Permission Denied", "Only admin users can create new accounts.");
    }
}
QDialog* AccountManager::setupAccountCreationDialog() {
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Create/Update Account");
    dialog->setFixedSize(400, 450);

    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 20, 20, 20);

    QStringList placeholders = {"First Name", "Last Name", "Username", "Email Address", "Initial Balance"};
    QList<QLineEdit*> inputs;
    for (const QString& placeholder : placeholders) {
        QLineEdit* input = new QLineEdit(dialog);
        input->setPlaceholderText(placeholder);
        input->setStyleSheet(
            "QLineEdit {"
            "    border: 1px solid #BDC3C7;"
            "    border-radius: 4px;"
            "    padding: 8px;"
            "    font-size: 14px;"
            "}"
            "QLineEdit:focus {"
            "    border-color: #3498DB;"
            "}"
        );
        layout->addWidget(input);
        inputs.append(input);
    }

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* createButton = new QPushButton("Create", dialog);
    QPushButton* cancelButton = new QPushButton("Cancel", dialog);

    QString buttonStyle = 
        "QPushButton {"
        "    background-color: #3498DB;"
        "    color: white;"
        "    border: none;"
        "    padding: 8px 16px;"
        "    border-radius: 4px;"
        "    font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2980B9;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #2473A6;"
        "}";

    createButton->setStyleSheet(buttonStyle);
    cancelButton->setStyleSheet(buttonStyle);

    buttonLayout->addWidget(createButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    connect(createButton, &QPushButton::clicked, [dialog, inputs, this]() {
        QString firstName = inputs[0]->text().trimmed();
        QString lastName = inputs[1]->text().trimmed();
        QString username = inputs[2]->text().trimmed();
        QString email = inputs[3]->text().trimmed();
        QString initialBalanceStr = inputs[4]->text().trimmed();

        if (firstName.isEmpty() || lastName.isEmpty() || username.isEmpty() || email.isEmpty() || initialBalanceStr.isEmpty()) {
            QMessageBox::warning(dialog, "Input Error", "Please fill in all fields.");
            return;
        }

        bool ok;
        double initialBalance = initialBalanceStr.toDouble(&ok);
        if (!ok || initialBalance < 0) {
            QMessageBox::warning(dialog, "Input Error", "Please enter a valid initial balance.");
            return;
        }

        QString accountId = generateUniqueAccountId();
        std::string owner = (firstName + " " + lastName).toStdString();
        Money initial = Money::fromDollars(initialBalance);
        Money minimum = Money::fromDollars(0);

        Account* newAccount = new Account(owner, accountId.toStdString(), minimum, initial);
        newAccount->setUsername(username.toStdString());
        newAccount->setEmail(email.toStdString());
        
        QString password = QString::fromStdString(bank->generatePassword(owner, accountId.toStdString()));
        newAccount->setPassword(password.toStdString());
        newAccount->setIsAdmin(false);

        saveAccountToDatabase(newAccount);

        QString message = QString("Account created successfully!\n\nAccount ID: %1\nUsername: %2\nPassword: %3")
                              .arg(accountId)
                              .arg(username)
                              .arg(password);
        QMessageBox::information(dialog, "Account Created", message);

        dialog->accept();
    });

    connect(cancelButton, &QPushButton::clicked, dialog, &QDialog::reject);

    return dialog;
}

void AccountManager::saveAccountToDatabase(const Account* account) {
    if (account->getOwner().empty() || account->getID().empty() || account->getUsername().empty()) {
        qDebug() << "Owner, ID, or Username cannot be empty.";
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO accounts (id, owner, username, email, password, balance, is_admin) "
                  "VALUES (:id, :owner, :username, :email, :password, :balance, :is_admin)");
    query.bindValue(":id", QString::fromStdString(account->getID()));
    query.bindValue(":owner", QString::fromStdString(account->getOwner()));
    query.bindValue(":username", QString::fromStdString(account->getUsername()));
    query.bindValue(":email", QString::fromStdString(account->getEmail()));
    query.bindValue(":password", QString::fromStdString(account->getPassword()));
    query.bindValue(":balance", account->getCurrent().getDollars());
    query.bindValue(":is_admin", account->isAdmin());

    if (!query.exec()) {
        qDebug() << "Error saving account:" << query.lastError().text();
    }
}

QString AccountManager::generateUniqueAccountId() {
    QDateTime now = QDateTime::currentDateTime();
    return now.toString("yyyyMMddHHmmsszzz");
}

void AccountManager::clearData() {
    accountTable->clearContents();
    accountTable->setRowCount(0);
    accountNameLabel->clear();
    accountIdLabel->clear();
    accountBalanceLabel->clear();
    accountEmailLabel->clear();
    transactionList->clear();
    qDeleteAll(allAccounts);
    allAccounts.clear();
}

void AccountManager::onTransactionCompleted() {
    loadAccountsFromDatabase();
    
    if (isAdminUser) {
        updateAccountList();
    } else {
        displayAccountDetails(QString::fromStdString(getCurrentUserAccountId()));
    }
}