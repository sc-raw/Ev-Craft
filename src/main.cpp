#include "MainWindow.h"

#include <QApplication>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("EV"));
    QApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    QApplication::setOrganizationName(QStringLiteral("EV"));

    QFont font(QStringLiteral("Segoe UI"), 9);
    app.setFont(font);

    MainWindow window;
    window.show();
    return app.exec();
}
