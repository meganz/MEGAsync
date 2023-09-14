#ifndef SYSTEMAPPLICATIONMANAGERFACTORY_H
#define SYSTEMAPPLICATIONMANAGERFACTORY_H

#include <unordered_map>
#include <memory>

#include "ISystemApplicationManager.h"

class IConcreteSystemApplicationManagerFactory
{
    public :
        virtual ISystemApplicationManager* makeSystemApplicationManager() = 0;
};

using tSystemApplicationManagerFactories = std::unordered_map<std::string, std::unique_ptr<IConcreteSystemApplicationManagerFactory>>;

class SystemApplicationManagerFactory
{
public:
    SystemApplicationManagerFactory() = delete;

    static ISystemApplicationManager* getSystemApplicationManager(const QString& systemApplicationId)
    {
        const auto& systemManagerFactories = getFactories();
        auto foundIt = systemManagerFactories.find(systemApplicationId.toStdString());
        if (foundIt != systemManagerFactories.end()) {
            return foundIt->second->makeSystemApplicationManager();
        }
        else {
            return nullptr;
        }
    }

    static bool registerFileManager(const std::string& systemApplicationId, IConcreteSystemApplicationManagerFactory* concreteFileManagerFactory)
    {
        auto& systemManagerFactories = getFactories();
        if (systemManagerFactories.find(systemApplicationId) == systemManagerFactories.end()) {
            auto result = systemManagerFactories.emplace(systemApplicationId, concreteFileManagerFactory);
            return result.second;
        }
        else {
            return false;
        }
    }

    private:
    static tSystemApplicationManagerFactories& getFactories() {
        static tSystemApplicationManagerFactories systemApplicationManagerFactories{};
        return systemApplicationManagerFactories;
    }
};

template<class TSystemApplicationManager>
class ConcreteSystemApplicationManagerFactory : public IConcreteSystemApplicationManagerFactory
{
    public:
        static bool Register(const std::string& systemApplicationFactoryId) {
            return SystemApplicationManagerFactory::registerFileManager(systemApplicationFactoryId, new ConcreteSystemApplicationManagerFactory<TSystemApplicationManager>());
        };

        TSystemApplicationManager* makeSystemApplicationManager() {
            return new TSystemApplicationManager();
        }
};

#endif // SYSTEMAPPLICATIONMANAGERFACTORY_H
