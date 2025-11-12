#ifndef ELIDEDLABEL_H
#define ELIDEDLABEL_H

#include <QFrame>
#include <QLabel>
#include <QPainter>
#include <QStyleOption>
#include <QtCore/QRect>
#include <QtCore/QString>
#include <QtGui/QResizeEvent>
#include <QWidget>

class ElidingLabel: public QLabel
{
    Q_OBJECT

public:
    explicit ElidingLabel(QWidget* parent = nullptr):
        QLabel(parent)
    {
        setMinimumWidth(0);
        setTextFormat(Qt::PlainText);
        setWordWrap(false);
    }

    void setText(const QString& text)
    {
        if (sizePolicy().horizontalPolicy() != QSizePolicy::Preferred ||
            sizePolicy().horizontalPolicy() != QSizePolicy::Expanding)
        {
            setSizePolicy(QSizePolicy::Preferred, sizePolicy().verticalPolicy());
        }

        QLabel::setText(text);
        // Refresh tooltip
        setToolTip(QString());
        update();
    }

    QSize minimumSizeHint() const override
    {
        return QSize(0, 0);
    }

    void setElideMode(Qt::TextElideMode mode)
    {
        if (mElidemode != mode)
        {
            mElidemode = mode;
            update();
        }
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);

        // Paint background
        if (testAttribute(Qt::WA_StyledBackground))
        {
            QStyleOption opt;
            opt.initFrom(this);
            style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
        }

        // Elide
        const QRect cr = contentsRect();
        const QFontMetrics fm(font());
        const QString elided = fm.elidedText(text(), mElidemode, cr.width());

        if (toolTip().isEmpty() && elided != text())
        {
            setToolTip(text());
        }
        else if (!toolTip().isEmpty() && elided == text())
        {
            setToolTip(QString());
        }

        // Paint elided text
        style()->drawItemText(&p,
                              cr,
                              static_cast<int>(alignment()),
                              palette(),
                              isEnabled(),
                              elided,
                              QPalette::WindowText);
    }

private:
    Qt::TextElideMode mElidemode = Qt::ElideMiddle;
};

class ElidedLabel : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(bool isElided READ isElided)
    Q_PROPERTY(bool singleline READ getSingleline WRITE setSingleline)

public:
    explicit ElidedLabel(QWidget *parent = 0);

    void setText(const QString &text);
    const QString & text() const { return content; }
    bool isElided() const { return elided; }

    bool getSingleline() const;
    void setSingleline(bool value);

protected:

    void paintEvent(QPaintEvent *event);

signals:
    void elisionChanged(bool elided);

private:
    bool singleline = false;
private:
    bool elided;
    QString content;
};

#endif // ELIDEDLABEL_H
