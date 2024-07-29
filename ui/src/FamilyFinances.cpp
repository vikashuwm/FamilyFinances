#include "FamilyFinances.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QStackedWidget>
#include "Bank.h"
#include "LoginPage.h"
#include "Transaction.h"

FamilyFinances::FamilyFinances(QWidget *parent)
    : QMainWindow(parent), bank(new Bank()), isAdminUser(false), nextAccountId(1) {
    setWindowTitle("FamilyFinances");

    loginPage = new LoginPage(this);
    bankWidget = new QWidget(this);

    QStackedWidget *stackedWidget = new QStackedWidget(this);
    stackedWidget->addWidget(loginPage);
    stackedWidget->addWidget(bankWidget);

    setCentralWidget(stackedWidget);

    connect(loginPage, &LoginPage::loginSuccessful, this, &FamilyFinances::onLoginSuccessful);

    setupUI();
}

FamilyFinances::~FamilyFinances() {
    delete bank;
}

void FamilyFinances::onLoginSuccessful(const QString &username, bool isAdmin) {
    setUserAccess(username, isAdmin);
    static_cast<QStackedWidget*>(centralWidget())->setCurrentWidget(bankWidget);
}

void FamilyFinances::setUserAccess(const QString &username, bool isAdmin) {
    currentUser = username;
    isAdminUser = isAdmin;

    accountCreationWidget->setVisible(isAdminUser);
    updateAccountList();

    if (!isAdminUser) {
        sourceInput->setText(username);
        sourceInput->setReadOnly(true);
    } else {
        sourceInput->setReadOnly(false);
    }
}

void FamilyFinances::createAccount() {
    if (!isAdminUser) {
        statusLabel->setText("Error: Only administrators can create accounts.");
        return;
    }

    QString firstName = firstNameInput->text().trimmed();
    QString lastName = lastNameInput->text().trimmed();
    QString email = emailInput->text().trimmed();
    QString ageStr = ageInput->text().trimmed();
    QString owner = ownerInput->text().trimmed();
    QString initialBalanceStr = initialBalanceInput->text().trimmed();
    QString sex = sexInput->currentText();

    if (owner.isEmpty() || initialBalanceStr.isEmpty() || email.isEmpty() || ageStr.isEmpty()) {
        statusLabel->setText("Error: Please fill in all required fields.");
        return;
    }

    bool conversionOk;
    double initialBalance = initialBalanceStr.toDouble(&conversionOk);
    int age = ageStr.toInt(&conversionOk);

    if (!conversionOk || initialBalance < 0 || age < 0) {
        statusLabel->setText("Error: Invalid input. Please check your entries.");
        return;
    }

    try {
        QString accountId = QString("ACC%1").arg(nextAccountId, 4, 10, QChar('0'));
        nextAccountId++;
        
        std::shared_ptr<Account> newAccount = bank->open(
            owner.toStdString(),
            accountId.toStdString(),
            Money::fromCents(0),  // Minimum balance
            Money::fromDollars(initialBalance)
        );

        updateAccountList();
        statusLabel->setText(QString("Account created successfully. ID: %1").arg(accountId));
    } catch (const std::exception& e) {
        statusLabel->setText(QString("Error: %1").arg(e.what()));
    }
    
    // Clear inputs
    firstNameInput->clear();
    lastNameInput->clear();
    emailInput->clear();
    ageInput->clear();
    ownerInput->clear();
    initialBalanceInput->clear();
}


void FamilyFinances::performTransaction() {
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
        
        updateAccountList();
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

void FamilyFinances::updateAccountList() {
    accountTable->setRowCount(0);
    Bank::Iterator it = bank->iterator();
    while (it.hasNext()) {
        std::shared_ptr<Account> account = it.next();
        if (isAdminUser || account->getOwner() == currentUser.toStdString()) {
            int row = accountTable->rowCount();
            accountTable->insertRow(row);
            accountTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(account->getID())));
            accountTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(account->getOwner())));
            accountTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(account->getCurrent().toString())));
        }
    }
}

