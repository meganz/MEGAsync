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
    void drawAvatarFromEmail(QString email);
    void clearData();

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);

    QPixmap mask_image(QString pathToFile, int size);

private:
    QPixmap backgroundPixmapOriginal;
    QPixmap backgroundPixmap;
    QChar letter;
    QString pathToFile;
    QString color;

    int lastloadedwidth;
};

#endif // AVATARWIDGET_H
