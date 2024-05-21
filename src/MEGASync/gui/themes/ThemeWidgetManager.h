#ifndef THEME_WIDGET_MANAGER_H
#define THEME_WIDGET_MANAGER_H

#include "Preferences/Preferences.h"

#include <QObject>

class ThemeWidgetManager : public QObject
{
    Q_OBJECT

public:
    static std::shared_ptr<ThemeWidgetManager> instance();

    void applyCurrentTheme(QWidget* widget);

private:
    using ColorTokens = QMap<QString, QString>;

    explicit ThemeWidgetManager(QObject *parent = nullptr);
    QString themeToString(Preferences::ThemeType theme) const;
    void loadColorThemeJson();
    void onThemeChanged(Preferences::ThemeType theme);
    void applyTheme(QWidget* widget);
    void changePixmapColor(QPixmap& pixmap, QColor toColor);
    void changeImageColor(QWidget* widget, const QString& mode, const QString& state, const ColorTokens& colorTokens, const QString& targetElementId, const QString& targetElementProperty, const QString& tokenId);

    QMap<QString, ColorTokens> mColorThemedTokens;
    QWidget* mCurrentWidget;
};

#endif // THEMEWIDGET_H
