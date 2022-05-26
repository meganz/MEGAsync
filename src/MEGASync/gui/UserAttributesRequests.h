#ifndef USERATTRIBUTESREQUESTS_H
#define USERATTRIBUTESREQUESTS_H

#include <control/UserAttributesManager.h>

namespace UserAttributes
{
class FullNameAttributeRequest : public AttributeRequest
{
    Q_OBJECT

public:
    FullNameAttributeRequest(const QString& userEmail) : AttributeRequest(userEmail){}

    static std::shared_ptr<const FullNameAttributeRequest> requestFullName(const char* user_email);

    void onRequestFinish(mega::MegaApi *, mega::MegaRequest *incoming_request, mega::MegaError *e) override;
    void requestAttribute() override;
    void updateAttributes(mega::MegaUser* user) override;

    QString getFullName() const;
    bool isAttributeReady() const override;
    const QString& getFirstName() const;
    const QString& getLastName() const;

signals:
    void attributeReady(const QString&);

private:
    QString mFirstName;
    QString mLastName;
};

class AvatarAttributeRequest : public AttributeRequest
{
    Q_OBJECT

public:

    AvatarAttributeRequest(const QString& userEmail);

    static std::shared_ptr<const AvatarAttributeRequest> requestAvatar(const char* user_email);
    std::shared_ptr<FullNameAttributeRequest> getFullNameRequest();

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
    std::shared_ptr<FullNameAttributeRequest> mFullNameRequest;
};
}

#endif // USERATTRIBUTESREQUESTS_H
