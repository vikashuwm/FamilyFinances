#include "UISetup.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>

UISetup::UISetup(QWidget *parent) : QObject(parent), parent(parent), createButton(nullptr), transferButton(nullptr) {}

void UISetup::setupUI(QLineEdit *&ownerInput, QLineEdit *&initialBalanceInput, QListWidget *&accountList,
                      QLineEdit *&sourceInput, QLineEdit *&destInput, QLineEdit *&amountInput, QLabel *&statusLabel) {
    QVBoxLayout *mainLayout = new QVBoxLayout(parent);
    
    // Account creation
    QHBoxLayout *createLayout = new QHBoxLayout();
    ownerInput = new QLineEdit(parent);
    ownerInput->setPlaceholderText("Owner Name");
    createLayout->addWidget(ownerInput);
    
    initialBalanceInput = new QLineEdit(parent);
    initialBalanceInput->setPlaceholderText("Initial Balance");
    createLayout->addWidget(initialBalanceInput);
    
    createButton = new QPushButton("Create Account", parent);
    createLayout->addWidget(createButton);
    
    mainLayout->addLayout(createLayout);
    
    // Account list
    accountList = new QListWidget(parent);
    mainLayout->addWidget(accountList);
    
    // Transaction
    QHBoxLayout *transactionLayout = new QHBoxLayout();
    sourceInput = new QLineEdit(parent);
    sourceInput->setPlaceholderText("Source Account ID");
    transactionLayout->addWidget(sourceInput);
    
    destInput = new QLineEdit(parent);
    destInput->setPlaceholderText("Destination Account ID");
    transactionLayout->addWidget(destInput);
    
    amountInput = new QLineEdit(parent);
    amountInput->setPlaceholderText("Amount");
    transactionLayout->addWidget(amountInput);
    
    transferButton = new QPushButton("Transfer", parent);
    transactionLayout->addWidget(transferButton);
    
    mainLayout->addLayout(transactionLayout);
    
    // Status label
    statusLabel = new QLabel("", parent);
    mainLayout->addWidget(statusLabel);

    // The parent widget should already be a QWidget, so we don't need to cast
    parent->setLayout(mainLayout);
}

QPushButton* UISetup::getCreateButton() const {
    return createButton;
}

QPushButton* UISetup::getTransferButton() const {
    return transferButton;
}