#ifndef PASSWORDLINEEDIT_H
#define PASSWORDLINEEDIT_H

#include <QIcon>
#include <QLineEdit>

class PasswordLineEdit: public QLineEdit
{
    Q_OBJECT

public:
    explicit PasswordLineEdit(QWidget* parent = nullptr);

    Q_PROPERTY(QString eyeRevealImage READ getEyeRevealImage WRITE setEyeRevealImage NOTIFY
                   eyeImageChanged)
    Q_PROPERTY(QString eyeRevealDisabledImage READ getEyeRevealDisabledImage WRITE
                   setEyeRevealDisabledImage NOTIFY eyeImageChanged)
    Q_PROPERTY(QString eyeClosedImage READ getEyeClosedImage WRITE setEyeClosedImage NOTIFY
                   eyeImageChanged)
    Q_PROPERTY(QString eyeClosedDisabledImage READ getEyeClosedDisabledImage WRITE
                   setEyeClosedDisabledImage NOTIFY eyeImageChanged)

    QString getEyeRevealImage() const;
    void setEyeRevealImage(const QString& eyeRevealImage);

    QString getEyeRevealDisabledImage() const;
    void setEyeRevealDisabledImage(const QString& eyeRevealDisabledImage);

    QString getEyeClosedImage() const;
    void setEyeClosedImage(const QString& eyeClosedImage);

    QString getEyeClosedDisabledImage() const;
    void setEyeClosedDisabledImage(const QString& eyeClosedDisabledImage);

signals:
    void eyeImageChanged();

private:
    void setEyeImage(QAction* action, const QString& imagePath, QIcon::Mode mode);
    void revealPassword();
    void hidePassword();

    void focusInEvent(QFocusEvent* e) override;
    void focusOutEvent(QFocusEvent* e) override;

    QAction* mHideAction;
    QAction* mShowAction;
};

#endif // PASSWORDLINEEDIT_H
