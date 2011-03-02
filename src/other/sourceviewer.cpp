#include "sourceviewer.h"
#include "qupzilla.h"
#include "webview.h"
SourceViewer::SourceViewer(QupZilla* mainClass, QWidget *parent) :
    QWidget(parent)
    ,p_QupZilla(mainClass)
{
    setWindowTitle(tr("Source of ")+p_QupZilla->weView()->url().toString());
    setWindowIcon(QIcon(":/icons/qupzilla.png"));
    m_layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    m_sourceEdit = new QTextEdit(this);

    m_layout->addWidget(m_sourceEdit);
    m_layout->setContentsMargins(1, 2, 1, 2);

    m_sourceEdit->insertPlainText(p_QupZilla->weView()->page()->mainFrame()->toHtml());

    this->resize(650, 600);
    m_sourceEdit->setReadOnly(true);
    m_sourceEdit->moveCursor(QTextCursor::Start);
    //CENTER on scren
    const QRect screen = QApplication::desktop()->screenGeometry();
    const QRect &size = QWidget::geometry();
    QWidget::move( (screen.width()-size.width())/2, (screen.height()-size.height())/2 );
}
