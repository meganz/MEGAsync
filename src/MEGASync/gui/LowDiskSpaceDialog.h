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
    void setupShadowEffect();

    Ui::LowDiskSpaceDialog *ui;
    HighDpiResize mHighDpiResize;
};

#endif // LOWDISKSPACEDIALOG_H
