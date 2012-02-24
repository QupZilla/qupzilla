#ifndef MOUSEGESTURES_H
#define MOUSEGESTURES_H

#include <QDebug>
#include <QObject>
#include <QMouseEvent>

class WebView;
class QjtMouseGestureFilter;
class MouseGestures : public QObject
{
    Q_OBJECT
public:
    explicit MouseGestures(QObject* parent = 0);
    ~MouseGestures();

    bool mousePress(QObject* obj, QMouseEvent* event);
    bool mouseRelease(QObject* obj, QMouseEvent* event);
    bool mouseMove(QObject* obj, QMouseEvent* event);

    void showSettings(QWidget* parent);

private slots:
    void upGestured();
    void downGestured();
    void leftGestured();
    void rightGestured();

    void downRightGestured();
    void downLeftGestured();

    void upDownGestured();
//    void upLeftGestured();
//    void upRightGestured();

private:
    QjtMouseGestureFilter* m_filter;
    WebView* m_view;

};

#endif // MOUSEGESTURES_H
