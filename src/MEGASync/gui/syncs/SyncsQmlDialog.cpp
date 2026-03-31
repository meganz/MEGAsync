#include "SyncsQmlDialog.h"

#include "DialogOpener.h"
#include "SettingsDialog.h"
#include "SyncInfo.h"

#include <QEvent>
#include <QScreen>
#include <QString>

namespace
{
constexpr double CENTERING_FACTOR = 0.5;
}

void SyncsQmlDialog::raise()
{
    setFlags((flags() & ~Qt::Dialog) | Qt::Window | Qt::WindowTitleHint |
             Qt::WindowCloseButtonHint);
    setModality(Qt::NonModal);

    const int targetWidth = qMax(width(), minimumWidth());
    const int targetHeight = qMax(height(), minimumHeight());
    const auto& geometry = screen()->geometry();
    const int xPos = geometry.x() +
                     static_cast<int>(geometry.width() * CENTERING_FACTOR -
                                      targetWidth * CENTERING_FACTOR);
    const int yPos = geometry.y() +
                     static_cast<int>(geometry.height() * CENTERING_FACTOR -
                                      targetHeight * CENTERING_FACTOR);

    setMinimumWidth(targetWidth);
    setMinimumHeight(targetHeight);
    setGeometry(xPos, yPos, targetWidth, targetHeight);
    setVisible(true);
    showNormal();
    resize(targetWidth, targetHeight);

    requestActivate();
    QmlDialog::raise();
}

bool SyncsQmlDialog::event(QEvent* event)
{
    if (event->type() == QEvent::Close || event->type() == QEvent::Show ||
        event->type() == QEvent::Hide)
    {
        if (auto dialog = DialogOpener::findDialog<SettingsDialog>())
        {
            dialog->getDialog()->setSyncAddButtonEnabled(
                event->type() != QEvent::Show,
                isBackup() ? SettingsDialog::Tabs::BACKUP_TAB : SettingsDialog::Tabs::SYNCS_TAB);
        }

        if (!isBackup() && event->type() == QEvent::Close)
        {
            emit MegaSyncApp->syncsDialogClosed();
        }
    }

    return QmlDialog::event(event);
}

bool SyncsQmlDialog::isBackup() const
{
    return mIsBackup;
}

void SyncsQmlDialog::setIsBackup(bool newIsBackup)
{
    mIsBackup = newIsBackup;
}
