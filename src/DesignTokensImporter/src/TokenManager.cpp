#include "TokenManager.h"
#include "utilities.h"
#include "PathProvider.h"

#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStringBuilder>

static const QString RELATIVE_TOKENS_PATH = QString::fromLatin1("../DesignTokensImporter/tokens");
static const QString RELATIVE_UI_PATH = QString::fromLatin1("/gui");
static const QString RELATIVE_UI_WIN_PATH = RELATIVE_UI_PATH + QString::fromLatin1("/win");
static const QString RELATIVE_UI_LINUX_PATH = RELATIVE_UI_PATH + QString::fromLatin1("/linux");
static const QString RELATIVE_UI_MAC_PATH = RELATIVE_UI_PATH + QString::fromLatin1("/macx");
static const QString RELATIVE_QRC_MAC_PATH = RELATIVE_UI_PATH + QString::fromLatin1("/Resources_macx.qrc");
static const QString RELATIVE_QRC_WINDOWS_PATH = RELATIVE_UI_PATH + QString::fromLatin1("/Resources_win.qrc");
static const QString RELATIVE_QRC_LINUX_PATH = RELATIVE_UI_PATH + QString::fromLatin1("/Resources_linux.qrc");
static const QString RELATIVE_THEMES_DIR_PATH = RELATIVE_UI_PATH + QString::fromLatin1("/themes");
static const QString RELATIVE_STYLES_DIR_PATH = RELATIVE_UI_PATH + QString::fromLatin1("/themes/styles");
static const QString RELATIVE_CSS_WIN_PATH = RELATIVE_STYLES_DIR_PATH + QString::fromLatin1("/win");
static const QString RELATIVE_CSS_LINUX_PATH = RELATIVE_STYLES_DIR_PATH + QString::fromLatin1("/linux");
static const QString RELATIVE_CSS_MAC_PATH = RELATIVE_STYLES_DIR_PATH + QString::fromLatin1("/macx");

static const QString RELATIVE_GENERATED_PATH = QString::fromLatin1("../DesignTokensImporter/generated");
static const QString RELATIVE_HASHES_PATH = RELATIVE_GENERATED_PATH + QString::fromLatin1("/hashes.json");
static const QString JSON_NAME_FILTER = QString::fromLatin1("*.json");
static const QString UI_NAME_FILTER = QString::fromLatin1("*.ui");
static const QString CSS_NAME_FILTER = QString::fromLatin1("*.css");
static const QString STYLESHEET_EXTENSION = QString::fromLatin1(".css");

//
// SVG Icons
//
static const QString RELATIVE_IMAGES_PATH = RELATIVE_UI_PATH + QString::fromLatin1("/images");
static const QString RELATIVE_SVG_PATH = RELATIVE_IMAGES_PATH + QString::fromLatin1("/svg");

namespace DTI
{
    const QMap<quint32, QString> TokenManager::OBJECT_ID_TO_STRING_MAP = {
        {ObjectNamesID::TOKEN, "token"},
        {ObjectNamesID::GUI_WIN, "gui_win"},
        {ObjectNamesID::GUI_LINUX, "gui_linux"},
        {ObjectNamesID::GUI_MAC, "gui_mac"},
        {ObjectNamesID::CSS_STYLESHEETS, "css_stylesheets"},
    };

    TokenManager::TokenManager()
    {
        QString currentDir = QDir::currentPath();
        qDebug() << "TokenManager::TokenManager - Current working directory = " << currentDir;

        // Set up directory structure
        Utilities::createDirectory(Utilities::resolvePath(currentDir, RELATIVE_TOKENS_PATH));
        Utilities::createDirectory(Utilities::resolvePath(currentDir, RELATIVE_GENERATED_PATH));
        Utilities::createDirectory(currentDir + RELATIVE_THEMES_DIR_PATH);
        Utilities::createDirectory(currentDir + RELATIVE_STYLES_DIR_PATH);
        Utilities::createDirectory(currentDir + RELATIVE_CSS_WIN_PATH);
        Utilities::createDirectory(currentDir + RELATIVE_CSS_LINUX_PATH);
        Utilities::createDirectory(currentDir + RELATIVE_CSS_MAC_PATH);

        // SVG
        Utilities::createDirectory(currentDir + RELATIVE_SVG_PATH);
    }

    TokenManager* TokenManager::instance()
    {
        static TokenManager manager;
        return &manager;
    }

