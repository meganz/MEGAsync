#ifndef MULTIQFILEDIALOG_H
#define MULTIQFILEDIALOG_H

#include <QFileDialog>
#include <QListView>
#include <QTreeView>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include "HighDpiResize.h"

class MultiQFileDialog : public QFileDialog
{
    Q_OBJECT
public:
    MultiQFileDialog(QWidget* parent = 0, const QString& caption = QString(),
                  const QString& directory = QString(),
                  bool mMultiSelect = true,
                  const QString& filter = QString());

protected:
    QLineEdit* mLe;
    QPushButton* mBOpen;
    bool mShowHidden;
    bool mMultiSelect;
    bool mEnableOkButton;
    HighDpiResize mHighDpiResize;

    bool eventFilter(QObject* obj, QEvent* e);

public slots:
    void accept();

private slots:
    void onSelectionChanged();
};

#endif // MULTIQFILEDIALOG_H
