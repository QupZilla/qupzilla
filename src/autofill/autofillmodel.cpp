#include "autofillmodel.h"
#include "qupzilla.h"
#include "webview.h"
#include "mainapplication.h"
#include "autofillwidget.h"

AutoFillModel::AutoFillModel(QupZilla* mainClass, QObject *parent) :
    QObject(parent)
    ,p_QupZilla(mainClass)
    ,m_isStoring(false)
{
    QTimer::singleShot(0, this, SLOT(loadSettings()));
}

void AutoFillModel::loadSettings()
{
    QSettings settings(MainApplication::getInstance()->getActiveProfil()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("Web-Browser-Settings");
    m_isStoring = settings.value("AutoFillForms",true).toBool();
    settings.endGroup();
}

bool AutoFillModel::isStored(const QUrl &url)
{
    QString server = url.host();
    QSqlQuery query;
    query.exec("SELECT count(id) FROM autofill WHERE server='"+server+"'");
    query.next();
    if (query.value(0).toInt()>0)
        return true;
    return false;
}

bool AutoFillModel::isStoringEnabled(const QUrl &url)
{
    if (!m_isStoring)
        return false;
    QString server = url.host();
    QSqlQuery query;
    query.exec("SELECT count(id) FROM autofill_exceptions WHERE server='"+server+"'");
    query.next();
    if (query.value(0).toInt()>0)
        return false;
    return true;
}

void AutoFillModel::blockStoringfor (const QUrl &url)
{
    QString server = url.host();
    QSqlQuery query;
    query.exec("INSERT INTO autofill_exceptions (server) VALUES ('"+server+"')");
}

QString AutoFillModel::getUsername(const QUrl &url)
{
    QString server = url.host();
    QSqlQuery query;
    query.exec("SELECT username FROM autofill WHERE server='"+server+"'");
    query.next();
    return query.value(0).toString();
}

QString AutoFillModel::getPassword(const QUrl &url)
{
    QString server = url.host();
    QSqlQuery query;
    query.exec("SELECT password FROM autofill WHERE server='"+server+"'");
    query.next();
    return query.value(0).toString();
}

///HTTP Authorization
bool AutoFillModel::addEntry(const QUrl &url, const QString &name, const QString &pass)
{
    QSqlQuery query;
    query.exec("SELECT username FROM autofill WHERE server='"+url.host()+"'");
    if (query.next())
        return false;
    query.prepare("INSERT INTO autofill (server, username, password) VALUES (?,?,?)");
    query.bindValue(0, url.host());
    query.bindValue(1, name);
    query.bindValue(2, pass);
    return query.exec();
}

///WEB Form
bool AutoFillModel::addEntry(const QUrl &url, const QByteArray &data, const QString &pass)
{
    QSqlQuery query;
    query.exec("SELECT data FROM autofill WHERE server='"+url.host()+"'");
    if (query.next())
        return false;

    query.prepare("INSERT INTO autofill (server, data, password) VALUES (?,?,?)");
    query.bindValue(0, url.host());
    query.bindValue(1, data);
    query.bindValue(2, pass);
    return query.exec();
}

void AutoFillModel::completePage(WebView* view)
{
    if (!isStored(view->url()))
        return;

    QWebFrame* frame = view->page()->mainFrame();
    QWebElementCollection inputs = frame->findAllElements("input");
    QSqlQuery query;
    query.exec("SELECT data FROM autofill WHERE server='"+view->url().host()+"'");
    query.next();
    QByteArray data = query.value(0).toByteArray();
    if (data.isEmpty())
        return;

    QList<QPair<QString, QString> > arguments = QUrl::fromEncoded(QByteArray("http://bla.com/?"+data)).queryItems();
    for (int i = 0; i<arguments.count(); i++) {
        QString key = arguments.at(i).first;
        QString value = arguments.at(i).second;
        key.replace("+"," ");
        value.replace("+"," ");

        for (int i = 0; i<inputs.count(); i++) {
            QWebElement element = inputs.at(i);

            if (element.attribute("type")!="text" && element.attribute("type")!="password" && element.attribute("type")!="")
                continue;
            if (key == element.attribute("name"))
                element.setAttribute("value",value);
        }
    }
}

void AutoFillModel::post(const QNetworkRequest &request, const QByteArray &outgoingData)
{
    //Dont save in private browsing
    if (MainApplication::getInstance()->webSettings()->testAttribute(QWebSettings::PrivateBrowsingEnabled))
        return;
    m_lastOutgoingData = outgoingData;

    QVariant v = request.attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 100));
    QWebPage* webPage = (QWebPage*)(v.value<void*>());
    v = request.attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 102));
    WebView* webView = (WebView*)(v.value<void*>());
    if (!webPage || !webView)
        return;

    v = request.attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 101));
    QWebPage::NavigationType type = (QWebPage::NavigationType)v.toInt();

    if (type!=QWebPage::NavigationTypeFormSubmitted)
        return;

    QString passwordName="";
    QWebFrame* frame = webPage->mainFrame();
    QWebElementCollection inputs = frame->findAllElements("input");
    for (int i = 0; i<inputs.count(); i++) {
        QWebElement element = inputs.at(i);
        QString type = element.attribute("type");

        if (type == "password")
            passwordName = element.attribute("name");
    }

    //Return if storing is not enabled, data for this page is already stored, no password element found in sent data
    if (!isStoringEnabled(request.url()) || isStored(request.url()) || passwordName.isEmpty())
        return;
    //Return if no password form has been sent
    if (!outgoingData.contains((passwordName+"=").toAscii()))
        return;

    QString pass = "";
    QList<QPair<QString, QString> > arguments = QUrl::fromEncoded(QByteArray("http://bla.com/?"+outgoingData)).queryItems();
    for (int i = 0; i<arguments.count(); i++) {
        if (arguments.at(i).first == passwordName) {
            pass = arguments.at(i).second;
            break;
        }
    }

    AutoFillWidget* aWidget = new AutoFillWidget(request.url(), outgoingData, pass);
    webView->addNotification(aWidget);

}
