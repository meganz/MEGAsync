#include "ThemeManager.h"
#include "ThemeIconManager.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>
#include <QDebug>
#include <QElapsedTimer>
#include <QDir>
#include <QMetaEnum>
#include <QtConcurrent/QtConcurrent>

#include <memory>

QMap<Preferences::Theme, QString> ThemeManager::mThemesMap = {
    {Preferences::Theme::LIGHT_THEME, QLatin1String("Light")},
    {Preferences::Theme::DARK_THEME, QLatin1String("Dark")}
};

ThemeManager::ThemeManager() :
    mCurrentStyle(Preferences::Theme::LAST),
    mThemePaths{{Preferences::Theme::LIGHT_THEME, QLatin1String("light_tokens/")},
               {Preferences::Theme::DARK_THEME, QLatin1String("dark_tokens/")}}
{
    loadStylesheets();
    setTheme(static_cast<Preferences::Theme>(Preferences::instance()->getTheme()));
}

Preferences::Theme ThemeManager::getSelectedTheme()
{
    return static_cast<Preferences::Theme>(Preferences::instance()->getTheme());
}

QString ThemeManager::getCssPath() const
{
    QString basePath;

#if defined(Q_OS_WIN)
    basePath = QLatin1String(":/themes/styles/win/semantic_tokens_");
#elif defined(Q_OS_LINUX)
    basePath = QLatin1String(":/themes/styles/linux/semantic_tokens_");
#elif defined(Q_OS_MAC)
    basePath = QLatin1String(":/themes/styles/macx/semantic_tokens_");
#else
    basePath = QLatin1String("");
#endif

    return basePath + mThemePaths.value(static_cast<Preferences::Theme>(Preferences::instance()->getTheme()), mThemePaths.value(Preferences::Theme::LIGHT_THEME));
}

ThemeManager* ThemeManager::instance()
{
    static ThemeManager manager;

    return &manager;
}


bool ThemeManager::addClassToThemeManager(QWidget* classWidget)
{
    return registerWidgetForTheming(classWidget);
}

void ThemeManager::applyStyleSheet(QWidget* parent)
{
    if (!parent)
        return;

    const QString className = QString::fromUtf8(parent->metaObject()->className());
    const QString themeKey = className % QLatin1String("_") % themeToString(getSelectedTheme());

    QSet<QString> objectNames = getObjectNamesInUICSSFile(themeKey);
    for (const QString &objectName : objectNames)
    {
        QWidget* item = (objectName == className) ? parent : parent->findChild<QWidget*>(objectName);

        if (!item)
        {
            qDebug() << "Widget not found for objectName:" << objectName;
            continue;
        }

        const QString key = themeKey % QLatin1String("_") % objectName;
        const QString currentStylesheet = item->styleSheet();
        const QString themeStylesheet = getThemeStylesheet(key); // Retrieve the stylesheet associated with the specified object
        const QString newStylesheet = currentStylesheet % QLatin1String("\n") % themeStylesheet; // Append themeStylesheet to the current stylesheet, overriding relevant parts

        item->setStyleSheet(newStylesheet);

        registerIconForTheming(item, key);
    }
}

void ThemeManager::addToStyleCache(QObject* item, QList<QObject*>& list)
{
    if(!list.contains(item))
    {
        list.append(item);
        item->connect(item, &QObject::destroyed,[item, &list](){
            list.removeOne(item);
        });
    }
}

void ThemeManager::loadStylesheets()
{
    QStringList uiCssFiles = getUiCssFiles();

    for (const QString& fileName : uiCssFiles)
    {
        for (auto it = mThemePaths.constBegin(); it != mThemePaths.constEnd(); ++it)
        {
            Preferences::Theme theme = it.key();
            const QString& themePath = it.value();

            QString fullThemeFilePath = themePath + fileName;
            QString filePath = getFilePath(fullThemeFilePath);
            QString key = fileName + QLatin1String("_") + themeToString(theme);

            // Load stylesheets corresponding to UI files
            loadStylesheetAsync(filePath, key);
        }
    }
}

QStringList ThemeManager::themesAvailable() const
{
    static QStringList themes;
    if (themes.isEmpty())
    {
        QMetaEnum metaEnum = QMetaEnum::fromType<Preferences::Theme>();
        for(int index = 0; index < static_cast<std::underlying_type<Preferences::Theme>::type>(Preferences::Theme::LAST); ++index)
        {
            Preferences::Theme theme = static_cast<Preferences::Theme>(metaEnum.value(index));
            if (theme != Preferences::Theme::LAST)
            {
                themes.append(themeToString(theme));
            }
        }
    }

    return themes;
}

QStringList ThemeManager::getUiCssFiles() const
{
    QStringList fileNames;
    const QString folderPath = getCssPath();
    QDir directory(folderPath);

    if (!directory.exists()) {
        return fileNames;
    }

    QFileInfoList files = directory.entryInfoList(QStringList() << QLatin1String("*.css"), QDir::Files);

    for (const QFileInfo& fileInfo : files) {
        const QString fileName = fileInfo.baseName();
        fileNames.append(fileName);
    }

    return fileNames;
}

bool ThemeManager::registerWidgetForTheming(QWidget* widget)
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

