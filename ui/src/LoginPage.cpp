#include "LoginPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

LoginPage::LoginPage(QWidget *parent) : QWidget(parent) {
    setupUI();
    connect(loginButton, &QPushButton::clicked, this, &LoginPage::attemptLogin);
}

void LoginPage::setupUI() {
    QVBoxLayout *containerLayout = new QVBoxLayout(this); // Container layout
    QVBoxLayout *mainLayout = new QVBoxLayout(); // Main layout

    QLabel *titleLabel = new QLabel("FamilyFinances Login", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin-bottom: 20px;");
    mainLayout->addWidget(titleLabel);

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

    adminCheckBox = new QCheckBox("Log in as Admin", this);
    mainLayout->addWidget(adminCheckBox, 0, Qt::AlignCenter); // Add checkbox to the layout

    loginButton = new QPushButton("Login", this);
    loginButton->setFixedSize(100, 30); // Set fixed size for login button
    mainLayout->addWidget(loginButton, 0, Qt::AlignCenter);

    mainLayout->setAlignment(Qt::AlignCenter); // Center align the main layout

    containerLayout->addStretch(); // Add stretch to push the main layout to the center
    containerLayout->addLayout(mainLayout); // Add the main layout
    containerLayout->addStretch(); // Add stretch to push the main layout to the center

    setLayout(containerLayout); // Set the container layout as the main layout
}

void LoginPage::attemptLogin() {
    QString username = usernameInput->text();
    QString password = passwordInput->text();
    bool isAdminLogin = adminCheckBox->isChecked(); // Check if admin checkbox is selected

    if (authenticateUser(username, password, isAdminLogin)) {
        emit loginSuccessful(username, isAdminLogin);
    } else {
        QMessageBox::warning(this, "Login Failed", "Invalid username or password.");
    }
}

bool LoginPage::authenticateUser(const QString &username, const QString &password, bool isAdminLogin) {
    QString configFile = isAdminLogin ? "resources/admin.cfg" : "resources/user.cfg";
    QFile file(configFile);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Couldn't open config file.");
        return false;
    }

    QTextStream in(&file);
    QString fileContent = in.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileContent.toUtf8());
    QJsonObject jsonObj = jsonDoc.object();

    return jsonObj.contains(username) && jsonObj[username].toString() == password;
}

bool LoginPage::isAdmin(const QString &username) {
    QFile file("resources/admin.cfg");
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Couldn't open admin config file.");
        return false;
    }

    QTextStream in(&file);
    QString fileContent = in.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileContent.toUtf8());
    QJsonObject jsonObj = jsonDoc.object();

    return jsonObj.contains(username);
}