    void TokenManager::run()
    {
        QString currentDir = QDir::currentPath();

        // Load .json token files
        mTokenFilePathsList = Utilities::findFilesInDir(Utilities::resolvePath(currentDir, RELATIVE_TOKENS_PATH), JSON_NAME_FILTER);

        // Stop if there are no Design Token files
        if (mTokenFilePathsList.isEmpty())
        {
            qDebug() << "TokenManager::run - ERROR! No Design Token .JSON files found in folder " << Utilities::resolvePath(currentDir, RELATIVE_TOKENS_PATH);
            return;
        }

        // Create a directory for every theme for every platform
        foreach (const QString& filePath, mTokenFilePathsList)
        {
            QString fileName = Utilities::extractFileNameNoExtension(filePath);
            Utilities::createDirectory(currentDir + RELATIVE_CSS_WIN_PATH + "/" + fileName);
            Utilities::createDirectory(currentDir + RELATIVE_CSS_LINUX_PATH + "/" + fileName);
            Utilities::createDirectory(currentDir + RELATIVE_CSS_MAC_PATH + "/" + fileName);
        }

        // Load gui .ui files
        mWinUIFilePathsList = Utilities::findFilesInDir(currentDir + RELATIVE_UI_WIN_PATH, UI_NAME_FILTER);
        mLinuxUIFilePathsList = Utilities::findFilesInDir(currentDir + RELATIVE_UI_LINUX_PATH, UI_NAME_FILTER);
        mMacUIFilePathsList = Utilities::findFilesInDir(currentDir + RELATIVE_UI_MAC_PATH, UI_NAME_FILTER);

        // Load .css files
        mCSSFiles = Utilities::findFilesInDir(currentDir + RELATIVE_STYLES_DIR_PATH, CSS_NAME_FILTER, true);

        // Parse Hash file
        auto parsedHashMap = readHashFile();

        if (parsedHashMap.isEmpty())
        {
            qDebug() << "TokenManager::didTokenUIOrCSSFilesChange - Unable to read Hash file";
        }
        else if (!didTokenUIOrCSSFilesChange(parsedHashMap))
        {
            qDebug() << "TokenManager::run - Hash, Design Token, UI and CSS Stylesheet files did not change; Nothing to do here...";
            return;
        }

        // Parse .json token files and create colour map
        FilePathColourMap fileToColourMap = parseTokenJSON(mTokenFilePathsList);

        // Save colourMaps .JSON files
        for (auto it = fileToColourMap.constBegin(); it != fileToColourMap.constEnd(); ++it)
        {
            const QString& filePath = it.key();
            const ColourMap& colourMap = it.value();
            QString fileName = Utilities::extractFileName(filePath);
            Utilities::writeColourMapToJSON(colourMap, Utilities::resolvePath(currentDir, RELATIVE_GENERATED_PATH) + "/" + fileName);
        }

        // Parse .ui class (xml) files and store the ones with Design Tokens
        mWinUIClasses.clear();
        mLinuxUIClasses.clear();
        mMacUIClasses.clear();
        parseUiFiles(mWinUIFilePathsList, mWinUIClasses);
        parseUiFiles(mLinuxUIFilePathsList, mLinuxUIClasses);
        parseUiFiles(mMacUIFilePathsList, mMacUIClasses);

        //Add svg images to svg.qrc resource file
        QString qrcPath = currentDir + PathProvider::RELATIVE_SVG_QRC_PATH;
        if (!Utilities::createNewQrcFile(qrcPath))
        {
            qDebug() << "Failed to create QRC file. " << qrcPath;
        }

        QString directoryPath = currentDir + PathProvider::RELATIVE_GENERATED_SVG_DIR_PATH;
        QStringList filters = {PathProvider::SVG_NAME_FILTER};
        QStringList filePaths;
        Utilities::traverseDirectory(directoryPath, filters, filePaths);

        if (!Utilities::addToResourcesBatch(filePaths, qrcPath, directoryPath))
        {
            qDebug() << "Failed to add resources to QRC file." << qrcPath;
        }

        QString priFilePath = currentDir + PathProvider::RELATIVE_GUI_PRI_PATH;
        QString qrcRelativePath = QFileInfo(priFilePath).absoluteDir()
                                      .relativeFilePath(currentDir + PathProvider::RELATIVE_SVG_QRC_PATH);
        if (!Utilities::includeQrcInPriFile(priFilePath, qrcRelativePath))
        {
            qDebug() << "Failed to include svg.qrc in gui.pri";
        }

        //Insert newly created qrc file path in CMakeLists.txt file
        QString cMakeListsFileDirPath = Utilities::resolvePath(currentDir, PathProvider::RELATIVE_CMAKE_FILE_LIST_DIR_PATH);
        if (!Utilities::insertQRCPathInCMakeListsFile(cMakeListsFileDirPath, PathProvider::RELATIVE_SVG_QRC_PATH))
        {
            qDebug() << "Failed to insert qrc path in CMakeLists.txt file";
        }


        QStringList winCSSFiles;
        QStringList linuxCSSFiles;
        QStringList macCSSFiles;

        // Generate .css stylesheets for all platforms
        for (auto it = fileToColourMap.constBegin(); it != fileToColourMap.constEnd(); ++it)
        {
            const QString& filePath = it.key();
            const ColourMap& colourMap = it.value();
            QString directory = Utilities::extractFileNameNoExtension(filePath);

            winCSSFiles << generateStylesheets(colourMap,
                                              mWinUIClasses,
                                              currentDir + RELATIVE_CSS_WIN_PATH + "/" + directory);
            linuxCSSFiles << generateStylesheets(colourMap,
                                                mLinuxUIClasses,
                                                currentDir + RELATIVE_CSS_LINUX_PATH + "/" + directory);
            macCSSFiles << generateStylesheets(colourMap,
                                              mMacUIClasses,
                                              currentDir + RELATIVE_CSS_MAC_PATH + "/" + directory);
        }

        //Generate ApplicationStyles Json File For Windows
        winCSSFiles << generateWinApplicationStyleJsonFile();

        // Add .css files to .qrc files
        // Note: css path must be relative to the /gui folder
        QString uiDir = currentDir + RELATIVE_UI_PATH + "/";

        auto processCSSFiles =
            [&](const QStringList& cssFiles, const QString& relativeQRCPath)
        {
            for (const QString& css : cssFiles)
            {
                QString strippedCSS = css;
                strippedCSS.remove(0, uiDir.length());
                Utilities::addToResources(strippedCSS, currentDir + relativeQRCPath);
            }
        };

        processCSSFiles(winCSSFiles, RELATIVE_QRC_WINDOWS_PATH);
        processCSSFiles(linuxCSSFiles, RELATIVE_QRC_LINUX_PATH);
        processCSSFiles(macCSSFiles, RELATIVE_QRC_MAC_PATH);

        writeHashFile();
    }

