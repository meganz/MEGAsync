#ifndef RENAMETARGETFOLDERDIALOG_H
#define RENAMETARGETFOLDERDIALOG_H

#include "megaapi.h"

#include <QDialog>

#include <memory>

namespace Ui {
class RenameTargetFolderDialog;
}

class RenameTargetFolderDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit RenameTargetFolderDialog(QWidget* parent = nullptr);
        ~RenameTargetFolderDialog();

        QString getNewFolderName() const;

    private:
        Ui::RenameTargetFolderDialog* mUi;

    private slots:
        void onFolderNameFieldChanged(const QString& text);
};

#endif // RENAMETARGETFOLDERDIALOG_H
