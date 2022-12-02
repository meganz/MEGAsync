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

    static std::shared_ptr<const Avatar> requestAvatar(const char* user_email = nullptr);

    void onRequestFinish(mega::MegaApi *, mega::MegaRequest *incoming_request, mega::MegaError *e) override;
    void requestAttribute() override;
    RequestInfo fillRequestInfo() override;

    const QPixmap& getPixmap(const int& size) const;

    bool isAttributeReady() const override;

signals:
    void attributeReady();

private slots:
    void onFullNameAttributeReady();

private:
    struct LetterInfo
    {
        QString symbol;
        QColor primaryColor;
        QColor secondaryColor;
        bool colorNeedsRefresh;

        bool isEmpty() const {return symbol.isNull();}
        void clear()
        {
            symbol.clear();
            primaryColor = QColor();
            secondaryColor = QColor();
            colorNeedsRefresh = true;
        }
    };

    void fillLetterInfo();
    void getLetterColor();

    mutable QMap<int,QPixmap> mIcon;
    QString mIconPath;
    LetterInfo mLetterAvatarInfo;
    std::shared_ptr<const FullName> mFullName;
    bool mUseImgFile;
};
}

#endif // USERATTRIBUTESREQUESTS_H
