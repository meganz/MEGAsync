#ifndef MULTIQFILEDIALOG_H
#define MULTIQFILEDIALOG_H

#include <QFileDialog>
#include <QListView>
#include <QTreeView>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

class MultiQFileDialog : public QFileDialog
{
    Q_OBJECT
public:
    MultiQFileDialog(QWidget* parent = 0, const QString& caption = QString(),
                  const QString& directory = QString(),
                  bool mMultiSelect = true,
                  const QString& filter = QString());

public slots:
    void accept();

protected:
    QLineEdit* mLe;
    QPushButton* mBOpen;
    bool mShowHidden;
    bool mMultiSelect;
    bool mEnableOkButton;

    bool eventFilter(QObject* obj, QEvent* e);

private:
    void findSelectedFilesAndFoldersCount(int& fileCount, int& folderCount);
    void updateOpenButtonEnabledStatus(int selectedItemCount);
    int findItemCountInLineEdit();
    static QString createWindowTitle(int fileCount, int folderCount);

    void onKeyPressEvent(QKeyEvent* e);
    bool onEnabledChangeEvent();
    void onHoverEnterEvent();

    bool isSelectionOverLimit = false;

private slots:
    void onSelectionChanged();
};

#endif // MULTIQFILEDIALOG_H
