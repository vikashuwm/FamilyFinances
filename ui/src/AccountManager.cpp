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
#include <QMenu>

AccountManager::AccountManager(Bank *bank, QWidget *parent)
    : QWidget(parent), bank(bank), isAdminUser(false) {
    setupDatabase();
    setupUI();
    loadAccountsFromDatabase();
}

AccountManager::~AccountManager() {
    qDeleteAll(allAccounts);
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
               "email TEXT UNIQUE, "
               "password TEXT, "
               "balance REAL, "
               "is_admin BOOLEAN)");

    query.exec("CREATE TABLE IF NOT EXISTS transactions ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "account_id TEXT, "
               "amount REAL, "
               "type TEXT, "
               "date TEXT, "
               "FOREIGN KEY (account_id) REFERENCES accounts(id))");
}

void AccountManager::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Create a horizontal layout for the button
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(0);
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    // Create the main part of the button
    userButton = new QPushButton(this);
    userButton->setObjectName("userButton");
    userButton->setText("Admin"); // Or "User", depending on the logged-in user type
    userButton->setCursor(Qt::PointingHandCursor);

    // Create a label for the dropdown arrow
    QLabel *arrowLabel = new QLabel(this);
    arrowLabel->setObjectName("arrowLabel");
    arrowLabel->setText("\u25BC"); // Unicode for downward triangle

    // Add the button and arrow label to the horizontal layout
    buttonLayout->addWidget(userButton);
    buttonLayout->addWidget(arrowLabel);

    // Create a container widget for the button and arrow
    QWidget *buttonContainer = new QWidget(this);
    buttonContainer->setLayout(buttonLayout);
    buttonContainer->setObjectName("buttonContainer");

    // Set the style sheet for the button components
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

    // Connect the click event to showUserMenu
    connect(userButton, &QPushButton::clicked, this, &AccountManager::showUserMenu);
    connect(arrowLabel, &QLabel::linkActivated, this, &AccountManager::showUserMenu);

    // Add the button container to the main layout
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addStretch();
    topLayout->addWidget(buttonContainer);
    mainLayout->addLayout(topLayout);

    // ... (rest of the setupUI function remains unchanged)

    // Create and set up the account table
    accountTable = new QTableWidget(this);
    accountTable->setObjectName("accountTable");
    accountTable->setColumnCount(3);
    accountTable->setHorizontalHeaderLabels({"Account ID", "Owner", "Balance"});
    accountTable->horizontalHeader()->setStretchLastSection(true);
    accountTable->setStyleSheet("QTableWidget { border: 1px solid #CCCCCC; border-radius: 4px; }"
                                "QHeaderView::section { background-color: #333333; color: white; padding: 5px; }");
    connect(accountTable, &QTableWidget::cellClicked, this, &AccountManager::showAccountDetails);
    
    int rowHeight = accountTable->verticalHeader()->defaultSectionSize();
    int headerHeight = accountTable->horizontalHeader()->height();
    int tableHeight = (rowHeight * 10) + headerHeight;
    accountTable->setMinimumHeight(tableHeight);
    
    mainLayout->addWidget(accountTable);

    // Create and set up the account details text edit
    accountDetailsTextEdit = new QTextEdit(this);
    accountDetailsTextEdit->setReadOnly(true);
    accountDetailsTextEdit->setStyleSheet("QTextEdit { border: 1px solid #CCCCCC; border-radius: 4px; padding: 10px; }");
    accountDetailsTextEdit->setMinimumHeight(150);
    mainLayout->addWidget(accountDetailsTextEdit);

    // Set the layout for the main widget
    setLayout(mainLayout);

    // Set minimum size for the widget
    int minWidth = 600;
    int minHeight = tableHeight + buttonContainer->sizeHint().height() + accountDetailsTextEdit->minimumHeight() + 40;
    setMinimumSize(minWidth, minHeight);
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

    QAction *addAccountAction = new QAction("Add Account", this);
    QAction *logoutAction = new QAction("Logout", this);

    connect(addAccountAction, &QAction::triggered, this, &AccountManager::showCreateAccountForm);
    connect(logoutAction, &QAction::triggered, this, &AccountManager::logout);

    menu->addAction(addAccountAction);
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
    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO accounts (id, owner, email, password, balance, is_admin) "
                  "VALUES (:id, :owner, :email, :password, :balance, :is_admin)");
    query.bindValue(":id", QString::fromStdString(account->getID()));
    query.bindValue(":owner", QString::fromStdString(account->getOwner()));
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
        QString email = query.value("email").toString();
        QString password = query.value("password").toString();
        double balance = query.value("balance").toDouble();
        bool isAdmin = query.value("is_admin").toBool();

        Money current = Money::fromDollars(balance);
        Money minimum = Money::fromDollars(0); // Or set an appropriate minimum balance

        Account* account = new Account(owner, id.toStdString(), minimum, current);
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
        Money initial = Money::fromDollars(initialBalance);
        Money minimum = Money::fromDollars(0); // Or set an appropriate minimum balance

        Account* newAccount = new Account(owner, accountId.toStdString(), minimum, initial);
        newAccount->setEmail(email.toStdString());
        
        QString password = QString::fromStdString(bank->generatePassword(owner, accountId.toStdString()));
        newAccount->setPassword(password.toStdString());
        newAccount->setIsAdmin(false);

        allAccounts.append(newAccount);
        saveAccountToDatabase(newAccount);

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

        QString accountDetails = QString("Account Details:\n\nID: %1\nOwner: %2\nEmail: %3\nBalance: $%4\n\n")
                                     .arg(accountId)
                                     .arg(owner)
                                     .arg(email)
                                     .arg(balance, 0, 'f', 2);

        accountDetails += "Recent Transactions:\n\n";

        QSqlQuery transactionQuery;
        transactionQuery.prepare("SELECT * FROM transactions WHERE account_id = :account_id ORDER BY date DESC LIMIT 5");
        transactionQuery.bindValue(":account_id", accountId);

        if (transactionQuery.exec()) {
            while (transactionQuery.next()) {
                QString date = transactionQuery.value("date").toString();
                QString amount = transactionQuery.value("amount").toString();
                QString type = transactionQuery.value("type").toString();

                accountDetails += QString("Date: %1\nAmount: $%2\nType: %3\n\n")
                                      .arg(date)
                                      .arg(amount)
                                      .arg(type);
            }
        }

        accountDetailsTextEdit->setPlainText(accountDetails);
    } else {
        accountDetailsTextEdit->setPlainText("No account details found for Account " + accountId);
    }
}

void AccountManager::setUserAccess(const QString &username, bool isAdmin) {
    currentUser = username;
    isAdminUser = isAdmin;
    userButton->setText(username);
    updateAccountList();
}

QString AccountManager::generateUniqueAccountId() {
    QDateTime now = QDateTime::currentDateTime();
    return now.toString("yyyyMMddHHmmsszzz");  // Format: YearMonthDayHourMinuteSecondMillisecond
}

void AccountManager::clearData() {
    accountTable->clearContents();
    accountTable->setRowCount(0);
    accountDetailsTextEdit->clear();
    qDeleteAll(allAccounts);
    allAccounts.clear();
}