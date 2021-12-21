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
        enum FolderType
        {
            TYPE_MYBACKUPS = 0,
            TYPE_BACKUP_FOLDER = 1,
        };

        explicit RenameTargetFolderDialog(const QString& syncName, FolderType type = TYPE_MYBACKUPS,
                                          std::shared_ptr<mega::MegaNode> existingFolderNode = std::shared_ptr<mega::MegaNode>(),
                                          QWidget* parent = nullptr);
        ~RenameTargetFolderDialog();

        QString getNewFolderName() const;

    private:
        Ui::RenameTargetFolderDialog* mUi;
        QString mSyncName;

    private slots:
        void onFolderNameFieldChanged(const QString& text);
};

#endif // RENAMETARGETFOLDERDIALOG_H