    bool TokenManager::didTokenUIOrCSSFilesChange(const QMap<QString, QMap<QString, QString>>& parsedHashMap)
    {
        const auto mapObjectNameToID = [](const QString& objectName) -> TokenManager::ObjectNamesID
        {
            for (auto it = OBJECT_ID_TO_STRING_MAP.cbegin(); it != OBJECT_ID_TO_STRING_MAP.cend(); ++it)
            {
                if (it.value() == objectName)
                {
                    return static_cast<TokenManager::ObjectNamesID>(it.key());
                }
            }
            return TokenManager::INVALID_ID;
        };

        for (auto outerIt = parsedHashMap.cbegin(); outerIt != parsedHashMap.cend(); ++outerIt)
        {
            const QString& objectName = outerIt.key();
            const QMap<QString, QString>& hashMap = outerIt.value();

            // Get the list of files for this objectName
            TokenManager::ObjectNamesID myObjectID = mapObjectNameToID(objectName);

            // Check if all files with a hash are still present
            // If they are, then check if any of them changed
            // If they are not, then that means that that file has since been deleted
            switch (myObjectID)
            {
            case TokenManager::TOKEN:
            {
                if (didFilesChange(mTokenFilePathsList, hashMap))
                {
                    qDebug() << "TokenManager::didTokenUIOrCSSFilesChange - Token file change detected!";
                    return true;
                }

                qDebug() << "TokenManager::didTokenUIOrCSSFilesChange - Token files are identical...";
            }
                break;
            case TokenManager::GUI_WIN:
            {
                if (didFilesChange(mWinUIFilePathsList, hashMap))
                {
                    qDebug() << "TokenManager::didTokenUIOrCSSFilesChange - Windows UI file change detected!";
                    return true;
                }

                qDebug() << "TokenManager::didTokenUIOrCSSFilesChange - Windows UI files are identical...";
            }
                break;
            case TokenManager::GUI_LINUX:
            {
                if (didFilesChange(mLinuxUIFilePathsList, hashMap))
                {
                    qDebug() << "TokenManager::didTokenUIOrCSSFilesChange - Linux UI file change detected!";
                    return true;
                }

                qDebug() << "TokenManager::didTokenUIOrCSSFilesChange - Linux UI files are identical...";
            }
                break;
            case TokenManager::GUI_MAC:
            {
                if (didFilesChange(mMacUIFilePathsList, hashMap))
                {
                    qDebug() << "TokenManager::didTokenUIOrCSSFilesChange - MAC UI file change detected!";
                    return true;
                }

                qDebug() << "TokenManager::didTokenUIOrCSSFilesChange - MAC UI files are identical...";
            }
                break;
            case TokenManager::CSS_STYLESHEETS:
            {
                if (didFilesChange(mCSSFiles, hashMap))
                {
                    qDebug() << "TokenManager::didTokenUIOrCSSFilesChange - CSS file change detected!";
                    return true;
                }

                qDebug() << "TokenManager::didTokenUIOrCSSFilesChange - CSS files are identical...";
            }
                break;
            case TokenManager::INVALID_ID:
            default:
            {
                qDebug() << "TokenManager::didTokenUIOrCSSFilesChange - ERROR! Object not found or Unknown object";
                return true;
            }
            }
        }

        return false;
    }

