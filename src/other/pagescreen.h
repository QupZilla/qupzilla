#ifndef PAGESCREEN_H
#define PAGESCREEN_H

#include <QWidget>
#include <QWebFrame>
#include <QDialogButtonBox>
#include <QAbstractButton>
#include <QFileDialog>

namespace Ui {
    class PageScreen;
}

class WebView;
class PageScreen : public QWidget
{
    Q_OBJECT

public:
    explicit PageScreen(WebView* view);
    ~PageScreen();

private slots:
    void buttonClicked(QAbstractButton* b);

private:
    void createPixmap();

    Ui::PageScreen *ui;
    WebView* m_view;
    QPixmap m_pagePixmap;
};

#endif // PAGESCREEN_H
