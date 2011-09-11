#include "toolbutton.h"

ToolButton::ToolButton(QWidget* parent)
    : QToolButton(parent)
    , m_usingMultiIcon(false)
{
}

void ToolButton::setThemeIcon(const QString &icon)
{
    m_themeIcon = icon;
    setIcon(QIcon::fromTheme(icon));
    m_usingMultiIcon = false;
}

void ToolButton::setIcon(const QIcon &icon)
{
    if (m_usingMultiIcon)
        setFixedSize(sizeHint());
    m_usingMultiIcon = false;

    QToolButton::setIcon(icon);
}

void ToolButton::setData(const QVariant &data)
{
    m_data = data;
}

QVariant ToolButton::data()
{
    return m_data;
}

void ToolButton::setMultiIcon(const QPixmap &icon)
{
    int w = icon.width();
    int h = icon.height();

    m_normalIcon = icon.copy(0, 0, w, h/4 );
    m_hoverIcon = icon.copy(0, h/4, w, h/4 );
    m_activeIcon = icon.copy(0, h/2, w, h/4 );
    m_disabledIcon = icon.copy(0, 3*h/4, w, h/4 );

    m_usingMultiIcon = true;

    setFixedSize(m_normalIcon.size());
}

void ToolButton::paintEvent(QPaintEvent *e)
{
    if (!m_usingMultiIcon) {
        QToolButton::paintEvent(e);
        return;
    }

    QPainter p(this);

    QStyleOptionToolButton opt;
    opt.init(this);

    if (!isEnabled()) {
        p.drawPixmap(0, 0, m_disabledIcon);
        return;
    }

    if (isDown()) {
        p.drawPixmap(0, 0, m_activeIcon);
        return;
    }

    if (opt.state & QStyle::State_MouseOver) {
        p.drawPixmap(0, 0, m_hoverIcon);
        return;
    }

    p.drawPixmap(0, 0, m_normalIcon);
}
