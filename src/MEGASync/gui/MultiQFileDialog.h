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
    MultiQFileDialog(QWidget * parent = 0, const QString & caption = QString(),
                  const QString & directory = QString(),
                  bool multiSelect = true,
                  const QString & filter = QString() );

protected:
    QLineEdit *le;
    bool eventFilter(QObject *obj, QEvent *e);
    bool showHidden;
    bool multiSelect;

signals:

public slots:
    void accept ();
    void onSelectionChanged ( const QItemSelection & selected, const QItemSelection & deselected );
};

#endif // MULTIQFILEDIALOG_H
