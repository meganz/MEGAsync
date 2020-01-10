#ifndef ELIDEDLABEL_H
#define ELIDEDLABEL_H

#include <QFrame>
#include <QtCore/QRect>
#include <QtGui/QResizeEvent>
#include <QtCore/QString>
#include <QWidget>

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
