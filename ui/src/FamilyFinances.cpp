#include "FamilyFinances.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QFont>
#include <QCloseEvent>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

FamilyFinances::FamilyFinances(QWidget *parent)
    : QMainWindow(parent), bank(new Bank()), isAdminUser(false) {
    setWindowTitle("Family Finances");

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

void FamilyFinances::closeEvent(QCloseEvent *event) {
    // You can add any cleanup code here if needed
    event->accept();
}

void FamilyFinances::onLoginSuccessful(const QString &username, bool isAdmin) {
    setUserAccess(username, isAdmin);
    static_cast<QStackedWidget*>(centralWidget())->setCurrentWidget(bankWidget);
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

void FamilyFinances::setUserAccess(const QString &username, bool isAdmin) {
    currentUser = username;
    isAdminUser = isAdmin;
    accountManager->setUserAccess(username, isAdmin);
    transactionManager->setUserAccess(username, isAdmin);
}

void FamilyFinances::setupUI() {
    // Set the main window background
    this->setStyleSheet("QMainWindow { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #4BA1D8, stop:1 #4BCAB2); }");

    QVBoxLayout *mainLayout = new QVBoxLayout(bankWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    // Add a header with user button
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *header = new QLabel("Family Finances", bankWidget);
    header->setStyleSheet("font-size: 24px; font-weight: bold; color: #2C3E50; margin-bottom: 20px;");
    headerLayout->addWidget(header);
    headerLayout->addStretch();
    headerLayout->addWidget(accountManager->getUserButton());
    mainLayout->addLayout(headerLayout);

    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(30);

    // Left side: Account Manager
    QFrame *accountFrame = createStyledFrame();
    QVBoxLayout *accountLayout = new QVBoxLayout(accountFrame);
    accountLayout->addWidget(accountManager);
    contentLayout->addWidget(accountFrame, 2);

    // Right side: Transaction Manager
    QFrame *transactionFrame = createStyledFrame();
    QVBoxLayout *transactionLayout = new QVBoxLayout(transactionFrame);
    transactionLayout->addWidget(transactionManager);
    contentLayout->addWidget(transactionFrame, 1);

    mainLayout->addLayout(contentLayout);

    bankWidget->setLayout(mainLayout);
}

QFrame* FamilyFinances::createStyledFrame() {
    QFrame *frame = new QFrame(bankWidget);
    frame->setObjectName("styledFrame");
    frame->setStyleSheet(
        "QFrame#styledFrame {"
        "    background-color: white;"
        "    border-radius: 8px;"
        "    border: 1px solid #E0E0E0;"
        "}"
        "QLabel { font-size: 18px; font-weight: bold; color: #34495E; margin-bottom: 10px; }"
    );
    frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    return frame;
}