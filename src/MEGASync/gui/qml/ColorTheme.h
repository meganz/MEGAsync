#ifndef COLORTHEME_H
#define COLORTHEME_H

#include <QObject>
#include <QQmlEngine>
#include <QTimer>

class ColorTheme : public QObject
{
    Q_OBJECT

    Q_PROPERTY (QString borderInteractive READ borderInteractive NOTIFY valueChanged)
    Q_PROPERTY (QString borderStrong READ borderStrong NOTIFY valueChanged)
    Q_PROPERTY (QString borderStrongSelected READ borderStrongSelected NOTIFY valueChanged)
    Q_PROPERTY (QString borderSubtle READ borderSubtle NOTIFY valueChanged)
    Q_PROPERTY (QString borderSubtleSelected READ borderSubtleSelected NOTIFY valueChanged)
    Q_PROPERTY (QString borderDisabled READ borderDisabled NOTIFY valueChanged)
    Q_PROPERTY (QString linkPrimary READ linkPrimary NOTIFY valueChanged)
    Q_PROPERTY (QString linkInverse READ linkInverse NOTIFY valueChanged)
    Q_PROPERTY (QString linkVisited READ linkVisited NOTIFY valueChanged)
    Q_PROPERTY (QString buttonBrand READ buttonBrand NOTIFY valueChanged)
    Q_PROPERTY (QString buttonBrandHover READ buttonBrandHover NOTIFY valueChanged)
    Q_PROPERTY (QString buttonBrandPressed READ buttonBrandPressed NOTIFY valueChanged)
    Q_PROPERTY (QString buttonPrimary READ buttonPrimary NOTIFY valueChanged)
    Q_PROPERTY (QString buttonPrimaryHover READ buttonPrimaryHover NOTIFY valueChanged)
    Q_PROPERTY (QString buttonPrimaryPressed READ buttonPrimaryPressed NOTIFY valueChanged)
    Q_PROPERTY (QString buttonOutline READ buttonOutline NOTIFY valueChanged)
    Q_PROPERTY (QString buttonOutlineHover READ buttonOutlineHover NOTIFY valueChanged)
    Q_PROPERTY (QString buttonOutlineBackgroundHover READ buttonOutlineBackgroundHover NOTIFY valueChanged)
    Q_PROPERTY (QString buttonOutlinePressed READ buttonOutlinePressed NOTIFY valueChanged)
    Q_PROPERTY (QString buttonSecondary READ buttonSecondary NOTIFY valueChanged)
    Q_PROPERTY (QString buttonSecondaryHover READ buttonSecondaryHover NOTIFY valueChanged)
    Q_PROPERTY (QString buttonSecondaryPressed READ buttonSecondaryPressed NOTIFY valueChanged)
    Q_PROPERTY (QString buttonError READ buttonError NOTIFY valueChanged)
    Q_PROPERTY (QString buttonErrorHover READ buttonErrorHover NOTIFY valueChanged)
    Q_PROPERTY (QString buttonErrorPressed READ buttonErrorPressed NOTIFY valueChanged)
    Q_PROPERTY (QString buttonDisabled READ buttonDisabled NOTIFY valueChanged)
    Q_PROPERTY (QString iconButton READ iconButton NOTIFY valueChanged)
    Q_PROPERTY (QString iconButtonHover READ iconButtonHover NOTIFY valueChanged)
    Q_PROPERTY (QString iconButtonPressed READ iconButtonPressed NOTIFY valueChanged)
    Q_PROPERTY (QString iconButtonPressedBackground READ iconButtonPressedBackground NOTIFY valueChanged)
    Q_PROPERTY (QString iconButtonDisabled READ iconButtonDisabled NOTIFY valueChanged)
    Q_PROPERTY (QString focus READ focus NOTIFY valueChanged)
    Q_PROPERTY (QString pageBackground READ pageBackground NOTIFY valueChanged)
    Q_PROPERTY (QString surface1 READ surface1 NOTIFY valueChanged)
    Q_PROPERTY (QString surface2 READ surface2 NOTIFY valueChanged)
    Q_PROPERTY (QString surface3 READ surface3 NOTIFY valueChanged)
    Q_PROPERTY (QString backgroundInverse READ backgroundInverse NOTIFY valueChanged)
    Q_PROPERTY (QString textPrimary READ textPrimary NOTIFY valueChanged)
    Q_PROPERTY (QString textSecondary READ textSecondary NOTIFY valueChanged)
    Q_PROPERTY (QString textAccent READ textAccent NOTIFY valueChanged)
    Q_PROPERTY (QString textPlaceholder READ textPlaceholder NOTIFY valueChanged)
    Q_PROPERTY (QString textInverseAccent READ textInverseAccent NOTIFY valueChanged)
    Q_PROPERTY (QString textOnColor READ textOnColor NOTIFY valueChanged)
    Q_PROPERTY (QString textOnColorDisabled READ textOnColorDisabled NOTIFY valueChanged)
    Q_PROPERTY (QString textError READ textError NOTIFY valueChanged)
    Q_PROPERTY (QString textSuccess READ textSuccess NOTIFY valueChanged)
    Q_PROPERTY (QString textInfo READ textInfo NOTIFY valueChanged)
    Q_PROPERTY (QString textWarning READ textWarning NOTIFY valueChanged)
    Q_PROPERTY (QString textInverse READ textInverse NOTIFY valueChanged)
    Q_PROPERTY (QString textDisabled READ textDisabled NOTIFY valueChanged)
    Q_PROPERTY (QString iconPrimary READ iconPrimary NOTIFY valueChanged)
    Q_PROPERTY (QString iconSecondary READ iconSecondary NOTIFY valueChanged)
    Q_PROPERTY (QString iconAccent READ iconAccent NOTIFY valueChanged)
    Q_PROPERTY (QString iconInverseAccent READ iconInverseAccent NOTIFY valueChanged)
    Q_PROPERTY (QString iconOnColor READ iconOnColor NOTIFY valueChanged)
    Q_PROPERTY (QString iconOnColorDisabled READ iconOnColorDisabled NOTIFY valueChanged)
    Q_PROPERTY (QString iconInverse READ iconInverse NOTIFY valueChanged)
    Q_PROPERTY (QString iconPlaceholder READ iconPlaceholder NOTIFY valueChanged)
    Q_PROPERTY (QString iconDisabled READ iconDisabled NOTIFY valueChanged)
    Q_PROPERTY (QString supportSuccess READ supportSuccess NOTIFY valueChanged)
    Q_PROPERTY (QString supportWarning READ supportWarning NOTIFY valueChanged)
    Q_PROPERTY (QString supportError READ supportError NOTIFY valueChanged)
    Q_PROPERTY (QString supportInfo READ supportInfo NOTIFY valueChanged)
    Q_PROPERTY (QString selectionControl READ selectionControl NOTIFY valueChanged)
    Q_PROPERTY (QString notificationSuccess READ notificationSuccess NOTIFY valueChanged)
    Q_PROPERTY (QString notificationWarning READ notificationWarning NOTIFY valueChanged)
    Q_PROPERTY (QString notificationError READ notificationError NOTIFY valueChanged)
    Q_PROPERTY (QString notificationInfo READ notificationInfo NOTIFY valueChanged)
    Q_PROPERTY (QString interactive READ interactive NOTIFY valueChanged)
    Q_PROPERTY (QString indicatorBackground READ indicatorBackground NOTIFY valueChanged)
    Q_PROPERTY (QString indicatorPink READ indicatorPink NOTIFY valueChanged)
    Q_PROPERTY (QString indicatorYellow READ indicatorYellow NOTIFY valueChanged)
    Q_PROPERTY (QString indicatorGreen READ indicatorGreen NOTIFY valueChanged)
    Q_PROPERTY (QString indicatorBlue READ indicatorBlue NOTIFY valueChanged)
    Q_PROPERTY (QString indicatorIndigo READ indicatorIndigo NOTIFY valueChanged)
    Q_PROPERTY (QString indicatorMagenta READ indicatorMagenta NOTIFY valueChanged)
    Q_PROPERTY (QString indicatorOrange READ indicatorOrange NOTIFY valueChanged)
    Q_PROPERTY (QString toastBackground READ toastBackground NOTIFY valueChanged)
    Q_PROPERTY (QString divider READ divider NOTIFY valueChanged)
    Q_PROPERTY (QString gradientRedTop READ gradientRedTop NOTIFY valueChanged)
    Q_PROPERTY (QString gradientRedBottom READ gradientRedBottom NOTIFY valueChanged)
    Q_PROPERTY (QString gradientGreyTop READ gradientGreyTop NOTIFY valueChanged)
    Q_PROPERTY (QString gradientGreyBottom READ gradientGreyBottom NOTIFY valueChanged)
    Q_PROPERTY (QString gradientPinkTop READ gradientPinkTop NOTIFY valueChanged)
    Q_PROPERTY (QString gradientPinkBottom READ gradientPinkBottom NOTIFY valueChanged)
    Q_PROPERTY (QString gradientYellowTop READ gradientYellowTop NOTIFY valueChanged)
    Q_PROPERTY (QString gradientYellowBottom READ gradientYellowBottom NOTIFY valueChanged)
    Q_PROPERTY (QString gradientGreenTop READ gradientGreenTop NOTIFY valueChanged)
    Q_PROPERTY (QString gradientGreenBottom READ gradientGreenBottom NOTIFY valueChanged)
    Q_PROPERTY (QString gradientIndigoTop READ gradientIndigoTop NOTIFY valueChanged)
    Q_PROPERTY (QString gradientIndigoBottom READ gradientIndigoBottom NOTIFY valueChanged)
    Q_PROPERTY (QString gradientMagentaTop READ gradientMagentaTop NOTIFY valueChanged)
    Q_PROPERTY (QString gradientMagentaBottom READ gradientMagentaBottom NOTIFY valueChanged)
    Q_PROPERTY (QString gradientOrangeTop READ gradientOrangeTop NOTIFY valueChanged)
    Q_PROPERTY (QString gradientOrangeBottom READ gradientOrangeBottom NOTIFY valueChanged)
    Q_PROPERTY (QString gradientBlueTop READ gradientBlueTop NOTIFY valueChanged)
    Q_PROPERTY (QString gradientBlueBottom READ gradientBlueBottom NOTIFY valueChanged)
    Q_PROPERTY (QString gradientContrastTop READ gradientContrastTop NOTIFY valueChanged)
    Q_PROPERTY (QString gradientContrastBottom READ gradientContrastBottom NOTIFY valueChanged)

public:
    explicit ColorTheme(QQmlEngine* engine, QObject *parent = nullptr);
    ~ColorTheme();

