#include "TrayIconManager.h"

#include "Platform.h"

namespace
{

struct IconEntry
{
    const char* stateName;
    const char* filename;
};

static constexpr IconEntry ICON_TABLE[] = {
    {"warning", "tray_icon_blocked.svg"},
    {"synching", "tray_icon_default.svg"},
    {"uptodate", "tray_icon_default.svg"},
    {"paused", "tray_icon_paused.svg"},
    {"logging", "tray_icon_default.svg"},
    {"alert", "tray_icon_blocked.svg"},
    {"someissues", "tray_icon_blocked.svg"},
};

} // namespace

const TrayIconManager::AnimationDef TrayIconManager::ANIMATION_DEFS[] = {
    /* Progress */ {"tray_icon_progress_frame_%1.svg", 30, 60},
    /* Logging  */ {"tray_icon_logging_frame_%1.svg", 30, 60},
    /* Offer    */ {"tray_icon_promo_frame_%1.svg", 60, 30},
};

// ---------------------------------------------------------------------------

TrayIconManager::TrayIconManager(QObject* parent):
    QObject(parent)
{}

TrayIconManager::~TrayIconManager() = default;

void TrayIconManager::initTrayIcon()
{
    if (mTrayIcon)
    {
        return;
    }

    mTrayIcon = new QSystemTrayIcon(this);

    connect(mTrayIcon, &QSystemTrayIcon::activated, this, &TrayIconManager::activated);
    connect(mTrayIcon, &QSystemTrayIcon::messageClicked, this, &TrayIconManager::messageClicked);

    mAnimationTimer = new QTimer(this);
    mAnimationTimer->setSingleShot(false);
    connect(mAnimationTimer, &QTimer::timeout, this, &TrayIconManager::onAnimationStep);

#ifndef Q_OS_MACOS
    connect(Platform::getInstance(),
            &AbstractPlatform::themeChanged,
            this,
            [this]()
            {
                updateTheme();
                loadIcons();
            });
#endif

    updateTheme();
    loadIcons();
}

QString TrayIconManager::themePrefix() const
{
#ifdef Q_OS_LINUX
    return QStringLiteral(":/reddish/");
#else
    return (mTheme == Theme::Dark) ? QStringLiteral(":/dark/") : QStringLiteral(":/light/");
#endif
}

QIcon TrayIconManager::loadIcon(const QString& filename) const
{
    QIcon icon(themePrefix() + filename);

#ifdef __APPLE__
    if (!icon.isNull())
    {
        icon.setIsMask(true);
    }
#endif

    return icon;
}

void TrayIconManager::loadIcons()
{
    // Static icons
    mStaticIcons.clear();
    for (const auto& entry: ICON_TABLE)
    {
        mStaticIcons[QString::fromLatin1(entry.stateName)] =
            loadIcon(QString::fromLatin1(entry.filename));
    }

    // Animation frame sets — one QVector<QIcon> per Animation type
    mAnimationFrameSets.clear();
    const Animation types[] = {Animation::Progress, Animation::Logging, Animation::Promo};
    for (auto type: types)
    {
        const auto& def = ANIMATION_DEFS[static_cast<int>(type)];
        QVector<QIcon> frames;
        frames.reserve(def.frameCount);
        for (int i = 1; i <= def.frameCount; ++i)
        {
            frames.append(loadIcon(QString::fromLatin1(def.filenameTemplate).arg(i)));
        }
        mAnimationFrameSets[type] = std::move(frames);
    }
}

void TrayIconManager::setIcon(const QString& stateName)
{
    mCurrentStateName = stateName;
    if (!isAnimating())
    {
        applyCurrentStaticIcon();
    }
}

void TrayIconManager::setIconAndTooltip(const QString& stateName, const QString& tooltip)
{
    setIcon(stateName);
    setTooltip(tooltip);
}

void TrayIconManager::applyCurrentStaticIcon()
{
    auto it = mStaticIcons.find(mCurrentStateName);
    if (it != mStaticIcons.end())
    {
        applyIcon(it.value());
    }
}

void TrayIconManager::applyIcon(const QIcon& icon)
{
    if (mTrayIcon)
    {
        mTrayIcon->setIcon(icon);
    }
}

void TrayIconManager::startAnimation(Animation animation)
{
    const bool animationChanged = (mCurrentAnimation != animation);
    mCurrentAnimation = animation;

    // Update the timer interval for this animation's speed
    const auto& def = ANIMATION_DEFS[static_cast<int>(animation)];
    mAnimationTimer->setInterval(def.intervalMs);

    if (mAnimationTimer->isActive())
    {
        // Already running — reset frame index only if the animation changed
        if (animationChanged)
        {
            mAnimationIndex = 0;
        }
        return;
    }

    mAnimationIndex = 0;
    mAnimationTimer->start();
}

void TrayIconManager::stopAnimation()
{
    if (mAnimationTimer && mAnimationTimer->isActive())
    {
        mAnimationTimer->stop();
    }

    applyCurrentStaticIcon();
}

bool TrayIconManager::isAnimating() const
{
    return mAnimationTimer && mAnimationTimer->isActive();
}

void TrayIconManager::onAnimationStep()
{
    if (!mTrayIcon)
    {
        return;
    }

    const auto& frames = mAnimationFrameSets.value(mCurrentAnimation);
    if (frames.isEmpty())
    {
        return;
    }

    mAnimationIndex = mAnimationIndex % frames.size();
    applyIcon(frames[mAnimationIndex]);
    ++mAnimationIndex;
}

void TrayIconManager::setTooltip(const QString& tooltip)
{
    if (mTrayIcon)
    {
        mTrayIcon->setToolTip(tooltip);
    }
}

void TrayIconManager::updateTheme()
{
#ifndef Q_OS_MACOS

    auto scheme = Platform::getInstance()->getPanelTheme();
    mTheme = (scheme == Preferences::ThemeAppeareance::DARK) ? Theme::Dark : Theme::Light;
#else
    mTheme = Theme::Dark;
#endif
}

TrayIconManager::Theme TrayIconManager::theme() const
{
    return mTheme;
}

void TrayIconManager::show()
{
    if (mTrayIcon)
    {
        mTrayIcon->show();
    }
}

void TrayIconManager::hide()
{
    if (mTrayIcon)
    {
        mTrayIcon->hide();
    }
}

QSystemTrayIcon* TrayIconManager::trayIcon() const
{
    return mTrayIcon;
}
