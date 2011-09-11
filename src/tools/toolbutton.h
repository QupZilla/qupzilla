#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <QToolButton>
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOptionToolButton>

class ToolButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY(QSize fixedsize READ size WRITE setFixedSize)
    Q_PROPERTY(int fixedwidth READ width WRITE setFixedWidth)
    Q_PROPERTY(int fixedheight READ height WRITE setFixedHeight)
    Q_PROPERTY(QPixmap multiIcon READ pixmap WRITE setMultiIcon)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(QString themeIcon READ themeIcon WRITE setThemeIcon)

public:
    explicit ToolButton(QWidget* parent = 0);

    void setData(const QVariant& data);
    QVariant data();

    void setMultiIcon(const QPixmap &icon);
    QPixmap pixmap() { return m_normalIcon; }

    void setThemeIcon(const QString &icon);
    QString themeIcon() { return m_themeIcon; }

    void setIcon(const QIcon &icon);

signals:

public slots:

private:
    void paintEvent(QPaintEvent *e);

    bool m_usingMultiIcon;

    QPixmap m_normalIcon;
    QPixmap m_hoverIcon;
    QPixmap m_activeIcon;
    QPixmap m_disabledIcon;

    QString m_themeIcon;
    QVariant m_data;

};

#endif // TOOLBUTTON_H
