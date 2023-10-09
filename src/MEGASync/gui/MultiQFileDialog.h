#ifndef MULTIQFILEDIALOG_H
#define MULTIQFILEDIALOG_H

#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QKeyEvent>

class MultiQFileDialog : public QFileDialog
{
    Q_OBJECT
public:
    MultiQFileDialog(QWidget* parent = nullptr, const QString& caption = QString(),
                  const QString& directory = QString(),
                  bool mMultiSelect = true,
                  const QString& filter = QString());

public slots:
    void accept() override;

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    void findSelectedFilesAndFoldersCount(int& fileCount, int& folderCount);
    void updateOpenButtonEnabledStatus(int selectedItemCount);
    int findItemCountInLineEdit();
    static QString createWindowTitle(int fileCount, int folderCount);

    void onKeyPressEvent(QKeyEvent* e);
    bool onEnabledChangeEvent();
    void onHoverEnterEvent();

    QLineEdit* mLe;
    QPushButton* mBOpen;
    bool mShowHidden;
    bool mMultiSelect;
    bool mEnableOkButton;
    bool isSelectionOverLimit = false;

private slots:
    void onSelectionChanged();
};

#endif // MULTIQFILEDIALOG_H
