#ifndef TOKENIZABLEITEMS_H
#define TOKENIZABLEITEMS_H

#include "TokenPropertyNames.h"

#include <QAbstractButton>
#include <QStyleOptionButton>

class TokenUtilities
{
public:
    TokenUtilities() = default;
    ~TokenUtilities() = default;

    bool anyTokenHasChanged() const
    {
        return mTokenHasChanged;
    }

protected:
    void fillToken(QString& token, const char* propertyName, QAbstractButton* button)
    {
        QString propertyValue(button->property(propertyName).toString());
        if (!propertyValue.isEmpty() && propertyValue != token)
        {
            if (!token.isEmpty())
            {
                mTokenHasChanged = true;
            }

            token = propertyValue;
        }
    }

private:
    bool mTokenHasChanged = false;
};

class BaseTokens: public TokenUtilities
{
public:
    BaseTokens() = default;

    virtual void fillTokens(QAbstractButton* button)
    {
        fillToken(mNormalOff, TOKEN_PROPERTIES::normalOff, button);
        fillToken(mNormalOn, TOKEN_PROPERTIES::normalOn, button);
        fillToken(mDisabledOff, TOKEN_PROPERTIES::disabledOff, button);
        fillToken(mDisabledOn, TOKEN_PROPERTIES::disabledOn, button);
    }

    // normal_off
    QString getNormalOffToken() const
    {
        return mNormalOff;
    }

    void setNormalOff(const QString& newNormalOff)
    {
        mNormalOff = newNormalOff;
    }

    // normal_on
    QString getNormalOnToken() const
    {
        return mNormalOn;
    }

    void setNormalOn(const QString& newNormalOn)
    {
        mNormalOn = newNormalOn;
    }

    // disabled_off
    QString getDisabledOffToken() const
    {
        return mDisabledOff;
    }

    void setDisabledOff(const QString& newDisabledOff)
    {
        mDisabledOff = newDisabledOff;
    }

    // disabled_on
    QString getDisabledOnToken() const
    {
        return mDisabledOn;
    }

    void setDisabledOn(const QString& newDisabledOn)
    {
        mDisabledOn = newDisabledOn;
    }

private:
    QString mNormalOff;
    QString mNormalOn;
    QString mDisabledOff;
    QString mDisabledOn;
};

class TokenizableItem
{
public:
    TokenizableItem();
    ~TokenizableItem();

    virtual void forceUpdate();

protected:
    bool stateHasChanged(const QStyleOption& option);
    bool themeHasChanged() const;

    void applyDefaultPixmap(QAbstractButton* button);
    void applyPixmap(QAbstractButton* button,
                     const QString& token,
                     const QIcon::Mode& mode,
                     const QIcon::State& state);

    bool isInitialized() const;
    virtual void init(QAbstractButton* button);

    BaseTokens mBaseTokens;

private:
    bool specificStateHasChanged(const QStyle::State& state, const QStyleOption& option);

    QStyleOption mCurrentOption;
    bool mInit;
    int mThemeType;
};

#endif // TOKENIZABLEITEMS_H
