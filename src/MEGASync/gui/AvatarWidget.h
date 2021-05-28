#ifndef AVATARWIDGET_H
#define AVATARWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QGraphicsDropShadowEffect>

class AvatarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AvatarWidget(QWidget* parent = 0);

    void setAvatarLetter(QChar letter, const QColor& color);
    void setAvatarImage(const QString& pathToFile);
    void drawAvatarFromEmail(const QString& email);
    void clearData();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);

    QPixmap mask_image(const QString& pathToFile, int size);

private:
    QString mPathToFile;
    QLinearGradient mGradient;
    QLabel mLetter;
    QGraphicsDropShadowEffect* mLetterShadow;
};

#endif // AVATARWIDGET_H
