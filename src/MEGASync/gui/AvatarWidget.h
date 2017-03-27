#ifndef AVATARWIDGET_H
#define AVATARWIDGET_H

#include <QWidget>

class AvatarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AvatarWidget(QWidget *parent = 0);
    ~AvatarWidget();

    void setAvatarLetter(QChar letter, QString color);
    void setAvatarImage(QString pathToFile);
    void clearData();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *event);

private:
    QChar letter;
    QString pathToFile;
    QString color;

};

#endif // AVATARWIDGET_H
