#ifndef QSEGMENTEDCONTROL_H
#define QSEGMENTEDCONTROL_H

#include <QWidget>
#include <QPointer>
#include <QMacCocoaViewContainer>

class QSegmentedControl : public QWidget
{
    Q_OBJECT

public:
    enum {
        TYPE_TABLE = 0,
        TYPE_TAB
    };

    enum {
        UNDEFINED = -1,
        ADD_BUTTON = 0,
        REMOVE_BUTTON
    };

    QSegmentedControl(QWidget *parent = 0);
    void configureTableSegment();
    void configureTabSegment(QStringList options);
    void clicked(long segment);
    void setTableButtonEnable(int segment, bool enable);
    ~QSegmentedControl();

private:
    QMacCocoaViewContainer *cocoaContainer;
    QLayout *parentLayout;
    int segmentType;

    void setupView(NSView *cocoaView, QWidget *parent);
    void clearLayout (QWidget* widget);

signals:
    // Signals for table segment control
    void addButtonClicked();
    void removeButtonClicked();
    // Signal for text tab segment control
    void segmentClicked(int segment); // Segments are numbered from 0 (left to right)
};

#endif // QSEGMENTEDCONTROL_H
