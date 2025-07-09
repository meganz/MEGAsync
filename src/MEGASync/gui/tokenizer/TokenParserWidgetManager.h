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

    void applyCurrentTheme();
    void applyCurrentTheme(QWidget* dialog);
    void registerWidgetForTheming(QWidget* dialog);
    QColor getColor(const QString& colorToken, const QString& defaultValue);

private:
    using ColorTokens = QMap<QString, QString>;

    explicit TokenParserWidgetManager(QObject *parent = nullptr);
    void loadColorThemeJson();
    void loadStandardStyleSheetComponents();
    void onThemeChanged(Preferences::ThemeType theme);
    void onUpdateRequested();
    void applyTheme(QWidget* widget, bool isSubWidget = false);
    void replaceThemeTokens(QString& styleSheet, const QString& currentTheme);
    void replaceIconColorTokens(QWidget* widget,
                                QString& styleSheet,
                                const ColorTokens& colorTokens);
    void replaceColorTokens(QString& styleSheet, const ColorTokens& colorTokens);
    void removeFrameOnDialogCombos(QWidget* widget);
    bool isTokenized(QWidget* widget);
    bool isRoot(QWidget* widget);
    void tokenizeChildStyleSheets(QWidget* widget, const QString& styleSheet);

    QMap<QString, ColorTokens> mColorThemedTokens;
    QMap<QString, QString> mThemedStandardComponentsStyleSheet;
    QMap<QString, QString> mWidgetsStyleSheets;
    QSet<QWidget*> mRegisteredWidgets;
};

#endif // THEMEWIDGET_H
