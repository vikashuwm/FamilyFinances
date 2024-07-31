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
#include <QTableWidget>
#include <QListWidgetItem>

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
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(0);
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    userButton = new QPushButton(this);
    userButton->setObjectName("userButton");
    userButton->setText("User");
    userButton->setCursor(Qt::PointingHandCursor);

    QLabel *arrowLabel = new QLabel(this);
    arrowLabel->setObjectName("arrowLabel");
    arrowLabel->setText("\u25BC"); // Unicode for downward triangle

    buttonLayout->addWidget(userButton);
    buttonLayout->addWidget(arrowLabel);

    QWidget *buttonContainer = new QWidget(this);
    buttonContainer->setLayout(buttonLayout);
    buttonContainer->setObjectName("buttonContainer");

    QString buttonStyle = 
        "QWidget#buttonContainer {"
        "    background-color: #007BFF;"
        "    border-radius: 4px;"
        "    border: 1px solid #0056b3;"
        "}"
        "QWidget#buttonContainer:hover {"
        "    background-color: #0056b3;"
        "}"
        "QPushButton#userButton {"
        "    color: white;"
        "    background-color: transparent;"
        "    border: none;"
        "    padding: 5px 10px 5px 10px;"
        "    text-align: left;"
        "    font-size: 14px;"
        "}"
        "QLabel#arrowLabel {"
        "    color: white;"
        "    background-color: transparent;"
        "    padding: 5px 8px 5px 0px;"
        "    font-size: 10px;"
        "}";

    buttonContainer->setStyleSheet(buttonStyle);

    connect(userButton, &QPushButton::clicked, this, &AccountManager::showUserMenu);
    connect(arrowLabel, &QLabel::linkActivated, this, &AccountManager::showUserMenu);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addStretch();
    topLayout->addWidget(buttonContainer);
    mainLayout->addLayout(topLayout);

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
    
    int rowHeight = accountTable->verticalHeader()->defaultSectionSize();
    int headerHeight = accountTable->horizontalHeader()->height();
    int tableHeight = (rowHeight * 10) + headerHeight;
    accountTable->setMinimumHeight(tableHeight);
    
    mainLayout->addWidget(accountTable);

    // User View: Account Details
    accountDetailsWidget = new QWidget(this);
    QVBoxLayout *detailsLayout = new QVBoxLayout(accountDetailsWidget);
    
    accountNameLabel = new QLabel(accountDetailsWidget);
    accountNameLabel->setAlignment(Qt::AlignCenter);
    accountNameLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #333333; margin-bottom: 10px;");
    detailsLayout->addWidget(accountNameLabel);

    accountIdLabel = new QLabel(accountDetailsWidget);
    accountIdLabel->setAlignment(Qt::AlignCenter);
    accountIdLabel->setStyleSheet("font-size: 16px; color: #666666; margin-bottom: 20px;");
    detailsLayout->addWidget(accountIdLabel);

    QHBoxLayout *balanceLayout = new QHBoxLayout();
    QLabel *balanceTitle = new QLabel("Current Balance:", accountDetailsWidget);
    balanceTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #333333;");
    balanceLayout->addWidget(balanceTitle);

    accountBalanceLabel = new QLabel(accountDetailsWidget);
    accountBalanceLabel->setStyleSheet("font-size: 18px; color: #28a745; font-weight: bold;");
    balanceLayout->addWidget(accountBalanceLabel);
    detailsLayout->addLayout(balanceLayout);

    accountEmailLabel = new QLabel(accountDetailsWidget);
    accountEmailLabel->setAlignment(Qt::AlignCenter);
    accountEmailLabel->setStyleSheet("font-size: 16px; color: #666666; margin-top: 10px;");
    detailsLayout->addWidget(accountEmailLabel);

    // Transaction history
    transactionHistoryLabel = new QLabel("Recent Transactions:", accountDetailsWidget);
    transactionHistoryLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #333333; margin-top: 20px;");
    detailsLayout->addWidget(transactionHistoryLabel);

    transactionList = new QListWidget(accountDetailsWidget);
    transactionList->setStyleSheet(
        "QListWidget { border: 1px solid #CCCCCC; border-radius: 4px; }"
        "QListWidget::item { padding: 5px; }"
    );
    detailsLayout->addWidget(transactionList);

    mainLayout->addWidget(accountDetailsWidget);

    // Set the layout for the main widget
    setLayout(mainLayout);

    // Initially hide both views
    accountTable->hide();
    accountDetailsWidget->hide();

    // Set minimum size for the widget
    setMinimumSize(600, tableHeight + buttonContainer->sizeHint().height() + 200);
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
        QAction *addAccountAction = new QAction("Add Account", this);
        connect(addAccountAction, &QAction::triggered, this, &AccountManager::showCreateAccountForm);
        menu->addAction(addAccountAction);
    }

    QAction *logoutAction = new QAction("Logout", this);
    connect(logoutAction, &QAction::triggered, this, &AccountManager::logout);
    menu->addAction(logoutAction);

    // Calculate the position for the menu
    QPoint pos = userButton->mapToGlobal(userButton->rect().bottomLeft());
    pos.setX(pos.x() - menu->sizeHint().width() + userButton->width());

    // Show the menu
    menu->popup(pos);

    // Connect the menu's aboutToHide signal to delete it
    connect(menu, &QMenu::aboutToHide, menu, &QObject::deleteLater);
}


