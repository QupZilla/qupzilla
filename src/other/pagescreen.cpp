#include "pagescreen.h"
#include "ui_pagescreen.h"
#include "webview.h"
#include "globalfunctions.h"

PageScreen::PageScreen(WebView *view)
    : QWidget()
    , ui(new Ui::PageScreen)
    , m_view(view)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    qz_centerWidgetOnScreen(this);

    createPixmap();
    ui->label->setPixmap(m_pagePixmap);

    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
}

void PageScreen::buttonClicked(QAbstractButton* b)
{
    QString path;

    switch (ui->buttonBox->standardButton(b)) {
    case QDialogButtonBox::Cancel:
        close();
        break;

    case QDialogButtonBox::Save:
        path = QFileDialog::getSaveFileName(this, tr("Save Page Screen..."), tr("screen.png"));
        if (!path.isEmpty())
            m_pagePixmap.save(path);
        break;

    default:
        break;
    }
}

void PageScreen::createPixmap()
{
    QWebPage* page = m_view->page();
    QSize originalSize = page->viewportSize();
    page->setViewportSize(page->mainFrame()->contentsSize());

    QImage image(page->viewportSize(), QImage::Format_ARGB32);
    QPainter painter(&image);
    page->mainFrame()->render(&painter);
    painter.end();

    m_pagePixmap = QPixmap::fromImage(image);

    page->setViewportSize(originalSize);
}

PageScreen::~PageScreen()
{
    delete ui;
}
