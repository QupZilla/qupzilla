#ifndef GLOBALFUNCTIONS_H
#define GLOBALFUNCTIONS_H

#include <QByteArray>
#include <QPixmap>
#include <QBuffer>
#include <QFile>

QByteArray qz_pixmapToByteArray(const QPixmap &pix);
QByteArray qz_readAllFileContents(const QString &filename);

#endif // GLOBALFUNCTIONS_H
