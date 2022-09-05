#ifndef AVATARATTRIBUTESREQUESTS_H
#define AVATARATTRIBUTESREQUESTS_H

#include <control/UserAttributesManager.h>

#include <QColor>
#include <QPixmap>

namespace UserAttributes
{

class FullName;

class Avatar : public AttributeRequest
{
    Q_OBJECT

public:

    Avatar(const QString& userEmail);

    static std::shared_ptr<const Avatar> requestAvatar(const char* user_email);
    std::shared_ptr<FullName> getFullName();

    void onRequestFinish(mega::MegaApi *, mega::MegaRequest *incoming_request, mega::MegaError *e) override;
    void requestAttribute() override;
    void updateAttributes(mega::MegaUser* user) override;

    const QPixmap& getPixmap(const int& size) const;

    bool isAttributeReady() const override;

signals:
    void attributeReady();

private slots:
    void onFullNameAttributeReady();

private:
    struct LetterInfo
    {
        QChar letter;
        QColor color;

        bool isEmpty() const {return letter.isNull();}
        void clear()
        {
            letter = QChar();
            color = QColor();
        }
    };

    void fillLetterInfo();
    void getLetterColor();

    mutable QMap<int,QPixmap> mIcon;
    QString mIconPath;
    LetterInfo mLetterAvatarInfo;
    std::shared_ptr<FullName> mFullName;
};
}

#endif // USERATTRIBUTESREQUESTS_H
