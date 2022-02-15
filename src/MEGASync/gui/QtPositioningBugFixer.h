#ifndef QTPOSITIONINGBUGFIXER_H
#define QTPOSITIONINGBUGFIXER_H

#include <QDialog>

/**
 * @brief Workaround for bug SNC-2285
 * The real bug is in QT, this is only a workaround
 * to avoid triggering the bug.
 */
class QtPositioningBugFixer : public QObject
{
    Q_OBJECT

public:
    QtPositioningBugFixer(QDialog* _dialog);
    ~QtPositioningBugFixer() = default;

    void onStartMove();
    void onEndMove();

private:
    QDialog* dialogToFix;
    Qt::WindowFlags originalFlags;
};

#endif // QTPOSITIONINGBUGFIXER_H
