#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>
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

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);
    
    QApplication::setApplicationName("Family Finances");
    QApplication::setApplicationVersion("1.0");
    QApplication::setOrganizationName("Your Organization");
    QApplication::setOrganizationDomain("yourorganization.com");

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