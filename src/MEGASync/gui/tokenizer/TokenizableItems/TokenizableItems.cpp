#include "TokenizableItems.h"

#include "IconTokenizer.h"
#include "Preferences.h"
#include "ThemeManager.h"

#include <QDateTime>
#include <QDebug>

TokenizableItem::TokenizableItem():
    mInit(false),
    mThemeType(static_cast<int>(Preferences::instance()->getThemeType()))
{}

TokenizableItem::~TokenizableItem() {}

bool TokenizableItem::stateHasChanged(const QStyleOption& option)
{
    auto result(false);
    int currentThemeType = static_cast<int>(ThemeManager::instance()->currentColorScheme());

    if (mThemeType != currentThemeType)
    {
        mThemeType = currentThemeType;
        result = true;
    }

    if (specificStateHasChanged(QStyle::State_Enabled, option) ||
        specificStateHasChanged(QStyle::State_Sunken, option) ||
        specificStateHasChanged(QStyle::State_MouseOver, option) ||
        specificStateHasChanged(QStyle::State_Off, option) ||
        specificStateHasChanged(QStyle::State_On, option))
    {
        mCurrentOption = option;
        result = true;
    }

    return result;
}

bool TokenizableItem::themeHasChanged() const
{
    int currentThemeType = static_cast<int>(ThemeManager::instance()->currentColorScheme());

    return mThemeType != currentThemeType;
}

void TokenizableItem::applyDefaultPixmap(QAbstractButton* button)
{
    QIcon::State state = button->isChecked() ? QIcon::State::On : QIcon::State::Off;

    if (button->isEnabled())
    {
        auto token = state == QIcon::State::Off ? mBaseTokens.getNormalOffToken() :
                                                  mBaseTokens.getNormalOnToken();

        QIcon::Mode mode = button->hasFocus() ? QIcon::Active : QIcon::Normal;

        applyPixmap(button, token, mode, state);
    }
    else
    {
        auto token = state == QIcon::State::Off ? mBaseTokens.getDisabledOffToken() :
                                                  mBaseTokens.getDisabledOnToken();
        applyPixmap(button, token, QIcon::Disabled, state);
    }
}

void TokenizableItem::applyPixmap(QAbstractButton* button,
                                  const QString& token,
                                  const QIcon::Mode& mode,
                                  const QIcon::State& state)
{
    if (button && !token.isEmpty())
    {
        IconTokenizer::tokenizeButtonIcon(button, mode, state, token);
    }
}

void TokenizableItem::forceUpdate()
{
    mInit = false;
    mCurrentOption = QStyleOptionButton();
    mThemeType = -1;
}

bool TokenizableItem::isInitialized() const
{
    return mInit;
}

void TokenizableItem::init(QAbstractButton* button)
{
    mBaseTokens.fillTokens(button);
    if (mBaseTokens.anyTokenHasChanged())
    {
        forceUpdate();
    }

    if (!isInitialized())
    {
        mBaseTokens.fillTokens(button);

        mInit = true;
    }
}

bool TokenizableItem::specificStateHasChanged(const QStyle::State& state,
                                              const QStyleOption& option)
{
    return ((option.state & state) != (mCurrentOption.state & state));
}