void AccountManager::logout() {
    emit logoutRequested();
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
        Money minimum = Money::fromDollars(0); // Or set an appropriate minimum balance

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
        
        // Skip admin accounts in the UI display
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

void AccountManager::showCreateAccountForm() {
    if (isAdminUser) {
        QDialog* dialog = setupAccountCreationDialog();
        if (dialog->exec() == QDialog::Accepted) {
            loadAccountsFromDatabase();  // Reload accounts from the database
            updateAccountList();  // Update the UI
        }
        delete dialog;
    } else {
        QMessageBox::warning(this, "Permission Denied", "Only admin users can create new accounts.");
    }
}

void AccountManager::clearData() {
    accountTable->clearContents();
    accountTable->setRowCount(0);
    accountDetailsTextEdit->clear();
    qDeleteAll(allAccounts);
    allAccounts.clear();
}

void AccountManager::setUserAccess(const QString &username, bool isAdmin) {
    currentUser = username;
    isAdminUser = isAdmin;
    userButton->setText(username);
    loadAccountsFromDatabase();
    
    if (isAdmin) {
        accountTable->show();
        accountDetailsWidget->hide();
        updateAccountList();
    } else {
        accountTable->hide();
        accountDetailsWidget->show();
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

QDialog* AccountManager::setupAccountCreationDialog() {
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Create Account");
    dialog->setObjectName("createAccountDialog");
    dialog->setFixedSize(400, 450);  // Increased height to accommodate the new field

    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 20, 20, 20);

    QLabel* titleLabel = new QLabel("Create New Account", dialog);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    QStringList placeholders = {"First Name", "Last Name", "Username", "Email Address", "Initial Balance"};
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

        // Show a message box with the account details
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

void AccountManager::showAccountDetails(int row, int column) {
    Q_UNUSED(column);
    QString accountId = accountTable->item(row, 0)->text();
    displayAccountDetails(accountId);
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
        accountBalanceLabel->setText(QString("$%1").arg(balance, 0, 'f', 2));
        accountEmailLabel->setText(email);

        // Fetch and display recent transactions
        QSqlQuery transactionQuery;
        transactionQuery.prepare("SELECT * FROM transactions WHERE account_id = :account_id ORDER BY date DESC LIMIT 5");
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
                
                // Set color based on transaction type
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
    } else {
        accountNameLabel->setText("No account found");
        accountIdLabel->clear();
        accountBalanceLabel->clear();
        accountEmailLabel->clear();
        transactionList->clear();
    }
}

QString AccountManager::generateUniqueAccountId() {
    QDateTime now = QDateTime::currentDateTime();
    return now.toString("yyyyMMddHHmmsszzz");  // Format: YearMonthDayHourMinuteSecondMillisecond
}


void AccountManager::onTransactionCompleted() {
    loadAccountsFromDatabase();
    
    if (isAdminUser) {
        updateAccountList();
    } else {
        displayAccountDetails(QString::fromStdString(getCurrentUserAccountId()));
    }
}