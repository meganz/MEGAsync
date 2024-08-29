#ifndef LOWDISKSPACEDIALOG_H
#define LOWDISKSPACEDIALOG_H

#include <QDialog>
#include <QFileIconProvider>
#include "drivedata.h"

namespace Ui {
class LowDiskSpaceDialog;
}

class LowDiskSpaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LowDiskSpaceDialog(long neededSize,
        long long freeSize,
        long long driveSize,
        const DriveDisplayData& driveDisplayData,
        QWidget* parent = nullptr);
    ~LowDiskSpaceDialog();

private:
    static QString toString(long long bytes);
    void setupShadowEffect();

    Ui::LowDiskSpaceDialog *ui;
};

#endif // LOWDISKSPACEDIALOG_H
