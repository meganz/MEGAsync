#ifndef THEME_WIDGET_MANAGER_H
#define THEME_WIDGET_MANAGER_H

#include "Preferences/Preferences.h"

#include <QObject>
#include <QIcon>

class TokenParserWidgetManager : public QObject
{
    Q_OBJECT

public:
    static std::shared_ptr<TokenParserWidgetManager> instance();

    void applyCurrentTheme(QWidget* widget);

private:
    using ColorTokens = QMap<QString, QString>;

    explicit TokenParserWidgetManager(QObject *parent = nullptr);
    QString themeToString(Preferences::ThemeType theme) const;
    void loadColorThemeJson();
    void onThemeChanged(Preferences::ThemeType theme);
    void onUpdateRequested();
    void applyTheme(QWidget* widget);
    bool replaceThemeTokens(QString& styleSheet, const QString& currentTheme);
    bool replaceIconColorTokens(QWidget* widget, QString& styleSheet, const ColorTokens& colorTokens);
    bool replaceColorTokens(QString& styleSheet, const ColorTokens& colorTokens);
    void removeFrameOnDialogCombos(QWidget* widget);
    void test(QWidget* widget);

    QMap<QString, ColorTokens> mColorThemedTokens;
    QWidget* mCurrentWidget;
};

#endif // THEMEWIDGET_H