    bool TokenManager::didFilesChange(const QStringList& filePathList,
                                      const QMap<QString, QString>& hashMap)
    {
        QStringList hashFilePathList = hashMap.keys();

        // Check if all files with a hash are still present
        // If they are, then check if any of them changed
        // If they are not, then that means that that file has since been deleted
        return (!Utilities::areAllStringsPresent(hashFilePathList, filePathList) ||
                !areHashesMatching(filePathList, hashMap));
    }

    //!
    //! \brief TokenManager::areHashesMatching
    //! \param filePathList: List of full paths to files
    //! \param hashMap: Map with [filePath][hash] as key/value pairs
    //! \returns true, if the current hashes for all files in @filePathList match with the hashes in @hashMap
    //! \returns false otherwise
    //!
    bool TokenManager::areHashesMatching(const QStringList& filePathList,
                                        const QMap<QString, QString>& hashMap)
    {
        if (filePathList.isEmpty() || hashMap.isEmpty())
        {
            return false;
        }

        foreach (const QString& filePath, filePathList)
        {
            // Check if the QString is present as a key in hashMap
            if (!hashMap.contains(filePath))
            {
                // File is not present, so was likely newly added
                return false;
            }

            // The file is present in the hashMap:
            // Check if the hash (and therefore the file itself) has changed:
            if (Utilities::getFileHash(filePath) != hashMap.value(filePath))
            {
                return false;
            }

            //! For debug purposes: print path and corresponding hash
            //!qDebug() << "[PATH]:" << filePath << ", [HASH]:" << hashMap.value(filePath);
        }

        return true;
    }

    QMap<QString, QMap<QString, QString>> TokenManager::readHashFile()
    {
        return Utilities::readHashesJSONFile(Utilities::resolvePath(QDir::currentPath(), RELATIVE_HASHES_PATH));
    }

    bool TokenManager::writeHashFile()
    {
        QList<QStringList> listOfStringLists;
        listOfStringLists.append(mTokenFilePathsList);
        listOfStringLists.append(mWinUIFilePathsList);
        listOfStringLists.append(mLinuxUIFilePathsList);
        listOfStringLists.append(mMacUIFilePathsList);
        listOfStringLists.append(mCSSFiles);

        return Utilities::writeHashesJsonFile(listOfStringLists,
                                              TokenManager::OBJECT_ID_TO_STRING_MAP.values(),
                                              Utilities::resolvePath(QDir::currentPath(), RELATIVE_HASHES_PATH));
    }


    FilePathColourMap TokenManager::parseTokenJSON(const QStringList& tokenFilePathsList)
    {
        FilePathColourMap retMap;

        if (tokenFilePathsList.isEmpty()) { return retMap; }

        foreach (const QString& filePath, tokenFilePathsList)
        {
            ColourMap colourMap =  Utilities::parseTokenJSON(filePath);
            retMap.insert(filePath, colourMap);
        }

        return retMap;
    }