void ThemeManager::applyApplicationStyles(QObject* item)
{
    QString cssPath = getCssPath();
    QFile cssFile(cssPath + QLatin1String("ApplicationStyles.json"));

    if (!cssFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Couldn't open application styles file.");
        return;
    }

    QString val = QString::fromUtf8(cssFile.readAll());
    cssFile.close();

    QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());
    if (d.isNull()) {
        qWarning("Failed to create JSON doc.");
        return;
    }
    if (!d.isObject()) {
        qWarning("JSON is not an object.");
        return;
    }

    QJsonObject jsonObject = d.object();
    QJsonArray stylesArray = jsonObject.value(QLatin1String("styles")).toArray();

    QString styleString;
    for (const QJsonValue &value : stylesArray) {
        QJsonObject styleObject = value.toObject();
        QString selector = styleObject.value(QLatin1String("selector")).toString();
        QJsonObject propertiesObject = styleObject.value(QLatin1String("properties")).toObject();

        styleString += selector + QLatin1String(" {");
        for (auto prop = propertiesObject.constBegin(); prop != propertiesObject.constEnd(); ++prop) {
            styleString += prop.key() + QLatin1String(": ") + prop.value().toString() + QLatin1String("; ");
        }
        styleString.append(QLatin1String("}\n"));
    }

    item->setProperty("ORIGINAL_CSS", styleString);

    // Apply the stylesheet
    QMetaObject::invokeMethod(item, "setStyleSheet", Qt::DirectConnection, Q_ARG(QString, styleString));
    addToStyleCache(item, mWidgetsStyledByCode);
}

QString ThemeManager::themeToString(Preferences::Theme theme) const
{
    return mThemesMap.value(theme, QLatin1String("Light"));
}

void ThemeManager::parseStyleSheet(const QString& stylesheet, const QString& uiFileName)
{
    mThemeStylesheetParser.parseStyleSheet(stylesheet, uiFileName);

}

QSet<QString> ThemeManager::getObjectNamesInUICSSFile(const QString& themeKey) const
{
    return mThemeStylesheetParser.getObjectNamesInUICSSFile(themeKey);
}

QMap<QString, ThemeStylesheetParser::IconStates> ThemeManager::getIconsForObject(const QString& objectName, const QString& key) const
{
    return mThemeStylesheetParser.getIconsForObject(objectName, key);
}

void ThemeManager::registerIconForTheming(QWidget* item, const QString& key)
{
    if (!item)
    {
        return;
    }

    static ThemeIconManager& themeIconManager = ThemeIconManager::instance();

    const QString& objectName = item->objectName();
    auto iconSets = getIconsForObject(objectName, key);

    for (auto it = iconSets.cbegin(); it != iconSets.cend(); ++it)
    {
        QString selectorObjectName = it.key();
        const ThemeStylesheetParser::IconStates& iconStates = it.value();

        // Convert IconStates to IconStateInfo
        ThemeIconManager::IconStateInfo iconSet;
        iconSet.normal = iconStates.normal;

        // Currently, icons are considered for all three states. ToDO: Moving forward, indicative icons will be distinguished.
        if(!iconStates.hover.isNull())
        {
            iconSet.hover = iconStates.hover;
        }
        else
        {
            iconSet.hover = iconStates.normal;
        }

        if(!iconStates.pressed.isNull())
        {
            iconSet.pressed = iconStates.pressed;
        }
        else
        {
            iconSet.pressed = iconStates.normal;
        }

        if(objectName == selectorObjectName)
        {
            themeIconManager.registerWidget(item, iconSet);
        }

        QWidget* widget = item->findChild<QWidget*>(selectorObjectName);
        if (widget)
        {
            themeIconManager.registerWidget(widget, iconSet);
        }
    }
}

void ThemeManager::loadStylesheetAsync(const QString& filename, const QString& key)
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

void ThemeManager::onStylesheetLoaded(const QString& stylesheet, const QString& key)
{
    parseStyleSheet(stylesheet, key);
}

QString ThemeManager::getThemeStylesheet(const QString& key) const
{
    return mThemeStylesheetParser.getThemeStylesheet(key);
}

void ThemeManager::setTheme(Preferences::Theme newTheme)
{
    if (mCurrentStyle != newTheme)
    {
        mCurrentStyle = newTheme;

        Preferences::instance()->setTheme(static_cast<int>(mCurrentStyle));

        for (QObject* obj : qAsConst(mWidgetsStyledByCSSFile))
        {
            QWidget* widget = qobject_cast<QWidget*>(obj);
            if (widget)
            {
                applyStyleSheet(widget);
            }
        }

        emit themeChanged(mCurrentStyle);
    }
}

QString ThemeManager::getFilePath(const QString& themeFilePath) const
{
    QString basePath;

#if defined(Q_OS_WIN)
    basePath = QLatin1String(":/themes/styles/win/semantic_tokens_");
#elif defined(Q_OS_LINUX)
    basePath = QLatin1String(":/themes/styles/linux/semantic_tokens_");
#elif defined(Q_OS_MAC)
    basePath = QLatin1String(":/themes/styles/macx/semantic_tokens_");
#else
    basePath = QLatin1String("");
#endif
    return basePath + themeFilePath + QLatin1String(".css");
}

