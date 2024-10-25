#include "WhatsNewController.h"

#include "MegaApplication.h"
#include "Preferences.h"

#include <QCoreApplication>

namespace
{
constexpr char ROCKET_IMAGE[] = "rocket";
constexpr char MEGA_CLOUD_IMAGE[] = "mega-cloud";
constexpr char CONTROLS_IMAGE[] = "controls";
constexpr char DEVICE_CENTRE_IMAGE[] = "device-centre-update";

static constexpr int FIRST_5_X_VERSION = 50000;
static constexpr int FIRST_DEVICE_CENTRE_VERSION = 50700;
}

WhatsNewController::WhatsNewController()
{
    init();
}

QHash<int, QByteArray> WhatsNewController::roleNames() const
{
    static QHash<int, QByteArray> roles{
        {TITLE,       "title"      },
        {DESCRIPTION, "description"},
        {IMAGE,       "image"      }
    };
    return roles;
}

int WhatsNewController::rowCount(const QModelIndex& parent) const
{
    return static_cast<int>(mUpdatesModel.mUpdates.size());
}

QVariant WhatsNewController::data(const QModelIndex& index, int role) const
{
    QVariant field;
    const auto row = static_cast<size_t>(index.row());
    if (!index.isValid() || index.row() >= rowCount())
    {
        return field;
    }
    switch (role)
    {
        case WhatsNewController::TITLE:
            field = mUpdatesModel.mUpdates[row].mTitle;
            break;
        case WhatsNewController::DESCRIPTION:
            field = mUpdatesModel.mUpdates[row].mDescription;
            break;
        case WhatsNewController::IMAGE:
            field = mUpdatesModel.mUpdates[row].mImage;
            break;
    }
    return field;
}

void WhatsNewController::init()
{
    if (Preferences::lastVersionUponStartup < ::FIRST_5_X_VERSION)
    {
        mUpdatesModel.mUpdates = {
            UpdatesList::UpdateItem{
                                    QApplication::translate("WhatsNewStrings", "Better Performance"),
                                    QApplication::translate("WhatsNewStrings",
                                    "Files now sync up to 5x faster than before"),
                                    QString::fromUtf8(::ROCKET_IMAGE)       },
            UpdatesList::UpdateItem{
                                    QApplication::translate("WhatsNewStrings", "Greater control"),
                                    QApplication::translate("WhatsNewStrings",
                                    "Full visibility into sync issues and total control over "
                                        "how conflicts are resolved"),
                                    QString::fromUtf8(::CONTROLS_IMAGE)     },
            UpdatesList::UpdateItem{
                                    QApplication::translate("WhatsNewStrings", "Advanced filters"),
                                    QApplication::translate("WhatsNewStrings",
                                    "Advanced settings allow you to write your own exclusion "
                                        "rules for each of your syncs"),
                                    QString::fromUtf8(::MEGA_CLOUD_IMAGE)   },
            UpdatesList::UpdateItem{
                                    QApplication::translate("WhatsNewStrings", "New device centre"),
                                    QApplication::translate(
                    "WhatsNewStrings",                                         "New features help you track and manage your syncs and backups in one place"),
                                    QString::fromUtf8(::DEVICE_CENTRE_IMAGE)}
        };
        mUpdatesModel.mAcceptButtonText = QApplication::translate("WhatsNewStrings", "Got it");
        mUpdatesModel.mAcceptFunction = []() {};
        return;
    }
    else if (Preferences::lastVersionUponStartup < ::FIRST_DEVICE_CENTRE_VERSION)
    {
        mUpdatesModel.mUpdates = {
            UpdatesList::UpdateItem{
                                    QApplication::translate("WhatsNewStrings",
                                    "Do more with your data in MEGA’s new device centre"),
                                    QApplication::translate("WhatsNewStrings",
                                    "Easier to use, and with new features, you can track your "
                                        "syncs and backups, view real time "
                                        "statuses and manage them all in one place"),
                                    QString::fromUtf8(::DEVICE_CENTRE_IMAGE)}
        };
        mUpdatesModel.mAcceptButtonText =
            QApplication::translate("WhatsNewStrings", "Take me there");
        mUpdatesModel.mAcceptFunction = []()
        {
            MegaSyncApp->openDeviceCentre();
        };
        return;
    }
}

QString WhatsNewController::acceptButtonText()
{
    return mUpdatesModel.mAcceptButtonText;
}

void WhatsNewController::acceptButtonClicked()
{
    mUpdatesModel.mAcceptFunction();
}
