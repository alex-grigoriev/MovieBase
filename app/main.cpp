#include "main_window.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName(QStringLiteral("MovieApp"));
    app.setOrganizationName(QStringLiteral("MovieApp"));

    ui::MainWindow window;
    window.show();

    return app.exec();
}
