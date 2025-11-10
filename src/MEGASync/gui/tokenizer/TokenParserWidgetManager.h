#ifndef THEME_WIDGET_MANAGER_H
#define THEME_WIDGET_MANAGER_H

#include <QColor>
#include <QFileDialog>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QString>
#include <QWidget>

#include <memory>

class TokenParserWidgetManager : public QObject
{
    Q_OBJECT

public:
    static std::shared_ptr<TokenParserWidgetManager> instance();

    void applyCurrentTheme();
    void registerWidgetForTheming(QWidget* dialog);
    void applyCurrentTheme(QWidget* dialog);
    void polish(QWidget* widget);
    QColor getColor(const QString& colorToken);
    QColor getColor(const QString& colorToken, const QString& colorSchema);
    static void styleQFileDialog(QPointer<QFileDialog> dialog);

private:
    using ColorTokens = QMap<QString, QString>;

    explicit TokenParserWidgetManager(QObject *parent = nullptr);
    void loadColorThemeJson();
    void loadStandardStyleSheetComponents();
    void onThemeChanged();
    void onUpdateRequested();
    void applyTheme(QWidget* widget);
    QString helpApplyTheme(QWidget* widget);
    void replaceIconColorTokens(QWidget* widget, QString& styleSheet);
    void replaceColorTokens(QString& styleSheet, const ColorTokens& colorTokens);
    void removeFrameOnDialogCombos(QWidget* widget);
    bool isTokenized(QWidget* widget);
    void tokenizeChildStyleSheets(QWidget* widget);

    QMap<QString, ColorTokens> mColorThemedTokens;
    QMap<QString, QString> mThemedStandardComponentsStyleSheet;
    QMap<QString, QMap<QString, QString>> mThemedWidgetStyleSheets;
    QSet<QWidget*> mRegisteredWidgets;
};

#endif // THEMEWIDGET_H
