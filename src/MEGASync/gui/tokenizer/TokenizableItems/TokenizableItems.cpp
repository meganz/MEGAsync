#include "TokenizableItems.h"

#include "ButtonTokensByType.h"
#include "EnumConverters.h"
#include "IconTokenizer.h"
#include "Preferences.h"
#include "TokenParserWidgetManager.h"

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

    int currentThemeType = static_cast<int>(Preferences::instance()->getThemeType());

    if (mThemeType != currentThemeType)
    {
        mThemeType = currentThemeType;
        result = true;
    }

    if (option.state != mCurrentOption.state)
    {
        mCurrentOption = option;
        result = true;
    }

    return result;
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
        IconTokenizer::tokenizeButtonIcon(button,
                                          mode,
                                          state,
                                          TokenParserWidgetManager::instance()->getColor(token));
    }
}

void TokenizableItem::clear()
{
    mInit = false;
    mBaseTokens = BaseTokens();
    mCurrentOption = QStyleOptionButton();
    mThemeType = -1;
}

bool TokenizableItem::isInitialized() const
{
    return mInit;
}

void TokenizableItem::init(QAbstractButton* button)
{
    if (!isInitialized())
    {
        mBaseTokens.fillTokens(button);

        // Init QIcon::Mode::Active and QIcon::Mode::Disabled
        if (!mBaseTokens.getNormalOffToken().isEmpty())
        {
            applyPixmap(button,
                        mBaseTokens.getNormalOffToken(),
                        QIcon::Mode::Active,
                        QIcon::State::Off);

            if (mBaseTokens.getDisabledOffToken().isEmpty())
            {
                applyPixmap(button,
                            mBaseTokens.getNormalOffToken(),
                            QIcon::Mode::Disabled,
                            QIcon::State::Off);
            }
        }

        if (!mBaseTokens.getNormalOnToken().isEmpty())
        {
            applyPixmap(button,
                        mBaseTokens.getNormalOnToken(),
                        QIcon::Mode::Active,
                        QIcon::State::On);

            if (!mBaseTokens.getNormalOnToken().isEmpty())
            {
                applyPixmap(button,
                            mBaseTokens.getNormalOnToken(),
                            QIcon::Mode::Disabled,
                            QIcon::State::On);
            }
        }
    }

    mInit = true;
}
