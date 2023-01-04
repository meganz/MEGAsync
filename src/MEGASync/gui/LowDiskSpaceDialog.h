#ifndef LOWDISKSPACEDIALOG_H
#define LOWDISKSPACEDIALOG_H

#include "HighDpiResize.h"

#include <QDialog>

namespace Ui {
class LowDiskSpaceDialog;
}

class LowDiskSpaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LowDiskSpaceDialog(qint64 neededSize, qint64 freeSize,
                                qint64 driveSize, const QString& driveName,
                                QWidget *parent = nullptr);
    ~LowDiskSpaceDialog();

private:
    static QString toString(qint64 bytes);
    static QPixmap getHardDrivePixmap();
    static QString getMacButtonStyle(const QString& backgroundColor,
                                     const QColor& referenceColor,
                                    const QColor& textColor);
    static QString getWinButtonStyle(const QColor &backgroundColor,
                                     const QColor& textColor);
    static QString getButtonStyle(const QString& backgroundColor, const QColor &referenceColor,
                                  const QColor& textColor, int horizontalPadding, int verticalPadding,
                                  int borderRadius);
    static QString getStylePaddings(int horizontal, int vertical);
    static QString getStyleBorders(int radius);
    static QString getStyleButtonStates(const QColor& referenceColor);
    void setupUiStyle();


    Ui::LowDiskSpaceDialog *ui;

    HighDpiResize mHighDpiResize;
};

#endif // LOWDISKSPACEDIALOG_H
