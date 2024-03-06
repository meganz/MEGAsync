#ifndef QTWIDGET_STYLE_TARGET_FACTORY_H
#define QTWIDGET_STYLE_TARGET_FACTORY_H

#include "IQTWIDGETStyleTarget.h"
#include "Types.h"


#include <unordered_map>
#include <memory>

namespace DTI
{
    class IStyleTargetFactory
    {
        public :
            virtual IQTWIDGETStyleTarget* makeQTWIDGETStyleTarget() = 0;
    };

    using QTWIDGETStyleTargetFactoryMap = std::unordered_map<std::string, std::unique_ptr<IStyleTargetFactory>>;

    class QTWIDGETStyleTargetFactory
    {
    public:
        QTWIDGETStyleTargetFactory() = delete;

        /*
         * Note on pointer ownership: function caller is responsible for deletion of this pointer.
        */
        static IQTWIDGETStyleTarget* getQTWIDGETStyleTarget(const QString& styleTargetId)
        {
            const auto& styleTargetFactories = getFactories();
            auto foundIt = styleTargetFactories.find(styleTargetId.toStdString());
            if (foundIt != styleTargetFactories.end())
            {
                return foundIt->second->makeQTWIDGETStyleTarget();
            }

            return nullptr;
        }

        static bool registerStyleTargetFactory(const std::string& styleTargetId, IStyleTargetFactory* styleTargetFactory)
        {
            auto& styleTargetFactories = getFactories();
            if (styleTargetFactories.find(styleTargetId) == styleTargetFactories.end())
            {
                auto result = styleTargetFactories.emplace(styleTargetId, styleTargetFactory);
                return result.second;
            }

            return false;
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
        static QTWIDGETStyleTargetFactoryMap& getFactories()
        {
            static QTWIDGETStyleTargetFactoryMap styleTargetFactories{};
            return styleTargetFactories;
        }
    };

    template<class TQTWIDGETStyleTarget>
    class QTWIDGETStyleFactory : public IStyleTargetFactory
    {
    public:
        static bool Register(const std::string& styleTargetId)
        {
            return QTWIDGETStyleTargetFactory::registerStyleTargetFactory(styleTargetId, new QTWIDGETStyleFactory<TQTWIDGETStyleTarget>());
        };

        TQTWIDGETStyleTarget* makeQTWIDGETStyleTarget()
        {
            return new TQTWIDGETStyleTarget();
        }
    };
}

#endif
