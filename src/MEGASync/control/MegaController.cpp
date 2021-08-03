#include "MegaController.h"
#include "Utilities.h"
#include "MegaApplication.h"
#include "model/Model.h"

#include <QDateTime>
#include <QPointer>
#include <QDebug>
using namespace mega;

Controller *Controller::controller = NULL;

void Controller::addSync(const QString &localFolder, const MegaHandle &remoteHandle,
                         QString syncName, ActionProgress *progress, mega::MegaSync::SyncType type,
                         bool wait)
{
    assert(api);

    if (!localFolder.size())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                     QString::fromUtf8("Adding invalid sync %1").arg(localFolder).toUtf8().constData());
        if (progress) progress->setFailed(MegaError::API_EARGS);
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                 QString::fromUtf8("Adding sync %1").arg(localFolder).toUtf8().constData());

    if (!wait)
    {
        api->syncFolder(type, localFolder.toUtf8().constData(),
                        syncName.toUtf8().constData(), remoteHandle, nullptr,
                        new ProgressFuncExecuterListener(progress,  true,
                                                         [](MegaApi *api, MegaRequest *request,
                                                         MegaError *e){
                            ///// onRequestFinish Management: ////
                        }));
    }
    else
    {
        mega::SynchronousRequestListener synchro;
        api->syncFolder(type, localFolder.toUtf8().constData(),
                        syncName.toUtf8().constData(), remoteHandle, nullptr, &synchro);
        synchro.wait();
        if (progress)
        {
            if (synchro.getError()->getErrorCode() != mega::MegaError::API_OK)
            {
                progress->setFailed(synchro.getError()->getErrorCode(),
                                    synchro.getRequest(), synchro.getError());
            }
            else
            {
                progress->setComplete();
            }
        }
    }
}

void Controller::removeSync(std::shared_ptr<SyncSetting> syncSetting, ActionProgress *progress,
                            mega::MegaSync::SyncType type)
{
    assert(api);
    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Removing invalid sync").toUtf8().constData());
        if (progress) progress->setFailed(MegaError::API_EARGS);
        return;
    }


    switch (type)
    {
        case mega::MegaSync::TYPE_BACKUP:
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Removing backup").toUtf8().constData());

            api->removeBackup(syncSetting->backupId(),
                              new ProgressFuncExecuterListener(progress,  true, [](MegaApi *api, MegaRequest *request, MegaError *e){
                                  ///// onRequestFinish Management: ////
                              }));
            break;
        }
        case mega::MegaSync::TYPE_TWOWAY:
        default:
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Removing sync").toUtf8().constData());

            api->removeSync(syncSetting->backupId(),
                            new ProgressFuncExecuterListener(progress,  true, [](MegaApi *api, MegaRequest *request, MegaError *e){
                                ///// onRequestFinish Management: ////
                            }));
            break;
        }
    }
}

void Controller::enableSync(std::shared_ptr<SyncSetting> syncSetting, ActionProgress *progress)
{
    assert(api);
    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromAscii("Enabling invalid sync").toUtf8().constData());
        if (progress) progress->setFailed(MegaError::API_EARGS);
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromAscii("Enabling sync %1 to %2")
                 .arg(syncSetting->getLocalFolder()).arg(syncSetting->getMegaFolder()).toUtf8().constData() );

    api->enableSync(syncSetting->backupId(),
        new ProgressFuncExecuterListener(progress,  true, [](MegaApi *api, MegaRequest *request, MegaError *e){
                        ///// onRequestFinish Management: ////
                    }));
}

void Controller::disableSync(std::shared_ptr<SyncSetting> syncSetting, ActionProgress *progress)
{
    assert(api);
    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromAscii("disabling invalid sync").toUtf8().constData());
        if (progress) progress->setFailed(MegaError::API_EARGS);
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromAscii("Disabling sync %1 to %2")
                 .arg(syncSetting->getLocalFolder()).arg(syncSetting->getMegaFolder()).toUtf8().constData() );

    api->disableSync(syncSetting->backupId(),
        new ProgressFuncExecuterListener(progress,  true, [](MegaApi *api, MegaRequest *request, MegaError *e){
                        ///// onRequestFinish Management: ////
                    }));
}

