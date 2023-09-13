#ifndef FILEMANAGERFACTORY_H
#define FILEMANAGERFACTORY_H

#include <unordered_map>
#include <memory>

#include "IFileManager.h"

class IConcreteFileManagerFactory
{
    public :
        virtual IFileManager* makeFileManager() = 0;
};

using tFileManagerParamFactories = std::unordered_map<std::string, std::unique_ptr<IConcreteFileManagerFactory>>;

class FileManagerFactory
{
public:
    FileManagerFactory() = delete;

    static IFileManager* getFileManager(const QString& fileManagerId)
    {
        const auto& fileManagerFactories = getFactories();
        auto foundIt = fileManagerFactories.find(fileManagerId.toStdString());
        if (foundIt != fileManagerFactories.end()) {
            return foundIt->second->makeFileManager();
        }
        else {
            return nullptr;
        }
    }

    static bool registerFileManager(const std::string& fileManagerId, IConcreteFileManagerFactory* concreteFileManagerFactory)
    {
        auto& fileManagerFactories = getFactories();
        if (fileManagerFactories.find(fileManagerId) == fileManagerFactories.end()) {
            auto result = fileManagerFactories.emplace(fileManagerId, concreteFileManagerFactory);
            return result.second;
        }
        else {
            return false;
        }
    }

    private:
    static tFileManagerParamFactories& getFactories() {
        static tFileManagerParamFactories fileManagerFactories{};
        return fileManagerFactories;
    }
};

template<class TFileManager>
class ConcreteFileManagerFactory : public IConcreteFileManagerFactory
{
    public:
        static bool Register(const std::string& factory) {
            return FileManagerFactory::registerFileManager(factory, new ConcreteFileManagerFactory<TFileManager>());
        };

        TFileManager* makeFileManager() {
            return new TFileManager();
        }
};

#endif // FILEMANAGERFACTORY_H
