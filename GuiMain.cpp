#include <QApplication>
#include <QtGlobal>
#include "Gui/Gui.h"

int main(int argc, char *argv[]) {
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", QByteArray("xcb"));
    }
    QApplication app(argc, argv);
    
    Gui w;
    w.show();
    return app.exec();
}
