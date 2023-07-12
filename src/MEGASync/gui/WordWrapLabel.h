#ifndef WORDWRAPLABEL_H
#define WORDWRAPLABEL_H

#include <QTextEdit>

class WordWrapLabel : public QTextEdit
{
public:
    WordWrapLabel(QWidget *parent);

    void setText(const QString &text);

protected:
    QSize sizeHint() const;
    void resizeEvent(QResizeEvent *e) override;
};

#endif // WORDWRAPLABEL_H
