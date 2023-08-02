#ifndef WORDWRAPLABEL_H
#define WORDWRAPLABEL_H

#include <QTextEdit>

class WordWrapLabel : public QTextEdit
{
public:
    WordWrapLabel(QWidget *parent);

    void setText(const QString &text);

protected:
    void resizeEvent(QResizeEvent *e) override;

private:
    void adaptHeight(bool sendEvent = false);
};

#endif // WORDWRAPLABEL_H
