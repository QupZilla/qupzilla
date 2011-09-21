#ifndef GLOBALFUNCTIONS_H
#define GLOBALFUNCTIONS_H

#include <QByteArray>
#include <QPixmap>
#include <QBuffer>
#include <QFile>
#include <QWidget>
#include <QApplication>
#include <QDesktopWidget>

QByteArray qz_pixmapToByteArray(const QPixmap &pix);
QByteArray qz_readAllFileContents(const QString &filename);

void qz_centerWidgetOnScreen(QWidget* w);

#endif // GLOBALFUNCTIONS_H
