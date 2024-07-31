#include "LoginPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFrame>

LoginPage::LoginPage(QWidget *parent) : QWidget(parent) {
    setupUI();
    connect(loginButton, &QPushButton::clicked, this, &LoginPage::attemptLogin);
}

void LoginPage::setupUI() {
    QVBoxLayout *containerLayout = new QVBoxLayout(this); // Container layout
    QVBoxLayout *mainLayout = new QVBoxLayout(); // Main layout

    usernameInput = new QLineEdit(this);
    usernameInput->setPlaceholderText("Username");
    usernameInput->setMinimumSize(250, 40); // Set minimum size
    usernameInput->setStyleSheet("padding: 5px;");

    passwordInput = new QLineEdit(this);
    passwordInput->setPlaceholderText("Password");
    passwordInput->setEchoMode(QLineEdit::Password);
    passwordInput->setMinimumSize(250, 40); // Set minimum size
    passwordInput->setStyleSheet("padding: 5px;");

    mainLayout->addWidget(usernameInput, 0, Qt::AlignCenter);
    mainLayout->addWidget(passwordInput, 0, Qt::AlignCenter);

    loginButton = new QPushButton("Login", this);
    loginButton->setFixedSize(100, 30); // Set fixed size for login button
    mainLayout->addWidget(loginButton, 0, Qt::AlignCenter);

    mainLayout->setAlignment(Qt::AlignCenter); // Center align the main layout

    containerLayout->addStretch(); // Add stretch to push the main layout to the center
    containerLayout->addLayout(mainLayout); // Add the main layout
    containerLayout->addStretch(); // Add stretch to push the main layout to the center

    setLayout(containerLayout); // Set the container layout as the main layout
}

void LoginPage::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    layout()->update();
}

void LoginPage::attemptLogin() {
    QString username = usernameInput->text();
    QString password = passwordInput->text();

    if (authenticateUser(username, password)) {
        bool isAdmin = checkIfAdmin(username);
        emit loginSuccessful(username, isAdmin);
    } else {
        QMessageBox::warning(this, "Login Failed", "Invalid username or password.");
    }
}

bool LoginPage::authenticateUser(const QString &username, const QString &password) {
    QSqlQuery query;
    query.prepare("SELECT password FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (query.exec() && query.next()) {
        QString storedPassword = query.value(0).toString();
        return (storedPassword == password);
    }

    return false;
}

bool LoginPage::checkIfAdmin(const QString &username) {
    QSqlQuery query;
    query.prepare("SELECT is_admin FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (query.exec() && query.next()) {
        return query.value(0).toBool();
    }

    return false;
}