void Controller::createMyBackupsDir(QString& name, ActionProgress* progress)
{
    // Check that name is not empty
    if (name.isEmpty())
    {
        // Name empty, report error
        if (progress) progress->setFailed(MegaError::API_EARGS);
    }
    else
    {
        // Check for name collision
        mega::MegaNode* rootNode (api->getRootNode());
        mega::MegaNode* backupsDirNode (api->getChildNode(rootNode, name.toUtf8()));

        if (backupsDirNode != nullptr)
        {
            // Folder exists, report error
            if (progress) progress->setFailed(MegaError::API_EARGS);
        }
        else
        {            
            mega::SynchronousRequestListener synchro;

            // Create folder
            api->createFolder(name.toUtf8(), rootNode, &synchro);
            synchro.wait();

            if (synchro.getError()->getErrorCode() != mega::MegaError::API_OK)
            {
                // Creation failed, pass error to caller
                if (progress)   progress->setFailed(synchro.getError()->getErrorCode(),
                                                    synchro.getRequest(), synchro.getError());
            }
            else
            {
                // Set the folder as MyBackups root folder
                mega::MegaHandle handle (synchro.getRequest()->getNodeHandle());
                backupsDirNode = api->getNodeByHandle(handle);
                api->setMyBackupsFolder(backupsDirNode->getHandle(), &synchro);
                synchro.wait();

                if (synchro.getError()->getErrorCode() != mega::MegaError::API_OK)
                {
                    // Setting failed, pass error to caller
                    if (progress)   progress->setFailed(synchro.getError()->getErrorCode(),
                                                        synchro.getRequest(), synchro.getError());
                }
                else
                {
                    // Everything went well, update Model
                    Model::instance()->setBackupsDirHandle(handle);
                    progress->setComplete();
                }
            }
        }

        if (backupsDirNode) delete backupsDirNode;
        if (rootNode) delete rootNode;
    }
}

void Controller::setDeviceDir(QString& name, bool reUseDir, ActionProgress* progress)
{
    // Check that name is not empty
    if (name.isEmpty())
    {
        // Name empty, report error
        if (progress) progress->setFailed(MegaError::API_EARGS);
    }
    else
    {
        // Check if folder exists
        mega::MegaNode* backupsDirNode (api->getNodeByHandle(Model::instance()->getBackupsDirHandle()));
        mega::MegaNode* deviceDirNode = api->getChildNode(backupsDirNode, name.toUtf8());

        if (deviceDirNode != nullptr && !reUseDir)
        {
            // Folder exists, report error
            if (progress) progress->setFailed(MegaError::API_EARGS);
        }
        else
        {
            mega::SynchronousRequestListener synchro;

            // Set device name
            api->setDeviceName(name.toUtf8(), &synchro);
            synchro.wait();

            if (synchro.getError()->getErrorCode() != mega::MegaError::API_OK)
            {
                // Setting failed, pass error to caller
                if (progress)   progress->setFailed(synchro.getError()->getErrorCode(),
                                                    synchro.getRequest(), synchro.getError());
            }
            else
            {
                // Everything went well, update the Model
                Model::instance()->setDeviceName(name);
                progress->setComplete();
            }
        }
        if (backupsDirNode) delete backupsDirNode;
        if (deviceDirNode) delete deviceDirNode;
    }
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

void ProgressHelper::onStepProgress(double percentage, int position)
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

void ProgressHelper::onStepCompleted(int position)
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

void ProgressFuncExecuterListener::onRequestStart(MegaApi *api, MegaRequest *request)
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
