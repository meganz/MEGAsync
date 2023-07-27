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
    void adaptHeight();
};

#endif // WORDWRAPLABEL_H
