#include "FFVision.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    FFVision window;
    window.show();
    return app.exec();
}
