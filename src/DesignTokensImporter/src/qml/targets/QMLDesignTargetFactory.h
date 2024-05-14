#ifndef QML_DESIGN_TARGET_FACTORY_H
#define QML_DESIGN_TARGET_FACTORY_H

#include "QMLDesignTarget.h"

#include <QStringList>

#include <unordered_map>
#include <memory>

namespace DTI
{
    class IConcreteDesignTargetFactory
    {
        public :
        virtual IQMLDesignTarget* makeQMLDesignTarget() = 0;
    };

    using QMLDesignTargetFactoryMap = std::unordered_map<std::string, std::unique_ptr<IConcreteDesignTargetFactory>>;

    class QMLDesignTargetFactory
    {
    public:
        QMLDesignTargetFactory() = delete;

        /*
         * Note on pointer ownership: function caller is responsible for deletion of this pointer.
        */
        static IQMLDesignTarget* getQMLDesignTarget(const QString& targetId)
        {
            const auto& qmlDesignTargetFactories = getDesignTargetFactories();
            auto foundIt = qmlDesignTargetFactories.find(targetId.toStdString());
            if (foundIt != qmlDesignTargetFactories.end())
            {
                return foundIt->second->makeQMLDesignTarget();
            }

            return nullptr;
        }

        static bool registerDesignTargetFactory(const std::string& targetId, IConcreteDesignTargetFactory* targetFactory)
        {
            auto& DesignTargetFactories = getDesignTargetFactories();
            if (DesignTargetFactories.find(targetId) == DesignTargetFactories.end())
            {
                auto result = DesignTargetFactories.emplace(targetId, targetFactory);
                return result.second;
            }

            return false;
        }

        static QStringList getRegisteredDesignTargets()
        {
            QStringList registeredDesignTargets;
            for (const auto& designTargetFactory : getDesignTargetFactories())
            {
                registeredDesignTargets.push_back(QString::fromStdString(designTargetFactory.first));
            }
            return registeredDesignTargets;
        }

        private:
        static QMLDesignTargetFactoryMap& getDesignTargetFactories()
        {
            static QMLDesignTargetFactoryMap designTargetFactories{};
            return designTargetFactories;
        }
    };

    template<class TQMLDesignTarget>
    class ConcreteQMLDesignTargetFactory : public IConcreteDesignTargetFactory
    {
    public:
        static bool Register(const std::string& targetId)
        {
            return QMLDesignTargetFactory::registerDesignTargetFactory(targetId, new ConcreteQMLDesignTargetFactory<TQMLDesignTarget>());
        };

        TQMLDesignTarget* makeQMLDesignTarget() override
        {
            return new TQMLDesignTarget();
        }
    };
}

#endif
