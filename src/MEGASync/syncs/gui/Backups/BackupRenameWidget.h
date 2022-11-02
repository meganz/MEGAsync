#ifndef BACKUPRENAMEWIDGET_H
#define BACKUPRENAMEWIDGET_H

#include <QFrame>

namespace Ui {
class BackupRenameWidget;
}

class BackupRenameWidget : public QFrame
{
    Q_OBJECT

public:
    explicit BackupRenameWidget(const QString& path, int number, QWidget *parent = nullptr);
    ~BackupRenameWidget();

    QString getNewName(QStringList brotherWdgNames);
    QString getNewNameRaw();

    QString getPath();

private slots:
    void openLocalPath(QString link);

private:
    Ui::BackupRenameWidget *ui;
    QString mPath;
};

#endif // BACKUPRENAMEWIDGET_H
