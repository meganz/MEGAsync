#include "MegaController.h"
#include "Utilities.h"
#include "MegaApplication.h"
#include <QDateTime>
#include <QPointer>
#include <QDebug>

using namespace mega;



Controller *Controller::controller = NULL;

Controller *Controller::instance()
{
    if (!controller)
    {
        controller = new Controller();
    }
    return Controller::controller;
}

void Controller::setApi(mega::MegaApi *value)
{
    api = value;
}

//void Controller::doSth(parameters, ActionProgress *progress)
//{
//    assert(api);

//    api->doSth(paremeters, new ProgressFuncExecuterListener(progress, [](MegaApi *api, MegaRequest *request, MegaError *e){
//        //emmit sync called, so that suscribers (menus & settings) reload their UI
//        qDebug() << "doSth call ended alright" << endl;

//        emit sthDone();
//    }, true));
//}

QString ProgressHelper::description() const
{
    return mDescription;
}

void ProgressHelper::setDescription(const QString &description)
{
    mDescription = description;
}

ProgressHelper::ProgressHelper(bool deleteOnCompletion, const QString description)
    : mDescription(description), mDeleteOnCompletion(deleteOnCompletion)
{

}

double ProgressHelper::percentage() const
{
    return mPercentage;
}

void ProgressHelper::setPercentage(double value)
{
    mPercentage = value;
    emit progress(mPercentage);
    qDebug() << " Task <" << description() << "> addvanced to " << percentage() << endl; //TODO: delete
}

void ProgressHelper::setComplete()
{
    if (!completed_signaled)
    {
        emit completed();
    }
    completed_signaled = true;
    if (mDeleteOnCompletion)
    {
        this->deleteLater();
    }
    qDebug() << " Task <" << description() << "> completed!"<< endl; //TODO: delete
}

ProgressHelper *ProgressHelper::addStep(const QString &description, double weight)
{
    ProgressHelper *task = new ProgressHelper(true, description);
    addStep(task, weight);
    return task;
}

void ProgressHelper::addStep(ProgressHelper *task, double weight)
{
    int pos = steps.size();

    connect(task, &ProgressHelper::progress, this, [this, pos](double progress)
    {
        onStepProgress(progress, pos);
    });
    connect(task, &ProgressHelper::completed, this, [this, pos]()
    {
        onStepCompleted(pos);
    });
    steps.append(ProgressStep(task, weight));
}

void ProgressHelper::onStepProgress(double percentage, int position)
{
    qDebug() << "Step " << position << ": <" << steps[position].task()->description()
             << "> addvanced to " << percentage << endl; //TODO: delete
    double weightsSum = 0;
    double newCompleted = 0.0;
    foreach(ProgressStep step, steps)
    {
        weightsSum += step.weight();
        newCompleted += step.weight() * step.task()->percentage();
    }
    double newPercentage = newCompleted / weightsSum;

    setPercentage(newPercentage);
}

void ProgressHelper::onStepCompleted(int position)
{
    qDebug() << "Step " << position << ": <" << steps[position].task()->description()
             << "> completed " << endl; //TODO: delete
    completedtasks++;
    if (completedtasks == steps.size())
    {
        this->setComplete();
    }
}

ProgressHelper::~ProgressHelper()
{
    if (!completed_signaled)
    {
        emit completed();
    }
}

ProgressStep::ProgressStep(ProgressHelper *task, double weight)
    : mTask(task), mWeight(weight)
{
}

ProgressHelper *ProgressStep::task() const
{
    return mTask;
}

double ProgressStep::weight() const
{
    return mWeight;
}


ActionProgress::ActionProgress(bool deleteOnCompletion, const QString description)
    : ProgressHelper(deleteOnCompletion, description)
{

}

QString ActionProgress::error() const
{
    return mError;
}

void ActionProgress::setFailed(int errorCode)
{
    mError = errorCode;
    emit failed(errorCode);
    setComplete();
}

void ProgressFuncExecuterListener::onRequestStart(MegaApi *api, MegaRequest *request, MegaError *e)
{
    if (mProgressHelper)
    {
        mProgressHelper->setPercentage(0.3); //consider starting as an initial progress
    }
}

void ProgressFuncExecuterListener::onRequestFinish(MegaApi *api, MegaRequest *request, MegaError *e)
{
    // progress the helper accordingly
    if (mProgressHelper)
    {
        mProgressHelper->setPercentage(1);
        if (e->getErrorCode())
        {
            mProgressHelper->setFailed(e->getErrorCode());
        }
        mProgressHelper->setComplete();
    }

    // launch callback
    if (executeInAppThread)
    {
        QObject temporary;

        MegaRequest *requestCopy = request->copy();
        MegaError *errorCopy = e->copy();
        QObject::connect(&temporary, &QObject::destroyed, qApp, [this, api, requestCopy, errorCopy](){
            onRequestFinishCallback(api, requestCopy, errorCopy);

            if (mAutoremove)
            {
                delete this;
            }

        }, Qt::QueuedConnection);
    }
    else
    {
        onRequestFinishCallback(api, request, e);

        if (mAutoremove)
        {
            delete this;
        }
    }
}
