#include "MediaTypeFilterWidget.h"

#include "ui_MediaTypeFilterWidget.h"

MediaTypeFilterWidget::MediaTypeFilterWidget(QWidget* parent):
    QWidget(parent),
    mUi(new Ui::MediaTypeFilterWidget)
{
    mUi->setupUi(this);

    initializeVisibilityStates();
    updateStrings();

    connect(mUi->bTitle,
            &QPushButton::toggled,
            this,
            &MediaTypeFilterWidget::handleGroupboxToggled);
}

MediaTypeFilterWidget::~MediaTypeFilterWidget()
{
    delete mUi;
}

bool MediaTypeFilterWidget::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);
        updateStrings();
    }
    return QWidget::event(event);
}

QFrame* MediaTypeFilterWidget::getFrame(TransfersWidget::TM_TAB tab) const
{
    switch (tab)
    {
        case TransfersWidget::TYPE_OTHER_TAB:
            return mUi->fOther;
        case TransfersWidget::TYPE_AUDIO_TAB:
            return mUi->fAudio;
        case TransfersWidget::TYPE_VIDEO_TAB:
            return mUi->fVideos;
        case TransfersWidget::TYPE_ARCHIVE_TAB:
            return mUi->fArchives;
        case TransfersWidget::TYPE_DOCUMENT_TAB:
            return mUi->fDocuments;
        case TransfersWidget::TYPE_IMAGE_TAB:
            return mUi->fImages;
        default:
            return nullptr;
    }
}

QLabel* MediaTypeFilterWidget::getLabel(TransfersWidget::TM_TAB tab) const
{
    switch (tab)
    {
        case TransfersWidget::TYPE_OTHER_TAB:
            return mUi->lOtherNb;
        case TransfersWidget::TYPE_AUDIO_TAB:
            return mUi->lAudioNb;
        case TransfersWidget::TYPE_VIDEO_TAB:
            return mUi->lVideosNb;
        case TransfersWidget::TYPE_ARCHIVE_TAB:
            return mUi->lArchivesNb;
        case TransfersWidget::TYPE_DOCUMENT_TAB:
            return mUi->lDocumentsNb;
        case TransfersWidget::TYPE_IMAGE_TAB:
            return mUi->lImagesNb;
        default:
            return nullptr;
    }
}

void MediaTypeFilterWidget::resetCounter(TransfersWidget::TM_TAB tab)
{
    if (!isVisible(tab))
    {
        return;
    }

    QLabel* label(getLabel(tab));
    if (label)
    {
        label->parentWidget()->hide();
        label->clear();
    }
}

void MediaTypeFilterWidget::showIfGroupboxVisible(TransfersWidget::TM_TAB tab,
                                                  unsigned long long counter)
{
    QLabel* label(getLabel(tab));
    if (!label)
    {
        return;
    }

    QString countLabelText(counter > 0 ? QString::number(counter) : QString());
    if (mUi->bTitle->isChecked())
    {
        label->parentWidget()->show();
    }
    label->setVisible(counter > 0);
    label->setText(countLabelText);
}

void MediaTypeFilterWidget::handleGroupboxToggled(bool checked)
{
    for (auto& [tab, frameVisible]: mVisibilityMap)
    {
        QFrame* frame = getFrame(tab);
        if (checked)
        {
            if (frameVisible)
            {
                frame->show();
            }
        }
        else
        {
            mVisibilityMap[tab] = frame->isVisible();
            frame->hide();
        }
    }
}

void MediaTypeFilterWidget::initializeVisibilityStates()
{
    mVisibilityMap[TransfersWidget::TYPE_OTHER_TAB] = false;
    mVisibilityMap[TransfersWidget::TYPE_AUDIO_TAB] = false;
    mVisibilityMap[TransfersWidget::TYPE_VIDEO_TAB] = false;
    mVisibilityMap[TransfersWidget::TYPE_ARCHIVE_TAB] = false;
    mVisibilityMap[TransfersWidget::TYPE_DOCUMENT_TAB] = false;
    mVisibilityMap[TransfersWidget::TYPE_IMAGE_TAB] = false;
}

bool MediaTypeFilterWidget::isVisible(TransfersWidget::TM_TAB tab) const
{
    auto it = mVisibilityMap.find(tab);
    return it != mVisibilityMap.end() ? it->second : false;
}

void MediaTypeFilterWidget::setIsVisible(TransfersWidget::TM_TAB tab, bool isVisible)
{
    auto it = mVisibilityMap.find(tab);
    if (it != mVisibilityMap.end())
    {
        it->second = isVisible;
    }
}

void MediaTypeFilterWidget::updateStrings()
{
    mUi->bTitle->setText(QCoreApplication::translate("TransferManager", "Media type"));
    mUi->bArchives->setText(QCoreApplication::translate("TransferManager", "Archives"));
    mUi->bDocuments->setText(QCoreApplication::translate("TransferManager", "Documents"));
    mUi->bImages->setText(QCoreApplication::translate("TransferManager", "Images"));
    mUi->bVideos->setText(QCoreApplication::translate("TransferManager", "Videos"));
    mUi->bAudio->setText(QCoreApplication::translate("TransferManager", "Audio"));
    mUi->bOther->setText(QCoreApplication::translate("TransferManager", "Other"));
}
