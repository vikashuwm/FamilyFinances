#ifndef LOGINPAGE_H
#define LOGINPAGE_H
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

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
    bool authenticateUser(const QString &username, const QString &password);
    bool checkIfAdmin(const QString &username);

    QLineEdit *usernameInput;
    QLineEdit *passwordInput;
    QPushButton *loginButton;

protected:
    void resizeEvent(QResizeEvent *event) override;
};
#endif // LOGINPAGE_H