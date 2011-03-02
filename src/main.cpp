#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QtGui/QApplication>
#include <QTextCodec>
#include <QtPlugin>
#include <QDebug>
#include "mainapplication.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(icons);
    Q_INIT_RESOURCE(html);

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#ifdef Q_WS_X11
    QApplication::setGraphicsSystem("raster"); // Better overall performance on X11
#endif

    MainApplication app(argc, argv);
    if (app.isExited())
        return 1;
    return app.exec();
}
