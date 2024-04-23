#include "ThemeWidget.h"

#include "themes/ThemeManager.h"

#include <QDir>
#include <QWidget>
#include <QtConcurrent/QtConcurrent>

const QMap<Preferences::ThemeType, QString> ThemeWidget::mThemePaths = {
    {Preferences::ThemeType::LIGHT_THEME,  QObject::tr("light/")},
    {Preferences::ThemeType::DARK_THEME,  QObject::tr("dark/")}
};

ThemeWidget::ThemeWidget(QObject *parent)
    : QObject{parent}
{
    connect(ThemeManager::instance(), &ThemeManager::themeChanged, this, &ThemeWidget::onThemeChanged);
    loadStylesheets();
}

bool ThemeWidget::registerWidgetForTheming(QWidget *widget)
{
    static QSet<QString> cssFiles = [this]()
    {
        QStringList fileNames = getCSSFiles();
        return QSet<QString>(fileNames.begin(), fileNames.end());
    }();

    if (mWidgetsStyledByCSSFile.contains(widget))
    {
        return true;
    }

    // If the widget's class is among the CSS files add it to the cache.
    const QString className = QString::fromUtf8(widget->metaObject()->className());
    if (cssFiles.contains(className))
    {
        addToStyleCache(widget);
        return true;
    }

    return false;
}

void ThemeWidget::loadStylesheets()
{
    QStringList cssFiles = getCSSFiles();

    for (const QString& fileName : cssFiles)
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

QStringList ThemeWidget::getCSSFiles() const
{
    QStringList fileNames;
    QDir directory(getCSSPath());

    if (!directory.exists())
    {
        return fileNames;
    }

    QFileInfoList files = directory.entryInfoList(QStringList() << QLatin1String("*.css"), QDir::Files);

    for (const QFileInfo& fileInfo : files)
    {
        fileNames.append(fileInfo.baseName());
    }

    return fileNames;
}

QString ThemeWidget::getCSSPath() const
{
    QString basePath;

#if defined(Q_OS_WIN)
    basePath = QLatin1String(":/themes/win/");
#elif defined(Q_OS_LINUX)
    basePath = QLatin1String(":/themes/linux/");
#elif defined(Q_OS_MAC)
    basePath = QLatin1String(":/themes/macx/");
#else
    basePath = QLatin1String("");
#endif

    return basePath + mThemePaths.value(Preferences::instance()->getThemeType(), mThemePaths.value(Preferences::ThemeType::LIGHT_THEME));

}

QString ThemeWidget::getFilePath(const QString &themeFilePath) const
{
    QString basePath;

#if defined(Q_OS_WIN)
    basePath = QLatin1String(":/themes/win/");
#elif defined(Q_OS_LINUX)
    basePath = QLatin1String(":/themes/linux/");
#elif defined(Q_OS_MAC)
    basePath = QLatin1String(":/themes/macx/");
#else
    basePath = QLatin1String("");
#endif
    return basePath + themeFilePath + QLatin1String(".css");
}

QString ThemeWidget::themeToString(Preferences::ThemeType theme) const
{
    return mThemePaths.value(theme, QLatin1String("light"));
}

void ThemeWidget::loadStylesheetAsync(const QString &filename, const QString &key)
{
    QtConcurrent::run([=]() {
        QFile file(filename);
        if (file.open(QFile::ReadOnly | QFile::Text))
        {
            QString stylesheet = QString::fromUtf8(file.readAll());
            QMetaObject::invokeMethod(this, [=]() {
                    parseStyleSheet(stylesheet, key);
                }, Qt::QueuedConnection);
        }
    });
}

void ThemeWidget::parseStyleSheet(const QString &stylesheet, const QString &uiFileName)
{
    mThemeStylesheetParser.parseStyleSheet(stylesheet, uiFileName);
}

void ThemeWidget::addToStyleCache(QObject *item)
{
    if(!mWidgetsStyledByCSSFile.contains(item))
    {
        mWidgetsStyledByCSSFile.append(item);
        item->connect(item, &QObject::destroyed,this, [this, item](){
            mWidgetsStyledByCSSFile.removeOne(item);
        });
    }
}

void ThemeWidget::applyStyleSheet(QWidget *widget)
{
    if (!widget)
    {
        return;
    }

    const QString className = QString::fromUtf8(widget->metaObject()->className());
    const QString widgetThemeKey = className % QLatin1String("_") % themeToString(ThemeManager::instance()->getSelectedTheme());

    QSet<QString> objectNames = getObjectNamesInCSSFile(widgetThemeKey);
    for (const QString &objectName : objectNames)
    {
        QWidget* item = (objectName == className) ? widget : widget->findChild<QWidget*>(objectName);

        if (!item)
        {
            continue;
        }

        const QString key = widgetThemeKey % QLatin1String("_") % objectName;
        const QString currentStylesheet = item->styleSheet();
        const QString themeStylesheet = getThemeStylesheet(key); // Retrieve the stylesheet associated with the specified object
        const QString newStylesheet = currentStylesheet % QLatin1String("\n") % themeStylesheet; // Append themeStylesheet to the current stylesheet, overriding relevant parts

        item->setStyleSheet(newStylesheet);
    }
}

QSet<QString> ThemeWidget::getObjectNamesInCSSFile(const QString &widgetThemeKey) const
{
    return mThemeStylesheetParser.getObjectNamesInCSSFile(widgetThemeKey);
}

QString ThemeWidget::getThemeStylesheet(const QString &key) const
{
    return mThemeStylesheetParser.getThemeStylesheet(key);
}

void ThemeWidget::onThemeChanged(Preferences::ThemeType theme)
{
    Q_UNUSED(theme)

    for (QObject* obj : qAsConst(mWidgetsStyledByCSSFile))
    {
        QWidget* widget = qobject_cast<QWidget*>(obj);
        if (widget)
        {
            applyStyleSheet(widget);
        }
    }
}



