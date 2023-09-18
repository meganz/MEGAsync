#ifndef WORDWRAPLABEL_H
#define WORDWRAPLABEL_H

#include <QEvent>
#include <QTextEdit>

class WordWrapLabel : public QTextEdit
{
public:
    static const QEvent::Type HeightAdapted;

    WordWrapLabel(QWidget *parent);

    void setText(const QString &text);

protected:
    void resizeEvent(QResizeEvent *e) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setCursor(const QCursor &cursor);
    void adaptHeight(bool sendEvent = false);
};

#endif // WORDWRAPLABEL_H
