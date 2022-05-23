#ifndef STALLEDISSUEBASEDELEGATEWIDGET_H
#define STALLEDISSUEBASEDELEGATEWIDGET_H

#include <StalledIssue.h>
#include <StalledIssuesUtilities.h>

#include <QWidget>
#include <QStyleOptionViewItem>


class StalledIssueBaseDelegateWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StalledIssueBaseDelegateWidget(QWidget *parent = nullptr);

    virtual void expand(bool){}

    void render(const QStyleOptionViewItem &option,
                QPainter *painter,
                const QRegion &sourceRegion);
    virtual void updateUi(const QModelIndex &index, const StalledIssueVariant &data);
    virtual void setIndent(int){}

    QModelIndex getCurrentIndex() const;
    const StalledIssueVariant &getData() const;

    bool keepEditor() const;
    void setKeepEditor(bool newKeepEditor);

signals:
    void editorKeepStateChanged(bool state);

protected:
    StalledIssuesUtilities mUtilities;
    bool mIsSolved;

private:
    virtual void refreshUi() = 0;

    StalledIssueVariant mData;
    QModelIndex mCurrentIndex;
    bool mKeepEditor;
};

#endif // STALLEDISSUEBASEDELEGATEWIDGET_H
