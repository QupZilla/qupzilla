#include "globalfunctions.h"

QByteArray qz_pixmapToByteArray(const QPixmap &pix)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    if (pix.save(&buffer, "PNG"))
        return buffer.buffer().toBase64();

    return QByteArray();
}

QByteArray qz_readAllFileContents(const QString &filename)
{
    QFile file(filename);
    file.open(QFile::ReadOnly);
    QByteArray a = file.readAll();
    file.close();

    return a;
}

void qz_centerWidgetOnScreen(QWidget *w)
{
    const QRect screen = QApplication::desktop()->screenGeometry();
    const QRect &size = w->geometry();
    w->move( (screen.width()-size.width())/2, (screen.height()-size.height())/2 );
}
