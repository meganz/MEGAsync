#ifndef AVATARWIDGET_H
#define AVATARWIDGET_H

#include <QWidget>

class AvatarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AvatarWidget(QWidget *parent = 0);
    ~AvatarWidget();

    void setAvatarLetter(QChar letter, const char* color);
    void setAvatarImage(QString pathToFile);
    void clearData();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *event);

private:
    QChar letter;
    QString pathToFile;
    const char *color;

};

#endif // AVATARWIDGET_H
