#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "FamilyFinances.h"

bool loadStyleSheet(QApplication &app, const QString &sheetName)
{
    QFile file(sheetName);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream textStream(&file);
        QString styleSheet = textStream.readAll();
        app.setStyleSheet(styleSheet);
        file.close();
        qDebug() << "Successfully loaded stylesheet:" << sheetName;
        return true;
    } else {
        qWarning() << "Failed to open stylesheet file:" << sheetName;
        qWarning() << "Error:" << file.errorString();
        return false;
    }
}

bool initializeDatabase() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/Users/vikashkumar/FamilyFinances/familyfinances.db");

    if (!db.open()) {
        qDebug() << "Error: connection with database failed - " << db.lastError().text();
        return false;
    }

    qDebug() << "Database opened successfully";

    QSqlQuery query;
    
    // Create accounts table
    if (!query.exec("CREATE TABLE IF NOT EXISTS accounts ("
                    "id TEXT PRIMARY KEY, "
                    "username TEXT NOT NULL UNIQUE, "
                    "owner TEXT, "
                    "email TEXT UNIQUE, "
                    "password TEXT NOT NULL, "
                    "balance REAL, "
                    "is_admin INTEGER NOT NULL)")) {
        qDebug() << "Error creating accounts table:" << query.lastError().text();
        return false;
    }

    qDebug() << "Accounts table created or already exists";

    // Create transactions table
    if (!query.exec("CREATE TABLE IF NOT EXISTS transactions ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "account_id TEXT, "
                    "amount REAL, "
                    "type TEXT, "
                    "date TEXT, "
                    "FOREIGN KEY (account_id) REFERENCES accounts(id))")) {
        qDebug() << "Error creating transactions table:" << query.lastError().text();
        return false;
    }

    qDebug() << "Transactions table created or already exists";

    // Check for admin user
    if (!query.exec("SELECT COUNT(*) FROM accounts WHERE username = 'admin' AND is_admin = 1")) {
        qDebug() << "Error executing admin user check query:" << query.lastError().text();
        // Instead of returning false, we'll proceed to create the admin user
    } else {
        if (!query.next()) {
            qDebug() << "Error fetching result from admin user check query:" << query.lastError().text();
            // Instead of returning false, we'll proceed to create the admin user
        } else {
            int count = query.value(0).toInt();
            qDebug() << "Number of admin users found:" << count;
            if (count > 0) {
                qDebug() << "Admin user already exists";
                return true;  // Admin exists, no need to create one
            }
        }
    }

    // If we've reached this point, we need to create an admin user
    query.prepare("INSERT INTO accounts (id, username, owner, email, password, balance, is_admin) "
                  "VALUES (:id, :username, :owner, :email, :password, :balance, :is_admin)");
    query.bindValue(":id", "admin");
    query.bindValue(":username", "admin");
    query.bindValue(":owner", "Administrator");
    query.bindValue(":email", "admin@example.com");
    query.bindValue(":password", "admin"); // In a real app, use a hashed password
    query.bindValue(":balance", 0.0);
    query.bindValue(":is_admin", 1);

    if (!query.exec()) {
        qDebug() << "Error creating admin user:" << query.lastError().text();
        return false;
    }

    qDebug() << "Admin user created successfully";
    return true;
}


int main(int argc, char *argv[]) {
    qDebug() << "Inse main:" << "\n";
    QApplication app(argc, argv);
    
    QApplication::setApplicationName("Family Finances");
    QApplication::setApplicationVersion("1.0");
    QApplication::setOrganizationName("Your Organization");
    QApplication::setOrganizationDomain("yourorganization.com");

    if (!initializeDatabase()) {
        QMessageBox::critical(nullptr, "Database Error", "Failed to initialize the database. The application will now exit.");
        return 1;
    }

    if (!loadStyleSheet(app, ":/FamilyFinances.qss")) {
        qWarning() << "Failed to load style sheet from resources. Trying absolute path...";
        if (!loadStyleSheet(app, QCoreApplication::applicationDirPath() + "/FamilyFinances.qss")) {
            QMessageBox::warning(nullptr, "Style Sheet Error", "Failed to load style sheet. The application may not look as intended.");
        }
    }

    FamilyFinances familyFinances;
    familyFinances.show();
    
    return app.exec();
}