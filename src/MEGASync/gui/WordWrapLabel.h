#ifndef WORDWRAPLABEL_H
#define WORDWRAPLABEL_H

#include <QEvent>
#include <QTextBrowser>
#include <QTimer>

class WordWrapLabel : public QTextBrowser
{
    Q_OBJECT

public:
    static const QEvent::Type HeightAdapted;

    WordWrapLabel(QWidget *parent);

    void setMaximumLines(int8_t lines);
    void setMaximumHeight(int maxHeight);
    void resetSizeLimits();

    void setText(const QString& text);

    //Try not to use maxLines/maxHeight with rich text strings, as it could potentially remove the hmtl tags when eliding
    void setTextFormat(Qt::TextFormat format);

protected:
    void resizeEvent(QResizeEvent *e) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;

private slots:
    void onLinkActivated(const QUrl& link);
    void onAdaptHeight();

private:
    void setCursor(const QCursor& cursor);

    bool mLinkActivated;
    int mMaxHeight;
    int8_t mMaxLines;
    QString mText;
    Qt::TextFormat mFormat;
    QTimer mAdaptHeightTimer;
};

#endif // WORDWRAPLABEL_H
