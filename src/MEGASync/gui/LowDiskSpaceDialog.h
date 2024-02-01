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
    explicit LowDiskSpaceDialog(qint64 neededSize, qint64 freeSize, qint64 driveSize,
                                const DriveDisplayData& driveDisplayData,
                                QWidget *parent = nullptr);
    ~LowDiskSpaceDialog();

private:
    static QString toString(qint64 bytes);
    void setupShadowEffect();

    Ui::LowDiskSpaceDialog *ui;
};

#endif // LOWDISKSPACEDIALOG_H