void FamilyFinances::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(bankWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20); // Add margins to the main layout

    setupAccountCreation(mainLayout); // Call the new function for account creation

    // Account table
    accountTable = new QTableWidget(bankWidget);
    accountTable->setColumnCount(3);
    accountTable->setHorizontalHeaderLabels({"Account ID", "Owner", "Balance"});
    accountTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    accountTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    accountTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    accountTable->setSelectionMode(QAbstractItemView::SingleSelection);
    accountTable->setFixedHeight(200);
    accountTable->setStyleSheet("border: 1px solid #4caf50; border-radius: 5px; padding: 5px;"); // Add border and padding
    mainLayout->addWidget(accountTable);
    
    // Transaction
    QHBoxLayout *transactionLayout = new QHBoxLayout();
    transactionLayout->setSpacing(10);

    sourceInput = new QLineEdit(bankWidget);
    sourceInput->setPlaceholderText("Source Account ID");
    sourceInput->setStyleSheet("padding: 5px;");
    transactionLayout->addWidget(sourceInput);
    
    QLabel *arrowLabel = new QLabel("→", bankWidget);
    arrowLabel->setStyleSheet("color: #4caf50; font-size: 20px;");
    arrowLabel->setAlignment(Qt::AlignCenter);
    transactionLayout->addWidget(arrowLabel);

    destInput = new QLineEdit(bankWidget);
    destInput->setPlaceholderText("Destination Account ID");
    destInput->setStyleSheet("padding: 5px;");
    transactionLayout->addWidget(destInput);
    
    amountInput = new QLineEdit(bankWidget);
    amountInput->setPlaceholderText("Amount");
    amountInput->setStyleSheet("padding: 5px;");
    transactionLayout->addWidget(amountInput);
    
    mainLayout->addLayout(transactionLayout);

    // Transfer button
    transferButton = new QPushButton("TRANSFER", bankWidget);
    transferButton->setFixedHeight(40);
    transferButton->setStyleSheet(R"(
        padding: 5px;
        font-size: 10px; /* Adjust font size */
    )");
    mainLayout->addWidget(transferButton);

    // What is a bank transfer? label
    QLabel *infoLabel = new QLabel("<a href='#'>What is a bank transfer?</a>", bankWidget);
    infoLabel->setTextFormat(Qt::RichText);
    infoLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    infoLabel->setOpenExternalLinks(false);
    infoLabel->setStyleSheet("color: #4caf50; text-decoration: underline; padding: 5px;"); // Add padding
    mainLayout->addWidget(infoLabel);

    // Status label
    statusLabel = new QLabel("", bankWidget);
    statusLabel->setStyleSheet("padding: 5px;");
    mainLayout->addWidget(statusLabel);

    bankWidget->setLayout(mainLayout);

    connect(createButton, &QPushButton::clicked, this, &FamilyFinances::createAccount);
    connect(transferButton, &QPushButton::clicked, this, &FamilyFinances::performTransaction);
    connect(infoLabel, &QLabel::linkActivated, this, &FamilyFinances::showTransferInfo);

    // Initially hide the account creation section
    accountCreationWidget->setVisible(false);

    // Apply stylesheet
    setStyleSheet(R"(
        QWidget {
            background-color: #1e2124;
            color: #ffffff;
            font-family: Arial, sans-serif;
        }
        QLineEdit {
            background-color: #2c2f33;
            border: 1px solid #4caf50;
            border-radius: 4px;
            padding: 10px;
            font-size: 14px;
            color: #99aab5;
        }
        QPushButton {
            background-color: #4caf50;
            color: #ffffff;
            border: 1px solid #4caf50;
            border-radius: 4px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
        QTableWidget {
            background-color: #2c2f33;
            border: 1px solid #4caf50;
            border-radius: 4px;
        }
        QHeaderView::section {
            background-color: #2c2f33;
            color: #ffffff;
            border: none;
            padding: 5px;
        }
        QTableWidget::item {
            border: none;
        }
        QLabel {
            color: #4caf50;
            font-size: 14px;
        }
        QLabel[textInteractionFlags="Qt::TextBrowserInteraction"] {
            color: #4caf50;
            text-decoration: underline;
        }
    )");
}


void FamilyFinances::setupAccountCreation(QVBoxLayout *mainLayout) {
    accountCreationWidget = new QWidget(bankWidget);
    QVBoxLayout *createLayout = new QVBoxLayout(accountCreationWidget);
    createLayout->setSpacing(10);
    createLayout->setContentsMargins(10, 10, 10, 10); // Adjust margins

    // First Name
    firstNameInput = new QLineEdit(accountCreationWidget);
    firstNameInput->setPlaceholderText("First Name");
    firstNameInput->setStyleSheet("padding: 5px;");
    createLayout->addWidget(firstNameInput);

    // Last Name
    lastNameInput = new QLineEdit(accountCreationWidget);
    lastNameInput->setPlaceholderText("Last Name");
    lastNameInput->setStyleSheet("padding: 5px;");
    createLayout->addWidget(lastNameInput);
    
    // Email
    emailInput = new QLineEdit(accountCreationWidget);
    emailInput->setPlaceholderText("Email");
    emailInput->setStyleSheet("padding: 5px;");
    createLayout->addWidget(emailInput);
    
    // Age
    ageInput = new QLineEdit(accountCreationWidget);
    ageInput->setPlaceholderText("Age");
    ageInput->setStyleSheet("padding: 5px;");
    ageInput->setValidator(new QIntValidator(0, 150, this)); // Age range
    createLayout->addWidget(ageInput);
    
    // Sex
    sexInput = new QComboBox(accountCreationWidget);
    sexInput->addItems({"Male", "Female", "Other"});
    sexInput->setStyleSheet("padding: 5px;");
    createLayout->addWidget(sexInput);

    // Owner Name
    ownerInput = new QLineEdit(accountCreationWidget);
    ownerInput->setPlaceholderText("Owner Name");
    ownerInput->setStyleSheet("padding: 5px;");
    createLayout->addWidget(ownerInput);
    
    // Initial Balance
    initialBalanceInput = new QLineEdit(accountCreationWidget);
    initialBalanceInput->setPlaceholderText("Initial Balance");
    initialBalanceInput->setStyleSheet("padding: 5px;");
    createLayout->addWidget(initialBalanceInput);

    // Create Button
    createButton = new QPushButton("CREATE ACCOUNT", accountCreationWidget);
    createButton->setFixedSize(120, 30);
    createButton->setStyleSheet(R"(
        padding: 5px;
        font-size: 10px; /* Adjust font size */
    )");
    createLayout->addWidget(createButton);

    accountCreationWidget->setStyleSheet("border: 1px solid #4caf50; border-radius: 5px; padding: 5px;"); // Add border and padding
    
    // Set fixed height and expandable width
    accountCreationWidget->setFixedHeight(300); // Adjust the height as needed
    accountCreationWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    mainLayout->addWidget(accountCreationWidget);
}
