
#include "FamilyFinances.h"
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QWidget>
#include <QMouseEvent>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "FamilyFinances.h"
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QWidget>
#include <QMouseEvent>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

FamilyFinances::FamilyFinances(QWidget *parent)
    : QMainWindow(parent), bank(new Bank()), isAdminUser(false) {
    qDebug() << "Inside FamilyFinances constructor:" << "\n";
    setWindowTitle("FamilyFinances");
    // Initialize database connection
    if (!initializeDatabase()) {
        qCritical() << "Failed to initialize database. Exiting.";
        exit(1);
    }

    loginPage = new LoginPage(this);
    bankWidget = new QWidget(this);
    accountManager = new AccountManager(bank, this);
    transactionManager = new TransactionManager(bank, this);

    QStackedWidget *stackedWidget = new QStackedWidget(this);
    stackedWidget->addWidget(loginPage);
    stackedWidget->addWidget(bankWidget);

    setCentralWidget(stackedWidget);

    connect(loginPage, &LoginPage::loginSuccessful, this, &FamilyFinances::onLoginSuccessful);
    connect(accountManager, &AccountManager::logoutRequested, this, &FamilyFinances::onLogoutRequested);
    connect(transactionManager, &TransactionManager::transactionCompleted, accountManager, &AccountManager::onTransactionCompleted);
    setupUI();
}

FamilyFinances::~FamilyFinances() {
    delete bank;
    QSqlDatabase::database().close();
}

bool FamilyFinances::initializeDatabase() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/Users/vikashkumar/FamilyFinances/familyfinances.db");

    if (!db.open()) {
        qDebug() << "Error: connection with database failed - " << db.lastError().text();
        return false;
    }

    qDebug() << "Database opened successfully";
    return true;
}

void FamilyFinances::onLogoutRequested() {
    qDebug() << "Logout requested";
    static_cast<QStackedWidget*>(centralWidget())->setCurrentWidget(loginPage);
    currentUser.clear();
    isAdminUser = false;
    loginPage->clearInputs();
    accountManager->clearData();
    transactionManager->clearData();
}

void FamilyFinances::closeEvent(QCloseEvent *event) {
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