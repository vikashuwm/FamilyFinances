// FamilyFinances.cpp
#include "FamilyFinances.h"
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QWidget>
#include <QMouseEvent>

FamilyFinances::FamilyFinances(QWidget *parent)
    : QMainWindow(parent), bank(new Bank()), isAdminUser(false) {
    setWindowTitle("FamilyFinances");

    loginPage = new LoginPage(this);
    bankWidget = new QWidget(this);
    accountManager = new AccountManager(bank, this);
    transactionManager = new TransactionManager(bank, this);

    QStackedWidget *stackedWidget = new QStackedWidget(this);
    stackedWidget->addWidget(loginPage);
    stackedWidget->addWidget(bankWidget);

    setCentralWidget(stackedWidget);

    connect(loginPage, &LoginPage::loginSuccessful, this, &FamilyFinances::onLoginSuccessful);

    setupUI();
}

FamilyFinances::~FamilyFinances() {
    delete bank;
    // Other widgets are deleted automatically
}

void FamilyFinances::closeEvent(QCloseEvent *event) {
    accountManager->saveAllAccountsToFile();
    event->accept();
}

void FamilyFinances::onLoginSuccessful(const QString &username, bool isAdmin) {
    setUserAccess(username, isAdmin);
    static_cast<QStackedWidget*>(centralWidget())->setCurrentWidget(bankWidget);
}

void FamilyFinances::setUserAccess(const QString &username, bool isAdmin) {
    currentUser = username;
    isAdminUser = isAdmin;
    accountManager->setUserAccess(username, isAdmin);
    transactionManager->setUserAccess(username, isAdmin);
}

void FamilyFinances::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(bankWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    mainLayout->addWidget(accountManager);
    mainLayout->addWidget(transactionManager);

    this->setStyleSheet("QMainWindow { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #4BA1D8, stop:1 #4BCAB2); }");
}