#include "WhatsNewController.h"

#include "MegaApplication.h"

#include <QCoreApplication>

namespace
{
constexpr char ROCKET_IMAGE[] = "rocket";
constexpr char MEGA_CLOUD_IMAGE[] = "mega-cloud";
constexpr char CONTROLS_IMAGE[] = "controls";
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
    mUpdatesModel.mUpdates = {
        UpdatesList::UpdateItem{
                                QApplication::translate("WhatsNewStrings", "Better Performance"),
                                QApplication::translate("WhatsNewStrings",
                                "Files now sync up to 5x faster than before"),
                                QString::fromUtf8(::ROCKET_IMAGE)    },
        UpdatesList::UpdateItem{
                                QApplication::translate("WhatsNewStrings", "Greater control"),
                                QApplication::translate("WhatsNewStrings",
                                "Full visibility into sync issues and total control over "
                                    "how conflicts are resolved"),
                                QString::fromUtf8(::CONTROLS_IMAGE)  },
        UpdatesList::UpdateItem{
                                QApplication::translate("WhatsNewStrings", "Advanced filters"),
                                QApplication::translate("WhatsNewStrings",
                                "Advanced settings allow you to write your own exclusion "
                                    "rules for each of your syncs"),
                                QString::fromUtf8(::MEGA_CLOUD_IMAGE)},
    };
    mUpdatesModel.mAcceptButtonText = QApplication::translate("WhatsNewStrings", "Got it");
    mUpdatesModel.mAcceptFunction = []() {};
    return;
}

QString WhatsNewController::acceptButtonText()
{
    return mUpdatesModel.mAcceptButtonText;
}

void WhatsNewController::acceptButtonClicked()
{
    mUpdatesModel.mAcceptFunction();
}
