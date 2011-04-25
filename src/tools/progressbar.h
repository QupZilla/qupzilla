#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <QStylePainter>
#include <QStyleOptionProgressBarV2>
#include <QStyleOptionProgressBar>

class ProgressBar : public QWidget
{
    Q_OBJECT
public:
    explicit ProgressBar(QWidget* parent = 0);

signals:

public slots:
    void setValue(int value);

protected:
    void paintEvent(QPaintEvent* e);
    void initStyleOption(QStyleOptionProgressBar* option);

private:
    int m_value;
    int m_lastPaintedValue;

};

#endif // PROGRESSBAR_H
