#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>

class LoginPage : public QWidget {
    Q_OBJECT

public:
    explicit LoginPage(QWidget *parent = nullptr);

signals:
    void loginSuccessful(const QString &username, bool isAdmin);

private slots:
    void attemptLogin();

private:
    void setupUI();
    bool authenticateUser(const QString &username, const QString &password, bool isAdminLogin);
    bool isAdmin(const QString &username);

    QLineEdit *usernameInput;
    QLineEdit *passwordInput;
    QPushButton *loginButton;
    QCheckBox *adminCheckBox; // New checkbox for admin login
};

#endif // LOGINPAGE_H