    QString borderInteractive();
    QString borderStrong();
    QString borderStrongSelected();
    QString borderSubtle();
    QString borderSubtleSelected();
    QString borderDisabled();
    QString linkPrimary();
    QString linkInverse();
    QString linkVisited();
    QString buttonBrand();
    QString buttonBrandHover();
    QString buttonBrandPressed();
    QString buttonPrimary();
    QString buttonPrimaryHover();
    QString buttonPrimaryPressed();
    QString buttonOutline();
    QString buttonOutlineHover();
    QString buttonOutlineBackgroundHover();
    QString buttonOutlinePressed();
    QString buttonSecondary();
    QString buttonSecondaryHover();
    QString buttonSecondaryPressed();
    QString buttonError();
    QString buttonErrorHover();
    QString buttonErrorPressed();
    QString buttonDisabled();
    QString iconButton();
    QString iconButtonHover();
    QString iconButtonPressed();
    QString iconButtonPressedBackground();
    QString iconButtonDisabled();
    QString focus();
    QString pageBackground();
    QString surface1();
    QString surface2();
    QString surface3();
    QString backgroundInverse();
    QString textPrimary();
    QString textSecondary();
    QString textAccent();
    QString textPlaceholder();
    QString textInverseAccent();
    QString textOnColor();
    QString textOnColorDisabled();
    QString textError();
    QString textSuccess();
    QString textInfo();
    QString textWarning();
    QString textInverse();
    QString textDisabled();
    QString iconPrimary();
    QString iconSecondary();
    QString iconAccent();
    QString iconInverseAccent();
    QString iconOnColor();
    QString iconOnColorDisabled();
    QString iconInverse();
    QString iconPlaceholder();
    QString iconDisabled();
    QString supportSuccess();
    QString supportWarning();
    QString supportError();
    QString supportInfo();
    QString selectionControl();
    QString notificationSuccess();
    QString notificationWarning();
    QString notificationError();
    QString notificationInfo();
    QString interactive();
    QString indicatorBackground();
    QString indicatorPink();
    QString indicatorYellow();
    QString indicatorGreen();
    QString indicatorBlue();
    QString indicatorIndigo();
    QString indicatorMagenta();
    QString indicatorOrange();
    QString toastBackground();
    QString divider();
    QString gradientRedTop();
    QString gradientRedBottom();
    QString gradientGreyTop();
    QString gradientGreyBottom();
    QString gradientPinkTop();
    QString gradientPinkBottom();
    QString gradientYellowTop();
    QString gradientYellowBottom();
    QString gradientGreenTop();
    QString gradientGreenBottom();
    QString gradientIndigoTop();
    QString gradientIndigoBottom();
    QString gradientMagentaTop();
    QString gradientMagentaBottom();
    QString gradientOrangeTop();
    QString gradientOrangeBottom();
    QString gradientBlueTop();
    QString gradientBlueBottom();
    QString gradientContrastTop();
    QString gradientContrastBottom();

public slots:
    void onThemeChanged(QString theme);

signals:
    void valueChanged();

private:
    QString mCurrentTheme;
    QQmlEngine* mEngine;
    QStringList mThemes;
    QMap<QString, QObject*> mThemesMap;

    void init();
    void loadThemes();
    QString getValue(const char* tokenId);
};


#endif // COLORTHEME_H
