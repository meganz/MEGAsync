#include "ThemeManager.h"

#include "Preferences/Preferences.h"

#include <QDir>
#include <QFile>
#include <QWidget>
#include <QtConcurrent/QtConcurrent>

const QMap<Preferences::ThemeType, QString> ThemeManager::mThemesMap = {
    {Preferences::ThemeType::LIGHT_THEME,  QObject::tr("Light")},
    {Preferences::ThemeType::DARK_THEME,  QObject::tr("Dark")}
};

const QMap<Preferences::ThemeType, QString> ThemeManager::mThemePaths = {
    {Preferences::ThemeType::LIGHT_THEME,  QObject::tr("Tokens_Color_Light/")},
    {Preferences::ThemeType::DARK_THEME,  QObject::tr("Tokens_Color_Dark/")}
};


ThemeManager::ThemeManager()
    : QObject(nullptr)
{
    loadStylesheets();
    setTheme(Preferences::instance()->getThemeType());
}

void ThemeManager::loadStylesheets()
{
    QStringList uiCssFiles = getUiCssFiles();

    for (const QString& fileName : uiCssFiles)
    {
        for (auto it = mThemePaths.constBegin(); it != mThemePaths.constEnd(); ++it)
        {
            Preferences::ThemeType theme = it.key();
            const QString& themePath = it.value();

            QString fullThemeFilePath = themePath + fileName;
            QString filePath = getFilePath(fullThemeFilePath);
            QString key = fileName + QLatin1String("_") + themeToString(theme);

            // Load stylesheets corresponding to UI files
            loadStylesheetAsync(filePath, key);
        }
    }
}

QStringList ThemeManager::getUiCssFiles() const
{
    QStringList fileNames;
    const QString folderPath = getCssPath();
    QDir directory(folderPath);

    if (!directory.exists())
    {
        return fileNames;
    }

    QFileInfoList files = directory.entryInfoList(QStringList() << QLatin1String("*.css"), QDir::Files);

    for (const QFileInfo& fileInfo : files)
    {
        const QString fileName = fileInfo.baseName();
        fileNames.append(fileName);
    }

    return fileNames;
}

QString ThemeManager::getCssPath() const
{
    QString basePath;

#if defined(Q_OS_WIN)
    basePath = QLatin1String(":/themes/styles/win/");
#elif defined(Q_OS_LINUX)
    basePath = QLatin1String(":/themes/styles/linux/");
#elif defined(Q_OS_MAC)
    basePath = QLatin1String(":/themes/styles/macx/");
#else
    basePath = QLatin1String("");
#endif

    return basePath + mThemePaths.value(Preferences::instance()->getThemeType(), mThemePaths.value(Preferences::ThemeType::LIGHT_THEME));
}

QString ThemeManager::getFilePath(const QString &themeFilePath) const
{
    QString basePath;

#if defined(Q_OS_WIN)
    basePath = QLatin1String(":/themes/styles/win/");
#elif defined(Q_OS_LINUX)
    basePath = QLatin1String(":/themes/styles/linux/");
#elif defined(Q_OS_MAC)
    basePath = QLatin1String(":/themes/styles/macx/");
#else
    basePath = QLatin1String("");
#endif
    return basePath + themeFilePath + QLatin1String(".css");
}

QString ThemeManager::themeToString(Preferences::ThemeType theme) const
{
    return mThemesMap.value(theme, QLatin1String("Light"));
}

void ThemeManager::loadStylesheetAsync(const QString &filename, const QString &key)
{
    QtConcurrent::run([=]() {
        QFile file(filename);
        if (file.open(QFile::ReadOnly | QFile::Text))
        {
            QString stylesheet = QString::fromUtf8(file.readAll());
            QMetaObject::invokeMethod(this, [=]() {
                    onStylesheetLoaded(stylesheet, key);
                }, Qt::QueuedConnection);
        }
    });
}

void ThemeManager::onStylesheetLoaded(const QString &stylesheet, const QString &key)
{
    parseStyleSheet(stylesheet, key);
}

void ThemeManager::parseStyleSheet(const QString &stylesheet, const QString &uiFileName)
{
    mThemeStylesheetParser.parseStyleSheet(stylesheet, uiFileName);
}

bool ThemeManager::addClassToThemeManager(QWidget *classWidget)
{
    return registerWidgetForTheming(classWidget);
}

bool ThemeManager::registerWidgetForTheming(QWidget *widget)
{
    static QSet<QString> uiCssFiles = []()
    {
        QStringList fileNames = ThemeManager::instance()->getUiCssFiles();
        return QSet<QString>(fileNames.begin(), fileNames.end());
    }();

    if (mWidgetsStyledByCSSFile.contains(widget))
    {
        return true;
    }

    // If the widget's class is among the CSS files add it to the cache.
    const QString className = QString::fromUtf8(widget->metaObject()->className());
    if (uiCssFiles.contains(className))
    {
        addToStyleCache(widget, mWidgetsStyledByCSSFile);
        return true;
    }

    return false;
}

void ThemeManager::addToStyleCache(QObject *item, QList<QObject *> &list)
{
    if(!list.contains(item))
    {
        list.append(item);
        item->connect(item, &QObject::destroyed,[item, &list](){
            list.removeOne(item);
        });
    }
}

void ThemeManager::applyStyleSheet(QWidget *parent)
{
    if (!parent)
    {
        return;
    }

    const QString className = QString::fromUtf8(parent->metaObject()->className());
    const QString themeKey = className % QLatin1String("_") % themeToString(getSelectedTheme());

    QSet<QString> objectNames = getObjectNamesInUICSSFile(themeKey);
    for (const QString &objectName : objectNames)
    {
        QWidget* item = (objectName == className) ? parent : parent->findChild<QWidget*>(objectName);

        if (!item)
        {
            continue;
        }

        const QString key = themeKey % QLatin1String("_") % objectName;
        const QString currentStylesheet = item->styleSheet();
        const QString themeStylesheet = getThemeStylesheet(key); // Retrieve the stylesheet associated with the specified object
        const QString newStylesheet = currentStylesheet % QLatin1String("\n") % themeStylesheet; // Append themeStylesheet to the current stylesheet, overriding relevant parts

        item->setStyleSheet(newStylesheet);
    }
}

QSet<QString> ThemeManager::getObjectNamesInUICSSFile(const QString &themeKey) const
{
    return mThemeStylesheetParser.getObjectNamesInUICSSFile(themeKey);
}

QString ThemeManager::getThemeStylesheet(const QString &key) const
{
    return mThemeStylesheetParser.getThemeStylesheet(key);
}

Preferences::ThemeType ThemeManager::getSelectedTheme() const
{
    return mCurrentTheme;
}

ThemeManager* ThemeManager::instance()
{
    static ThemeManager manager;

    return &manager;
}

QStringList ThemeManager::themesAvailable() const
{
    return mThemesMap.values();
}

void ThemeManager::setTheme(Preferences::ThemeType theme)
{
    if (mCurrentTheme != theme)
    {
        mCurrentTheme = theme;

        Preferences::instance()->setThemeType(mCurrentTheme);

        for (QObject* obj : qAsConst(mWidgetsStyledByCSSFile))
        {
            QWidget* widget = qobject_cast<QWidget*>(obj);
            if (widget)
            {
                applyStyleSheet(widget);
            }
        }

        emit themeChanged(theme);
    }
}



