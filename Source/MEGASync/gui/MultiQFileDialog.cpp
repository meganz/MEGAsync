#include "MultiQFileDialog.h"
#include <QCoreApplication>

MultiQFileDialog::MultiQFileDialog(QWidget *parent, const QString &caption, const QString &directory, const QString &filter)
    : QFileDialog(parent, caption, directory, filter)
{
    setOption(QFileDialog::DontUseNativeDialog, true);
    le = findChild<QLineEdit*>(QString::fromUtf8("fileNameEdit"));

    QListView *l = findChild<QListView*>(QString::fromUtf8("listView"));
    if (l)
    {
        l->setSelectionMode(QListView::ExtendedSelection);
        if(le)
            connect(l->selectionModel(), SIGNAL(selectionChanged ( const QItemSelection &, const QItemSelection & )),
                this, SLOT(onSelectionChanged ( const QItemSelection &, const QItemSelection & )));
    }

    QTreeView *t = findChild<QTreeView*>();
    if (t)
    {
        t->setSelectionMode(QAbstractItemView::ExtendedSelection);
        if(le)
            connect(t->selectionModel(), SIGNAL(selectionChanged ( const QItemSelection &, const QItemSelection & )),
                this, SLOT(onSelectionChanged ( const QItemSelection &, const QItemSelection & )));
    }

    QLabel *label = findChild<QLabel*>(QString::fromUtf8("fileNameLabel"));
    if(label) label->hide();
    label = findChild<QLabel*>(QString::fromUtf8("fileTypeLabel"));
    if(label) label->hide();
    label = findChild<QLabel*>(QString::fromUtf8("lookInLabel"));
    if(label) label->hide();

    QDialogButtonBox *buttonBox = findChild<QDialogButtonBox*>(QString::fromUtf8("buttonBox"));
    if(buttonBox) buttonBox->button(QDialogButtonBox::Open)->setText(QCoreApplication::translate("QDialogButtonBox", "&OK"));

    setFileMode(QFileDialog::ExistingFiles);
    le->setText(QCoreApplication::translate("ShellExtension", "Upload to MEGA"));
}

void MultiQFileDialog::accept()
{
    QStringList files = selectedFiles();
    if (files.isEmpty())
        return;
    emit filesSelected(files);
    QDialog::accept();
}

void MultiQFileDialog::onSelectionChanged(const QItemSelection &, const QItemSelection &)
{
    QString actionString = QCoreApplication::translate("ShellExtension", "Upload to MEGA");

    QStringList files = selectedFiles();
    int numFiles = 0;
    int numFolders = 0;
    for(int i=0; i<files.size(); i++)
    {
        if(files[i] != directory().absolutePath())
        {
            QFileInfo fi(files[i]);
            if(fi.exists())
            {
                if(fi.isDir()) numFolders++;
                else numFiles++;
            }
        }
    }

    if (!numFiles && !numFolders)
    {
        le->setText(actionString);
        return;
    }

    QString sNumFiles;
    if(numFiles == 1) sNumFiles = QCoreApplication::translate("ShellExtension", "1 file");
    else if(numFiles > 1) sNumFiles = QCoreApplication::translate("ShellExtension", "%1 files").arg(numFiles);

    QString sNumFolders;
    if(numFolders == 1) sNumFolders = QCoreApplication::translate("ShellExtension", "1 folder");
    else if(numFolders > 1) sNumFolders = QCoreApplication::translate("ShellExtension", "%1 folders").arg(numFolders);

    QString fullString;
    if(numFiles && numFolders) fullString = QCoreApplication::translate("ShellExtension", "%1 (%2, %3)").arg(actionString).arg(sNumFiles).arg(sNumFolders);
    else if(numFiles && !numFolders) fullString = QCoreApplication::translate("ShellExtension", "%1 (%2)").arg(actionString).arg(sNumFiles);
    else if(!numFiles && numFolders) fullString = QCoreApplication::translate("ShellExtension", "%1 (%2)").arg(actionString).arg(sNumFolders);

    le->setText(fullString);
}