    void TokenManager::parseUiFiles(const QStringList& uiFilePathsList,
                                    QVector<QSharedPointer<UIClass>>& UIClasses)
    {
        if (uiFilePathsList.isEmpty()) { return; }

        // Check if the vector contains a specific QSharedPointer
        auto containsShared = [&UIClasses](const QSharedPointer<UIClass>& item) -> bool {
            return std::any_of(UIClasses.begin(), UIClasses.end(), [&item](const QSharedPointer<UIClass>& ptr) {
                return ptr.data() == item.data();
            });
        };

        foreach (const QString& filePath, uiFilePathsList)
        {
            QSharedPointer<UIClass> sharedUIClass = QSharedPointer<UIClass>::create(filePath);

            // Add UIClass if it has tokens; prevent duplicates
            if (sharedUIClass->containsTokens() && !containsShared(sharedUIClass))
            {
                UIClasses.append(sharedUIClass);
            }
        }
    }

    QStringList TokenManager::generateStylesheets(const ColourMap& colourMap,
                                                  const QVector<QSharedPointer<UIClass>>& uiClassList,
                                                  const QString& saveDirectory)
    {
        QStringList cssPaths;

        if ((colourMap.isEmpty()) || (uiClassList.isEmpty())) { return cssPaths; }

        for (const auto &uiClass : uiClassList)
        {
            if (!uiClass) { continue; }

            QString styleSheet = uiClass->getStyleSheet(colourMap, saveDirectory);
            QString fileName = Utilities::extractFileNameNoExtension(uiClass->getFilePath()) + STYLESHEET_EXTENSION;
            QString fullPath = saveDirectory + "/" + fileName;

            if(Utilities::writeStyleSheetToFile(styleSheet, fullPath))
            {
                cssPaths.append(fullPath);
            }
        }

        return cssPaths;
    }

    QStringList TokenManager::generateWinApplicationStyleJsonFile()
    {
        QStringList cssPaths;

        const QString resourcePathDTI = ":/StyleSheet";
        const QString applicationStylesJsonFileName = "ApplicationStyles.json";
        const QString winDirectoryPath = QDir::currentPath() % RELATIVE_CSS_WIN_PATH;

        //Copy the Application Style Json File to the MegaSync App for global styling.
        for (const QString& filePath : qAsConst(mTokenFilePathsList))
        {
            // Get the Application Style Json File based on Theme
            Utilities::Theme theme =  Utilities::getTheme(filePath);
            const QString themeDirName = Utilities::themeToString(theme);
            const QString sourcePath = resourcePathDTI  % QDir::separator() % themeDirName % QDir::separator() % applicationStylesJsonFileName;
            const QString sourceResourcePath = QDir::fromNativeSeparators(sourcePath);

            // Destination Path in MegaSync Application for storing Application Style Json File
            const QString tokenDirName = Utilities::extractFileNameNoExtension(filePath);
            const QString winTokenDirectoryPath = winDirectoryPath  % QDir::separator() % tokenDirName;
            const QString destinationPath = winTokenDirectoryPath % QDir::separator() % applicationStylesJsonFileName;
            const QString destinationResourcePath = QDir::fromNativeSeparators(destinationPath);

            // Check if the resource file exists and is readable.
            QFile resourceFile(sourceResourcePath);
            if (!resourceFile.exists() || !resourceFile.open(QIODevice::ReadOnly))
            {
                qWarning() << "Resource file not found or not accessible: " << sourceResourcePath;
            }

            // Ensure the target directory exists.
            QDir().mkpath(QDir::fromNativeSeparators(winTokenDirectoryPath));

            // Check and delete the existing file at the destination path if it exists.
            QFile destinationFile(destinationResourcePath);
            destinationFile.setPermissions(QFile::WriteOwner | QFile::ReadOwner |
                                           QFile::WriteUser | QFile::ReadUser);
            if (destinationFile.exists() )
            {
                if(!destinationFile.remove())
                {
                    qWarning() << "Failed to delete the existing file at:" << destinationResourcePath;
                }
            }

            if (!QFile::copy(sourceResourcePath, destinationResourcePath))
            {
                qWarning() << "Failed to copy ApplicationStyles.json from " << sourceResourcePath << " to " << destinationResourcePath;

            } else
            {
                cssPaths.append(destinationResourcePath);
            }
        }
        return cssPaths;
    }

} // namespace DTI
