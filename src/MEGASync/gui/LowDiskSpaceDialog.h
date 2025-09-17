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
    explicit LowDiskSpaceDialog(long long neededSize,
                                long long freeSize,
                                long long driveSize,
                                const QString& driveName,
                                QWidget* parent = nullptr);
    ~LowDiskSpaceDialog();

protected:
    bool event(QEvent* event) override;

private:
    static QString toString(long long bytes);
    void setupShadowEffect();

    Ui::LowDiskSpaceDialog* mUi;
    long long mneededSize;
    long long mfreeSize;
    long long mdriveSize;
    QString mdriveName;

    void updateStrings();
};

#endif // LOWDISKSPACEDIALOG_H
