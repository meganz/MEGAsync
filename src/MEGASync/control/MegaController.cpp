#include "MegaController.h"
#include "Utilities.h"
#include "MegaApplication.h"
#include <QDateTime>
#include <QPointer>
#include <QDebug>
using namespace mega;

Controller *Controller::controller = NULL;

void Controller::addSync(const QString &localFolder, MegaHandle remoteHandle, QString syncName, ActionProgress *progress)
{
    assert(api);

    if (!localFolder.size())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromAscii("Adding invalid sync %1").arg(localFolder).toUtf8().constData());
        if (progress) progress->setFailed(MegaError::API_EARGS);
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromAscii("Adding sync %1").arg(localFolder).toUtf8().constData());

    api->syncFolder(MegaSync::TYPE_TWOWAY, localFolder.toUtf8().constData(), syncName.toUtf8().constData(), remoteHandle,
        nullptr, new ProgressFuncExecuterListener(progress,  true, [](MegaApi*, MegaRequest*, MegaError*){
                        ///// onRequestFinish Management: ////
                    }));
}

void Controller::removeSync(std::shared_ptr<SyncSetting> syncSetting, ActionProgress *progress)
{
    assert(api);
    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromAscii("Removing invalid sync").toUtf8().constData());
        if (progress) progress->setFailed(MegaError::API_EARGS);
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromAscii("Removing sync").toUtf8().constData());

    api->removeSync(syncSetting->backupId(),
        new ProgressFuncExecuterListener(progress,  true, [](MegaApi*, MegaRequest*, MegaError*){
                        ///// onRequestFinish Management: ////
                    }));
}

void Controller::setSyncRunState(mega::MegaSync::SyncRunningState newState, std::shared_ptr<SyncSetting> syncSetting, ActionProgress *progress)
{
    assert(api);
    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromAscii("Set state of invalid sync").toUtf8().constData());
        if (progress) progress->setFailed(MegaError::API_EARGS);
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                 QString::fromAscii("Set state of sync %1 to %2")
                 .arg(syncSetting->getLocalFolder())
                 .arg(newState)
                 .toUtf8()
                 .constData());

    api->setSyncRunState(syncSetting->backupId(), newState,
        new ProgressFuncExecuterListener(progress,  true, [](MegaApi *, MegaRequest *, MegaError *){
                        ///// onRequestFinish Management: ////
                    }));
}

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

QString ProgressHelper::description() const
{
    return mDescription;
}

void ProgressHelper::setDescription(const QString &description)
{
    mDescription = description;
}

void ProgressHelper::checkCompletion()
{
    if (completedtasks == steps.size())
    {
        this->setComplete();
    }
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

void ProgressHelper::onStepProgress(double/* percentage*/, int/* position*/)
{
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

void ProgressHelper::onStepCompleted(int/* position*/)
{
    completedtasks++;
    checkCompletion();
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

void ActionProgress::setFailed(int errorCode, MegaRequest *request, MegaError *e)
{
    mError = errorCode;
    if (request && e)
    {
        emit failedRequest(request, e);
    }
    else
    {
        emit failed(errorCode);
    }
    setComplete();
}

void ProgressFuncExecuterListener::onRequestStart(MegaApi*, MegaRequest*)
{
    if (mProgressHelper)
    {
        mProgressHelper->setPercentage(0.3); //considered started as an initial progress
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
            mProgressHelper->setFailed(e->getErrorCode(), request, e);
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

            if (onRequestFinishCallback)
            {
                onRequestFinishCallback(api, requestCopy, errorCopy);
            }

            if (mAutoremove)
            {
                delete this;
            }

        }, Qt::QueuedConnection);
    }
    else
    {
        if (onRequestFinishCallback)
        {
            onRequestFinishCallback(api, request, e);
        }

        if (mAutoremove)
        {
            delete this;
        }
    }
}
