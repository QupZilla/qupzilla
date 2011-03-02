#include "siteinfo.h"
#include "ui_siteinfo.h"
#include "qupzilla.h"
#include "webview.h"

SiteInfo::SiteInfo(QupZilla* mainClass, QWidget *parent) :
    QDialog(parent)
    ,ui(new Ui::SiteInfo)
    ,p_QupZilla(mainClass)
{
    ui->setupUi(this);
    WebView* view = p_QupZilla->weView();
    QWebFrame* frame = view->page()->mainFrame();
    QString title = view->title();
    if (title.isEmpty())
        title = tr("No Named Page");

    ui->siteName->setText(title);
    ui->siteAddress->setText(frame->baseUrl().toString());

    QWebElementCollection meta = frame->findAllElements("meta");
    for (int i = 0; i<meta.count(); i++) {
        QWebElement element = meta.at(i);

        QString content = element.attribute("content");
        QString name = element.attribute("name");
        if (name.isEmpty())
            name = element.attribute("http-equiv");

        if (content.isEmpty() || name.isEmpty())
            continue;
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeTags);
        item->setText(0, name);
        item->setText(1, content);
        ui->treeTags->addTopLevelItem(item);
    }

    QWebElementCollection img = frame->findAllElements("img");
    for (int i = 0; i<img.count(); i++) {
        QWebElement element = img.at(i);

        QString src = element.attribute("src");
        QString alt = element.attribute("alt");
        if (alt.isEmpty()) {
            if (src.indexOf("/") == -1)
                alt = src;
            else{
                int pos = src.lastIndexOf("/");
                alt = src.mid(pos);
                alt.remove("/");
            }
        }
        if (src.isEmpty() || alt.isEmpty())
            continue;
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeImages);
        item->setText(0, alt);
        item->setText(1, src);
        ui->treeImages->addTopLevelItem(item);
    }

}

SiteInfo::~SiteInfo()
{
    delete ui;
}
