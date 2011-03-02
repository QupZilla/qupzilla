#ifndef SOURCEVIEWER_H
#define SOURCEVIEWER_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QWidget>
#include <QBoxLayout>
#include <QTextEdit>
#include <QApplication>

class QupZilla;
class SourceViewer : public QWidget
{
    Q_OBJECT
public:
    explicit SourceViewer(QupZilla* mainClass, QWidget *parent = 0);

signals:

public slots:

private:
    QupZilla* p_QupZilla;

    QBoxLayout* m_layout;
    QTextEdit* m_sourceEdit;

};

#endif // SOURCEVIEWER_H
