#ifndef SYNCSETTINGS_H
#define SYNCSETTINGS_H

#include <QWidget>

namespace Ui {
class SyncSettings;
}

class SyncSettings : public QWidget
{
    Q_OBJECT

public:
    explicit SyncSettings(QWidget *parent = nullptr);
    ~SyncSettings();

private:
    Ui::SyncSettings *ui;
};

#endif // SYNCSETTINGS_H
