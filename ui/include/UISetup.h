#pragma once

#include <QObject>

class QWidget;
class QLineEdit;
class QListWidget;
class QLabel;
class QPushButton;

class UISetup : public QObject {
    Q_OBJECT

public:
    explicit UISetup(QWidget *parent);
    ~UISetup() = default;
    
    void setupUI(QLineEdit *&ownerInput, QLineEdit *&initialBalanceInput, QListWidget *&accountList,
                 QLineEdit *&sourceInput, QLineEdit *&destInput, QLineEdit *&amountInput, QLabel *&statusLabel);
    
    QPushButton* getCreateButton() const;
    QPushButton* getTransferButton() const;

private:
    QWidget *parent;
    QPushButton *createButton;
    QPushButton *transferButton;
};