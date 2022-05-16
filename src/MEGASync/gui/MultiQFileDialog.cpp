#include "MultiQFileDialog.h"
#include <QApplication>
#include <QKeyEvent>

MultiQFileDialog::MultiQFileDialog(QWidget *parent, const QString &caption, const QString &directory, bool multiSelect, const QString &filter)
    : QFileDialog(parent, caption, directory, filter),
      mShowHidden(false),
      mMultiSelect(multiSelect),
      mEnableOkButton(false)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setOption(QFileDialog::DontUseNativeDialog, false);

    if (multiSelect)
    {
        setOption(QFileDialog::DontUseNativeDialog, true);
        setFileMode(QFileDialog::Directory);

        setMimeTypeFilters(QStringList(QLatin1String("application/octet-stream")));

        mLe = findChild<QLineEdit*>(QString::fromUtf8("fileNameEdit"));

        QListView *l = findChild<QListView*>(QString::fromUtf8("listView"));
        if (l)
        {
            l->setSelectionMode(QListView::ExtendedSelection);
            if (mLe)
            {
                connect(l->selectionModel(), &QItemSelectionModel::selectionChanged,
                        this, &MultiQFileDialog::onSelectionChanged);
            }
        }

        QTreeView *t = findChild<QTreeView*>();
        if (t)
        {
            t->setSelectionMode(QAbstractItemView::ExtendedSelection);
            if (mLe)
            {
                connect(t->selectionModel(), &QItemSelectionModel::selectionChanged,
                        this, &MultiQFileDialog::onSelectionChanged);
            }
        }

        QLabel *label = findChild<QLabel*>(QString::fromUtf8("fileNameLabel"));
        if (label)
        {
            label->hide();
        }

        label = findChild<QLabel*>(QString::fromUtf8("fileTypeLabel"));
        if (label)
        {
            label->hide();
        }

        label = findChild<QLabel*>(QString::fromUtf8("lookInLabel"));
        if (label)
        {
            label->hide();
        }

        QDialogButtonBox *buttonBox = findChild<QDialogButtonBox*>(QString::fromUtf8("buttonBox"));
        if (buttonBox)
        {
            mBOpen = buttonBox->button(QDialogButtonBox::Open);
        }

        if (mBOpen)
        {
            mBOpen->setText(QCoreApplication::translate("QDialogButtonBox", "&OK"));
        }

        if (mLe)
        {
            mLe->setText(QCoreApplication::translate("ShellExtension", "Upload to MEGA"));
        }
    }

    QList<QWidget *> widgets = findChildren<QWidget *>();
    for (QList<QWidget *>::const_iterator it = widgets.cbegin(); it != widgets.cend(); ++it)
    {
       (*it)->installEventFilter(this);
    }
    installEventFilter(this);
    mHighDpiResize.init(this);
}

bool MultiQFileDialog::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(e);
        Qt::KeyboardModifiers modifiers = QApplication::queryKeyboardModifiers();
        if (modifiers.testFlag(Qt::ControlModifier) && keyEvent && keyEvent->key() == Qt::Key_H)
        {
            if (mShowHidden)
            {
                if (mMultiSelect)
                {
                    setFilter(QDir::AllDirs | QDir::AllEntries | QDir::NoDotAndDotDot);
                }
                else
                {
                    setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
                }
            }
            else
            {
                if (mMultiSelect)
                {
                    setFilter(QDir::AllDirs | QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
                }
                else
                {
                    setFilter(QDir::AllDirs | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
                }
            }
            mShowHidden = !mShowHidden;
        }
    }

    // Override the OK button enabled state to respect our own logic.
    // We return to prevent QFileDialog from overriding us.
    else if ((obj == mBOpen) && (e->type() == QEvent::EnabledChange))
    {
        bool enabled (mBOpen->isEnabled());
        bool pathExists (QFileInfo::exists(mLe->text()));
        if (!pathExists)
        {
            pathExists = true;
            const QStringList items (mLe->text().split(QString::fromUtf8("\"")));
            auto item (items.cbegin());

            while (pathExists && item != items.cend())
            {
                if (!item->trimmed().isEmpty())
                {
                    pathExists &= QFileInfo::exists(directory().absolutePath()
                                                    + QDir::separator() + *item);
                }
                item++;
            }
        }

        if (enabled && !mEnableOkButton && !pathExists)
        {
            mBOpen->setEnabled(false);
            mEnableOkButton = false;
            return true;
        }
        else if (!enabled && pathExists)
        {
            mEnableOkButton = true;
            mBOpen->setEnabled(true);
            return true;
        }
        else
        {
            return false;
        }
    }

    return QFileDialog::eventFilter(obj, e);
}

void MultiQFileDialog::accept()
{
    QStringList files = selectedFiles();
    if (files.isEmpty())
    {
        return;
    }
    emit filesSelected(files);
    QDialog::accept();
}

void MultiQFileDialog::onSelectionChanged()
{
    QString actionString = QCoreApplication::translate("ShellExtension", "Upload to MEGA");

    QStringList files = selectedFiles();
    int numFiles = 0;
    int numFolders = 0;

    QString dir (directory().absolutePath());

    // Do not select a file/folder whose name is the same as the parent
    // upon entering a directory.
    if (files.size() == 2 && (files[0] == dir || files[0] == files[1]))
    {
        files.clear();
    }

    for (auto file : qAsConst(files))
    {
        if (file != dir)
        {
            QFileInfo fi(file);
            if (fi.exists())
            {
                if (fi.isDir())
                {
                    numFolders++;
                }
                else
                {
                    numFiles++;
                }
            }
        }
    }

    if (mBOpen)
    {
        int nbSelected (numFolders + numFiles);
        mEnableOkButton = (nbSelected > 0);

        // Check that the lineEdit and the view are in sync
        if (nbSelected > 0)
        {
            int nbItemsInLineEdit (0);
            const QStringList items (mLe->text().split(QString::fromUtf8("\"")));
            for (auto item (items.cbegin()); item != items.cend(); item++)
            {
                if (!item->trimmed().isEmpty())
                {
                    nbItemsInLineEdit++;
                }
            }
            mEnableOkButton = nbItemsInLineEdit == nbSelected;
        }

        mBOpen->setEnabled(mEnableOkButton);
    }

    QString sNumFiles;
    if (numFiles == 1)
    {
        sNumFiles = QCoreApplication::translate("ShellExtension", "1 file");
    }
    else if (numFiles > 1)
    {
        sNumFiles = QCoreApplication::translate("ShellExtension", "%1 files")
                .arg(numFiles);
    }

    QString sNumFolders;
    if (numFolders == 1)
    {
        sNumFolders = QCoreApplication::translate("ShellExtension", "1 folder");
    }
    else if (numFolders > 1)
    {
        sNumFolders = QCoreApplication::translate("ShellExtension", "%1 folders")
                .arg(numFolders);
    }

    QString fullString;
    if (numFiles && numFolders)
    {
        fullString = QCoreApplication::translate("ShellExtension", "%1 (%2, %3)")
                .arg(actionString).arg(sNumFiles).arg(sNumFolders);
    }
    else if (numFiles && !numFolders)
    {
        fullString = QCoreApplication::translate("ShellExtension", "%1 (%2)")
                .arg(actionString).arg(sNumFiles);
    }
    else if (!numFiles && numFolders)
    {
        fullString = QCoreApplication::translate("ShellExtension", "%1 (%2)")
                .arg(actionString).arg(sNumFolders);
    }
    setWindowTitle(fullString);
}
