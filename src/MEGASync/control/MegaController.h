#pragma once

#include <memory>
#include <functional>
#include <QString>
#include <QList>

#include "model/Model.h"
#include "megaapi.h"


class ProgressStep;
class ActionProgress;

/**
 * @brief Controller class
 *
 * This class intends to serve the most common use cases derived from
 * user interaction and thus alliviate MegaApplication of those.
 *
 * Currently it holds sync use cases.
 *
 * It uses Progress Helper classes to provide callers with progress updates
 *  and the chance to configure actions to be taken in case of failures.
 *
 * For further info: see ActionProgress.
 *
 */
class Controller
{
public:

    void addSync(const QString &localFolder, const mega::MegaHandle &remoteHandle, QString syncName = QString(), ActionProgress *progress = nullptr);
    void removeSync(std::shared_ptr<SyncSetting> syncSetting, ActionProgress *progress = nullptr);
    void enableSync(std::shared_ptr<SyncSetting> syncSetting, ActionProgress *progress = nullptr);
    void disableSync(std::shared_ptr<SyncSetting> syncSetting, ActionProgress *progress = nullptr);

    static Controller *instance();
    void setApi(mega::MegaApi *value);
    static QString getSyncNameFromPath(const QString& path);

private:

    mega::MegaApi *api;
    static Controller *controller;
};


/**
 * @brief A listener that holds an ActionProgress object
 * to manage progress/completion/failure of a MegaRequest
 * and a function to be executed upon onRequestFinish
 * *
 */
class ProgressFuncExecuterListener : public mega::MegaRequestListener
{
private:

    ActionProgress *mProgressHelper;

    bool mAutoremove = true;
    bool executeInAppThread = true;
    std::function<void(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError *e)> onRequestFinishCallback;

public:

    /**
     * @brief ProgressFuncExecuterListener constructor
     * @param func to call upon onRequestFinish
     * @param autoremove whether this should be deleted after func is called
     */
    ProgressFuncExecuterListener(ActionProgress *progressHepler, bool autoremove = false,
                                 std::function<void(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError *e)> func = nullptr
                                 )
        : mProgressHelper(progressHepler), mAutoremove(autoremove), onRequestFinishCallback(std::move(func))
    {
    }

    virtual void onRequestFinish(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError *e);
    virtual void onRequestStart(mega::MegaApi* api, mega::MegaRequest *request);
    virtual void onRequestUpdate(mega::MegaApi*, mega::MegaRequest*) {}
    virtual void onRequestTemporaryError(mega::MegaApi*, mega::MegaRequest*, mega::MegaError*) {}
};


/**
 * @brief Progress Helper
 *
 * An object of this type can be used to manage the progress of a task.
 * It allows for setting progress/completion and wil emit signals when
 * progress advances and upon completion too, so that client classes
 * can connect to them and interact accordingly
 *
 * It features a hierarchical structure of steps (ProgressStep) with
 * different weights, so that progress is advanced automatically
 * according to the progress of child steps.
 *
 * Objects of this class will be deleted after emiting completion
 * unless otherwise specified (see Constructor)
 */
class ProgressHelper : public QObject
{
    Q_OBJECT

private:
    bool completed_signaled = false;
    double mPercentage = 0.0;
    QString mDescription;

    QList<ProgressStep> steps;
    int completedtasks = 0;

    bool mDeleteOnCompletion = false;

public:
    ProgressHelper(bool deleteOnCompletion = false, const QString description = QString());
    ~ProgressHelper();
    double percentage() const;
    void setPercentage(double value);
    void setComplete();
    ProgressHelper *addStep(const QString &description = QString(), double weight = 1.0);
    void addStep(ProgressHelper *task, double weight = 1.0);


    QString description() const;
    void setDescription(const QString &description);

    void checkCompletion();

private slots:
    void onStepProgress(double percentage, int position);
    void onStepCompleted(int position);

signals:
    void progress(double percentage);
    void completed();
};

/**
 * @brief Class to ensure that a progress helper created in a context gets completed if no subtasks are asigned to it
 */
class ProgressHelperCompletionGuard
{
private:
    ProgressHelper *mProgressHelper = nullptr;
public:
    ProgressHelperCompletionGuard(ProgressHelper *progressHelper) : mProgressHelper(progressHelper) {};
    ~ProgressHelperCompletionGuard()
    {
        if (mProgressHelper)
        {
            mProgressHelper->checkCompletion();
        }
    }
};


/**
 * @brief Action progress helper
 *
 * This is an special ProgressHelper that can be used to track
 * the progress of a Request.
 *
 * It will emit `failedRequest` or `failed` in case the caller sets it as failed.
 * `failed` will be emmited only in the abscense of MegaRequest/MegaError.
 *
 * You probably want to connect for both signals in case you wan to handle
 * all errors.
 */
class ActionProgress : public ProgressHelper
{
    Q_OBJECT

private:
    QString mError;

public:
    ActionProgress(bool deleteOnCompletion = false, const QString description = QString());

    QString error() const;
    void setFailed(int errorCode, mega::MegaRequest *request = nullptr,  mega::MegaError *error = nullptr); //note, this will call SetComplete() & hence emit a completion signal

signals:
    /**
     * @brief to be emited when the action fails without a MegaRequest/MegaError (it might not even have reached to a request)
     * @param errorCode
     */
    void failed(int errorCode);

    /**
     * @brief to be emited when a request fails
     * @param request. ActionProgress simply forwards the pointer to the request, this signal should be connected
     * with Qt::DirectConnection before this becomes a dangling pointer
     * @param error. ActionProgress simply forwards the pointer to the error, this signal should be connected
     * with Qt::DirectConnection before this becomes a dangling pointer
     */
    void failedRequest(mega::MegaRequest *request, mega::MegaError *error);
};


/**
 * @brief Progress step
 *
 * A combination of a ProgressHelper object (a task)
 * and its weight
 */
class ProgressStep
{
public:
    ProgressStep(ProgressHelper *task, double weight = 1.0);
    ProgressHelper *task() const;
    double weight() const;


private:
    ProgressHelper *mTask;
    double mWeight;
};



