#ifndef WORDWRAPLABEL_H
#define WORDWRAPLABEL_H

#include <QEvent>
#include <QTextBrowser>

class WordWrapLabel : public QTextBrowser
{
    Q_OBJECT

public:
    static const QEvent::Type HeightAdapted;

    WordWrapLabel(QWidget *parent);

    void setText(const QString &text);

protected:
    void resizeEvent(QResizeEvent *e) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;

private slots:
    void onLinkActivated(const QUrl& link);

private:
    void setCursor(const QCursor &cursor);
    void adaptHeight(bool sendEvent = false);

    bool mLinkActivated;
};

#endif // WORDWRAPLABEL_H
