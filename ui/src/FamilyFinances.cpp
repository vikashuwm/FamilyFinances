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
#include <QDateTime>  // Include QDateTime header
#include <iostream>
#include <QStandardPaths>

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
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    createAccountButton = new QPushButton("Create Account", bankWidget);
    createAccountButton->setObjectName("createAccountButton");
    connect(createAccountButton, &QPushButton::clicked, this, &FamilyFinances::showCreateAccountForm);
    mainLayout->addWidget(createAccountButton, 0, Qt::AlignRight);

    accountTable = new QTableWidget(bankWidget);
    accountTable->setObjectName("accountTable");
    accountTable->setColumnCount(3);
    accountTable->setHorizontalHeaderLabels({"Account ID", "Owner", "Balance"});
    accountTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    accountTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    accountTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    accountTable->setSelectionMode(QAbstractItemView::SingleSelection);
    accountTable->setMinimumHeight(200);
    mainLayout->addWidget(accountTable);

    QHBoxLayout *transferLayout = new QHBoxLayout;
    transferLayout->setSpacing(10);

    sourceInput = new QLineEdit(bankWidget);
    sourceInput->setPlaceholderText("Source Account");

    QLabel *arrowLabel = new QLabel("→", bankWidget);
    arrowLabel->setObjectName("arrowLabel");
    arrowLabel->setAlignment(Qt::AlignCenter);

    destInput = new QLineEdit(bankWidget);
    destInput->setPlaceholderText("Destination Account");

    amountInput = new QLineEdit(bankWidget);
    amountInput->setPlaceholderText("Amount");

    transferButton = new QPushButton("TRANSFER", bankWidget);
    transferButton->setObjectName("transferButton");
    transferButton->setFixedHeight(40);
    connect(transferButton, &QPushButton::clicked, this, &FamilyFinances::performTransaction);

    transferLayout->addWidget(sourceInput);
    transferLayout->addWidget(arrowLabel);
    transferLayout->addWidget(destInput);
    transferLayout->addWidget(amountInput);
    transferLayout->addWidget(transferButton);
    mainLayout->addLayout(transferLayout);

    statusLabel = new QLabel("", bankWidget);
    mainLayout->addWidget(statusLabel);
    updateAccountList();
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

        // Print the current working directory
        qDebug() << "Current working directory:" << QDir::currentPath();

        if (saveAccountToFile(accountId, password, QString::fromStdString(owner), email)) {
            dialog->accept();
            updateAccountList();
        } else {
            QMessageBox::critical(dialog, "Error", "Failed to save account information.");
        }
    });

    connect(cancelButton, &QPushButton::clicked, dialog, &QDialog::reject);

    return dialog;
}

bool FamilyFinances::saveAccountToFile(const QString& accountId, const QString& password, 
                                       const QString& owner, const QString& email) {
    QStandardPaths::StandardLocation location = QStandardPaths::AppDataLocation;
    QString appDataPath = QStandardPaths::writableLocation(location);
    QDir dir(appDataPath);
    
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString filePath = dir.filePath("user_accounts.cfg");
    QFileInfo fileInfo(filePath);
    qDebug() << "Absolute file path:" << fileInfo.absoluteFilePath();

    QFile file(filePath);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        qDebug() << "Failed to open file:" << file.errorString();
        return false;
    }

    QTextStream out(&file);
    out << "Account ID: " << accountId << "\n";
    out << "Password: " << password << "\n";
    out << "Owner: " << owner << "\n";
    out << "Email: " << email << "\n\n";

    out.flush();  // Ensure data is written to the file
    file.close(); // Explicitly close the file

    if (out.status() != QTextStream::Ok) {
        qDebug() << "Failed to write to file:" << file.errorString();
        return false;
    }

    // Verify file content after writing
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString content = in.readAll();
        qDebug() << "File content after writing:";
        qDebug() << content;
        file.close();
    } else {
        qDebug() << "Failed to read file after writing:" << file.errorString();
    }

    qDebug() << "File size after writing:" << QFileInfo(filePath).size() << "bytes";
    qDebug() << "Successfully wrote to file";

    return true;
}

QString FamilyFinances::generateUniqueAccountId() {
    return QString::number(QDateTime::currentMSecsSinceEpoch());
}

void FamilyFinances::updateAccountList() {
    accountTable->setRowCount(0);

    for (const auto& account : bank->getAccounts()) {
        int row = accountTable->rowCount();
        accountTable->insertRow(row);

        accountTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(account->getID())));
        accountTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(account->getOwner())));
        accountTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(account->getCurrent().toString())));
    }
}