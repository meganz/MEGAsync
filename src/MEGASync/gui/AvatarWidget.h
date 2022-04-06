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


private:
    QString mLetter;
    QString mPathToFile;
    QLinearGradient mGradient;
};

class AvatarPixmap
{
public:
    static QPixmap maskFromImagePath(const QString& pathToFile, int size);
    static QPixmap createFromLetter(const QString& letter, QLinearGradient gradient, int size);
};

#endif // AVATARWIDGET_H
