#include "MediaTypeFilterWidget.h"

#include "tokenizer/TokenizableItems/TokenPropertySetter.h"
#include "ui_MediaTypeFilterWidget.h"

MediaTypeFilterWidget::MediaTypeFilterWidget(QWidget* parent):
    QWidget(parent),
    mUi(new Ui::MediaTypeFilterWidget)
{
    mUi->setupUi(this);

    initializeVisibilityStates();
    updateStrings();

    connect(mUi->bGroupBoxTitle,
            &QPushButton::toggled,
            this,
            &MediaTypeFilterWidget::handleGroupboxToggled);

    mTabSelectorsByType.insert(TransfersWidget::TYPE_ARCHIVE_TAB, mUi->tabArchives);
    mTabSelectorsByType.insert(TransfersWidget::TYPE_DOCUMENT_TAB, mUi->tabDocuments);
    mTabSelectorsByType.insert(TransfersWidget::TYPE_IMAGE_TAB, mUi->tabImages);
    mTabSelectorsByType.insert(TransfersWidget::TYPE_VIDEO_TAB, mUi->tabVideos);
    mTabSelectorsByType.insert(TransfersWidget::TYPE_AUDIO_TAB, mUi->tabAudio);
    mTabSelectorsByType.insert(TransfersWidget::TYPE_OTHER_TAB, mUi->tabOther);

    // Left pane tokens
    {
        BaseTokens iconTokens;
        iconTokens.setNormalOff(QLatin1String("icon-secondary"));
        iconTokens.setNormalOn(QLatin1String("icon-primary"));
        auto iconTokenSetter = std::make_shared<TokenPropertySetter>(iconTokens);

        TabSelector::applyTokens(mUi->gbMediaType, iconTokenSetter);
    }
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

void MediaTypeFilterWidget::resetCounter(TransfersWidget::TM_TAB tab)
{
    auto tabSelector = mTabSelectorsByType.value(tab);
    if (tabSelector)
    {
        tabSelector->hide();
    }
}

void MediaTypeFilterWidget::setCounter(TransfersWidget::TM_TAB tab, unsigned long long counter)
{
    auto tabSelector = mTabSelectorsByType.value(tab);
    if (tabSelector)
    {
        tabSelector->show();
        tabSelector->setCounter(counter);
    }
}

TabSelector* MediaTypeFilterWidget::getTabSelectorByType(TransfersWidget::TM_TAB tab)
{
    return mTabSelectorsByType.value(tab);
}

TransfersWidget::TM_TAB MediaTypeFilterWidget::getTabByTabSelector(TabSelector* tabSelector)
{
    return mTabSelectorsByType.key(tabSelector);
}

void MediaTypeFilterWidget::handleGroupboxToggled(bool checked)
{
    mUi->gbMediaType->setVisible(checked);
}

void MediaTypeFilterWidget::updateStrings()
{
    mUi->bGroupBoxTitle->setText(QCoreApplication::translate("TransferManager", "Media type"));
    mUi->tabArchives->setTitle(QCoreApplication::translate("TransferManager", "Archives"));
    mUi->tabDocuments->setTitle(QCoreApplication::translate("TransferManager", "Documents"));
    mUi->tabImages->setTitle(QCoreApplication::translate("TransferManager", "Images"));
    mUi->tabVideos->setTitle(QCoreApplication::translate("TransferManager", "Videos"));
    mUi->tabAudio->setTitle(QCoreApplication::translate("TransferManager", "Audio"));
    mUi->tabOther->setTitle(QCoreApplication::translate("TransferManager", "Other"));
}
