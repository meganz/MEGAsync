#ifndef MEGAUPDATER_H
#define MEGAUPDATER_H

#include <QApplication>

class MegaUpdater : public QApplication
{
    Q_OBJECT
public:
    explicit MegaUpdater(int &argc, char **argv);

signals:

public slots:

};

#endif // MEGAUPDATER_H
