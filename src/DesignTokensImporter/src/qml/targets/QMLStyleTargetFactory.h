#ifndef QML_STYLE_TARGET_FACTORY_H
#define QML_STYLE_TARGET_FACTORY_H

#include "IQMLStyleTarget.h"

#include <QStringList>

#include <unordered_map>
#include <memory>

namespace DTI
{
    class IConcreteStyleTargetFactory
    {
        public :
            virtual IQMLStyleTarget* makeQMLStyleTarget() = 0;
    };

    using QMLStyleTargetFactoryMap = std::unordered_map<std::string, std::unique_ptr<IConcreteStyleTargetFactory>>;

    class QMLStyleTargetFactory
    {
    public:
        QMLStyleTargetFactory() = delete;

        /*
         * Note on pointer ownership: function caller is responsible for deletion of this pointer.
        */
        static IQMLStyleTarget* getQMLStyleTarget(const QString& styleTargetId)
        {
            const auto& styleTargetFactories = getFactories();
            auto foundIt = styleTargetFactories.find(styleTargetId.toStdString());
            if (foundIt != styleTargetFactories.end())
            {
                return foundIt->second->makeQMLStyleTarget();
            }
            else
            {
                return nullptr;
            }
        }

        static bool registerStyleTargetFactory(const std::string& styleTargetId, IConcreteStyleTargetFactory* styleTargetFactory)
        {
            auto& styleTargetFactories = getFactories();
            if (styleTargetFactories.find(styleTargetId) == styleTargetFactories.end())
            {
                auto result = styleTargetFactories.emplace(styleTargetId, styleTargetFactory);
                return result.second;
            }
            else
            {
                return false;
            }
        }

        static QStringList getRegisteredStyleTargets()
        {
            QStringList registeredStyleTargets;
            for (const auto& styleTargetFactory : getFactories())
            {
                registeredStyleTargets.push_back(QString::fromStdString(styleTargetFactory.first));
            }
            return registeredStyleTargets;
        }

        private:
        static QMLStyleTargetFactoryMap& getFactories()
        {
            static QMLStyleTargetFactoryMap styleTargetFactories{};
            return styleTargetFactories;
        }
    };

    template<class TQMLStyleTarget>
    class ConcreteQMLStyleFactory : public IConcreteStyleTargetFactory
    {
    public:
        static bool Register(const std::string& styleTargetId)
        {
            return QMLStyleTargetFactory::registerStyleTargetFactory(styleTargetId, new ConcreteQMLStyleFactory<TQMLStyleTarget>());
        };

        TQMLStyleTarget* makeQMLStyleTarget()
        {
            return new TQMLStyleTarget();
        }
    };
}

#endif
