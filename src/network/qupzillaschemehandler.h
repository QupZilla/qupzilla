#ifndef QUPZILLASCHEMEHANDLER_H
#define QUPZILLASCHEMEHANDLER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QBuffer>
#include <QTimer>
#include <QTextStream>

class QupZillaSchemeHandler : public QObject
{
    Q_OBJECT
public:
    explicit QupZillaSchemeHandler(QObject* parent = 0);

    QNetworkReply* createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice* outgoingData);

signals:

public slots:

};

class QupZillaSchemeReply : public QNetworkReply
{
    Q_OBJECT
public:
    explicit QupZillaSchemeReply(const QNetworkRequest &req, QObject* parent = 0);

    virtual qint64 bytesAvailable() const;

protected:
    virtual qint64 readData(char *data, qint64 maxSize);
    virtual void abort() { }

private slots:
    void delayedFinish();
    void loadPage();

private:
    QString aboutPage();
    QString reportbugPage();

    QBuffer m_buffer;
    QString m_pageName;

};

#endif // QUPZILLASCHEMEHANDLER_H
