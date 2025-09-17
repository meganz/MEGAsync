#ifndef TOKENIZABLEITEMS_H
#define TOKENIZABLEITEMS_H

#include "TokenPropertyNames.h"

#include <QAbstractButton>
#include <QStyleOptionButton>

class BaseTokens
{
public:
    BaseTokens() = default;

    virtual void fillTokens(QAbstractButton* button)
    {
        mNormalOff = button->property(TOKEN_PROPERTIES::normalOff).toString();
        mNormalOn = button->property(TOKEN_PROPERTIES::normalOn).toString();
        mDisabledOff = button->property(TOKEN_PROPERTIES::disabledOff).toString();
        mDisabledOn = button->property(TOKEN_PROPERTIES::disabledOn).toString();
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

    virtual void clear();

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

private:
    bool specificStateHasChanged(const QStyle::State& state, const QStyleOption& option);

    BaseTokens mBaseTokens;
    QStyleOption mCurrentOption;
    bool mInit;
    int mThemeType;
    bool mTokenChanged;
};

#endif // TOKENIZABLEITEMS_H
