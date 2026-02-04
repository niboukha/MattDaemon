#include <QApplication>
#include "Gui/Gui.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Gui w;
    w.show();
    return app.exec();
}
