#ifndef SYNCEXCLUSIONS_H
#define SYNCEXCLUSIONS_H

#include "ExclusionRulesModel.h"
#include "QmlDialogWrapper.h"

#include <QScreen>

class SyncExclusions : public QMLComponent
{
    Q_OBJECT
    Q_PROPERTY(double minimumAllowedSize MEMBER mMinimumAllowedSize READ getMinimumAllowedSize WRITE setMinimumAllowedSize NOTIFY minimumAllowedSizeChanged)
    Q_PROPERTY(double maximumAllowedSize MEMBER mMaximumAllowedSize READ getMaximumAllowedSize WRITE setMaximumAllowedSize NOTIFY maximumAllowedSizeChanged)
    Q_PROPERTY(int minimumAllowedUnit MEMBER mMinimumAllowedUnit READ getMinimumAllowedUnit WRITE setMinimumAllowedUnit NOTIFY minimumAllowedUnitChanged)
    Q_PROPERTY(int maximumAllowedUnit MEMBER mMaximumAllowedUnit READ getMaximumAllowedUnit WRITE setMaximumAllowedUnit NOTIFY maximumAllowedUnitChanged)
    Q_PROPERTY(ExclusionRulesModel* rulesModel MEMBER mRulesModel CONSTANT)
    Q_PROPERTY(SizeExclusionStatus sizeExclusionStatus READ getSizeExclusionStatus WRITE setSizeExclusionStatus NOTIFY sizeExclusionStatusChanged)
    Q_PROPERTY(QString folderName READ getFolderName NOTIFY folderNameChanged)
    Q_PROPERTY(QString folderPath MEMBER mFolderFullPath)
    Q_PROPERTY(bool askOnExclusionRemove READ isAskOnExclusionRemove WRITE setAskOnExclusionRemove NOTIFY askOnExclusionRemoveChanged)

public:
    SyncExclusions(QWidget *parent = 0, const QString &path = QString::fromUtf8(""));
    ~SyncExclusions();

    enum SizeExclusionStatus{
        OUTSIDE_OF = 2,
        SMALLER_THAN = 1,
        BIGGER_THAN = 0,
        DISABLED = 3
    };
    Q_ENUM(SizeExclusionStatus)

    double getMinimumAllowedSize() const { return mMinimumAllowedSize; }
    void setMinimumAllowedSize(double minimumSize);
    double getMaximumAllowedSize()  const {return mMaximumAllowedSize;}
    void setMaximumAllowedSize(double maximumSize);
    int getMinimumAllowedUnit() const {return mMinimumAllowedUnit;}
    void setMinimumAllowedUnit(int minimumUnit);
    int getMaximumAllowedUnit()  const {return mMaximumAllowedUnit;}
    void setMaximumAllowedUnit(int maximumUnit);
    SizeExclusionStatus getSizeExclusionStatus()  const;
    void setSizeExclusionStatus(SizeExclusionStatus);
    QString getFolderName() const { return mFolderName; }
    void setFolder(const QString& folderName);

    Q_INVOKABLE void restoreDefaults();
    Q_INVOKABLE void showRemoveRuleConfirmationMessageDialog(const QString& descriptionText);

    bool isAskOnExclusionRemove()  const;
    void setAskOnExclusionRemove(bool);

    std::pair<unsigned long long, int> fromDisplay(double value , int unit) const;
    std::pair<double, int> toDisplay(unsigned long long value , int unit) const;

    QUrl getQmlUrl() override;

public slots:
    void applyChanges();

signals:
    void minimumAllowedSizeChanged(double);
    void maximumAllowedSizeChanged(double);
    void minimumAllowedUnitChanged(int);
    void maximumAllowedUnitChanged(int);
    void sizeExclusionStatusChanged(SizeExclusionStatus);
    void folderNameChanged(QString);
    void askOnExclusionRemoveChanged(bool);
    void acceptedClicked();

private:
    double mMinimumAllowedSize;
    double mMaximumAllowedSize;
    MegaIgnoreSizeRule::UnitTypes mMinimumAllowedUnit = MegaIgnoreSizeRule::B;
    MegaIgnoreSizeRule::UnitTypes mMaximumAllowedUnit = MegaIgnoreSizeRule::B;
    std::shared_ptr<MegaIgnoreManager> mMegaIgnoreManager; // TODO: Remove this and make all usage through the model
    ExclusionRulesModel* mRulesModel;
    QString mFolderName;
    QString mFolderFullPath;
};

#endif // SYNCEXCLUSIONS_H
