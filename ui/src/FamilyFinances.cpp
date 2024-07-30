#include "FamilyFinances.h"
#include "Bank.h"
#include "LoginPage.h"
#include "Transaction.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QStackedWidget>
#include <QDialog>
#include <QCheckBox>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>
#include <QGroupBox>    // Add this include
#include <QCloseEvent>  // Add this include
#include <QDateTime>
#include <QStandardPaths>
#include "Account.h"
#include "Transaction.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>

FamilyFinances::FamilyFinances(QWidget *parent)
    : QMainWindow(parent), bank(new Bank()), isAdminUser(false) {
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
    // All child widgets and layouts are automatically deleted
}

void FamilyFinances::onLoginSuccessful(const QString &username, bool isAdmin) {
    setUserAccess(username, isAdmin);
    static_cast<QStackedWidget*>(centralWidget())->setCurrentWidget(bankWidget);
}

void FamilyFinances::setUserAccess(const QString &username, bool isAdmin) {
    currentUser = username;
    isAdminUser = isAdmin;

    createAccountButton->setVisible(isAdminUser);
    updateAccountList();

    if (!isAdminUser) {
        sourceInput->setText(username);
        sourceInput->setReadOnly(true);
    } else {
        sourceInput->setReadOnly(false);
    }
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

void FamilyFinances::showCreateAccountForm() {
    QDialog* dialog = setupAccountCreationDialog();
    dialog->exec();
    delete dialog;
}

void FamilyFinances::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(bankWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Create Account button
    createAccountButton = new QPushButton("Create Account", bankWidget);
    createAccountButton->setObjectName("createAccountButton");
    createAccountButton->setStyleSheet("background-color: #007BFF; color: white; border-radius: 15px; padding: 10px 20px;");
    connect(createAccountButton, &QPushButton::clicked, this, &FamilyFinances::showCreateAccountForm);
    mainLayout->addWidget(createAccountButton, 0, Qt::AlignRight);

    // Account table
    accountTable = new QTableWidget(bankWidget);
    accountTable->setObjectName("accountTable");
    accountTable->setColumnCount(3);
    accountTable->setHorizontalHeaderLabels({"Account ID", "Owner", "Balance"});
    accountTable->horizontalHeader()->setStretchLastSection(true);
    accountTable->setStyleSheet("QTableWidget { border: 1px solid #CCCCCC; border-radius: 10px; }"
                                "QHeaderView::section { background-color: #333333; color: white; padding: 5px; }");
    mainLayout->addWidget(accountTable);

    // Transaction group
    QGroupBox* transactionGroup = new QGroupBox(bankWidget);
    transactionGroup->setStyleSheet("QGroupBox { border: 1px solid #CCCCCC; border-radius: 10px; padding: 10px; background-color: rgba(255, 255, 255, 0.1); }");
    QVBoxLayout* transactionLayout = new QVBoxLayout(transactionGroup);

    // Transaction inputs
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

    // Transfer button
    QPushButton* transferButton = new QPushButton("TRANSFER", transactionGroup);
    transferButton->setStyleSheet("background-color: #28A745; color: white; border-radius: 5px; padding: 10px;");
    connect(transferButton, &QPushButton::clicked, this, &FamilyFinances::performTransaction);
    transactionLayout->addWidget(transferButton);

    mainLayout->addWidget(transactionGroup);

    // Status label
    statusLabel = new QLabel(bankWidget);
    statusLabel->setObjectName("statusLabel");
    statusLabel->setStyleSheet("color: white;");
    mainLayout->addWidget(statusLabel);

    // Set background gradient for the main window
    this->setStyleSheet("QMainWindow { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #4BA1D8, stop:1 #4BCAB2); }");
}

void FamilyFinances::updateAccountList() {
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

QDialog* FamilyFinances::setupAccountCreationDialog() {
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

void FamilyFinances::saveAllAccountsToFile() {
    // Get the path to the executable
    QString executablePath = QCoreApplication::applicationDirPath();
    
    // Navigate to the project's root directory (assuming the executable is in a subdirectory)
    QDir projectDir(executablePath);
    projectDir.cdUp(); // Move up one directory
    
    // Create the path to the resources directory
    QString resourcesPath = projectDir.absoluteFilePath("resources");
    QDir resourcesDir(resourcesPath);
    
    // Ensure the resources directory exists
    if (!resourcesDir.exists()) {
        if (!resourcesDir.mkpath(".")) {
            qDebug() << "Failed to create resources directory:" << resourcesPath;
            return;
        }
    }
    
    // Set the file path
    QString filePath = resourcesDir.filePath("user_accounts.cfg");
    QFileInfo fileInfo(filePath);
    qDebug() << "Absolute file path:" << fileInfo.absoluteFilePath();

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

    out.flush();
    file.close();

    if (out.status() != QTextStream::Ok) {
        qDebug() << "Failed to write to file:" << file.errorString();
        return;
    }

    qDebug() << "File size after writing:" << QFileInfo(filePath).size() << "bytes";
    qDebug() << "Successfully wrote all accounts and their last 5 transactions to file";
}

void FamilyFinances::closeEvent(QCloseEvent *event) {
    saveAllAccountsToFile();
    event->accept();
}

QString FamilyFinances::generateUniqueAccountId() {
    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString("yyyyMMddHHmmsszzz");  // Format the date and time
    return timestamp;
}

