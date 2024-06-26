#ifndef INTERVALEXECUTIONER_H
#define INTERVALEXECUTIONER_H

#include <QObject>
#include <QTimer>

#include <memory>

class IntervalExecutioner : public QObject
{
    Q_OBJECT

public:
    IntervalExecutioner(int minAmountMsBetweenExecutions = 200); // Constructor with default delay value

    void scheduleExecution();

signals:
    void execute(); // Signal to notify when execution can take place

private slots:
    void timerTimeout(); // Slot to handle QTimer's timeout signal

private:
    void startExecution();

    std::unique_ptr<QTimer> mTimer;
    bool mExecutionPending;
};

#endif // INTERVALEXECUTIONER_H
