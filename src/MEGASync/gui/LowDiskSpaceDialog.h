#ifndef LOWDISKSPACEDIALOG_H
#define LOWDISKSPACEDIALOG_H

#include "drivedata.h"

#include <QDialog>
#include <QFileIconProvider>

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
                                const QString& driveName,
                                QWidget* parent = nullptr);
    ~LowDiskSpaceDialog();

private:
    static QString toString(long long bytes);
    void setupShadowEffect();

    Ui::LowDiskSpaceDialog *ui;
};

#endif // LOWDISKSPACEDIALOG_H
