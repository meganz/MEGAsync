#include "QTWIDGETStyleGenerator.h"

#include "Utilities.h"
#include "PathProvider.h"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStringBuilder>

using namespace DTI;

const QMap<QTWIDGETStyleGenerator::ObjectNamesID, QString> QTWIDGETStyleGenerator::OBJECT_ID_TO_STRING_MAP = {
    {ObjectNamesID::TOKEN, "token"},
    {ObjectNamesID::GUI_WIN, "gui_win"},
    {ObjectNamesID::GUI_LINUX, "gui_linux"},
    {ObjectNamesID::GUI_MAC, "gui_mac"},
    {ObjectNamesID::CSS_STYLESHEETS, "css_stylesheets"},
    };

QTWIDGETStyleGenerator::QTWIDGETStyleGenerator(QObject* parent)
    : QObject{parent}
{
    mCurrentDir = QDir::currentPath();
}

void QTWIDGETStyleGenerator::start(const ThemedColourMap& fileToColourMap)
{
    initialize(fileToColourMap);

    if (fileToColourMap.isEmpty() || !checkInitialFileConditions())
    {
        return;
    }

    createDirectories();

    parseAndStoreUIDesignTokens();

    generateStyleSheet(fileToColourMap);

    addStyleSheetToResource();

    writeHashFile();
}

void QTWIDGETStyleGenerator::initialize(const ThemedColourMap& fileToColourMap)
{
    // Load gui .ui files
    mWinUIFilePathsList = Utilities::findFilesInDir(mCurrentDir + PathProvider::RELATIVE_UI_WIN_PATH, PathProvider::UI_NAME_FILTER);
    mLinuxUIFilePathsList = Utilities::findFilesInDir(mCurrentDir + PathProvider::RELATIVE_UI_LINUX_PATH, PathProvider::UI_NAME_FILTER);
    mMacUIFilePathsList = Utilities::findFilesInDir(mCurrentDir + PathProvider::RELATIVE_UI_MAC_PATH, PathProvider::UI_NAME_FILTER);

    // Load .css files
    mCSSFiles = Utilities::findFilesInDir(mCurrentDir + PathProvider::RELATIVE_THEMES_DIR_PATH, PathProvider::CSS_NAME_FILTER, true);

    //Append list of css theming file paths
    for (auto it = fileToColourMap.cbegin(); it != fileToColourMap.cend(); ++it)
    {
        mCSSThemeFilePathsList.append(it.key().toLower());
    }
}

void QTWIDGETStyleGenerator::createDirectories()
{
    // Set up directory structure
    Utilities::createDirectory(Utilities::resolvePath(mCurrentDir, PathProvider::RELATIVE_GENERATED_PATH));
    Utilities::createDirectory(Utilities::resolvePath(mCurrentDir, PathProvider::RELATIVE_THEMES_DIR_PATH));
    Utilities::createDirectory(mCurrentDir + PathProvider::RELATIVE_CSS_WIN_PATH);
    Utilities::createDirectory(mCurrentDir + PathProvider::RELATIVE_CSS_LINUX_PATH);
    Utilities::createDirectory(mCurrentDir + PathProvider::RELATIVE_CSS_MAC_PATH);

    // Create a directory for every theme for every platform
    foreach (const QString& filePath, mCSSThemeDirectoryNames)
    {
        Utilities::createDirectory(mCurrentDir + PathProvider::RELATIVE_CSS_WIN_PATH + "/" + filePath);
        Utilities::createDirectory(mCurrentDir + PathProvider::RELATIVE_CSS_LINUX_PATH + "/" + filePath);
        Utilities::createDirectory(mCurrentDir + PathProvider::RELATIVE_CSS_MAC_PATH + "/" + filePath);
    }
}

void QTWIDGETStyleGenerator::parseAndStoreUIDesignTokens()
{
    // Parse .ui class (xml) files and store the ones with Design Tokens
    mWinDesignTokenUIs.clear();
    mLinuxDesignTokenUIs.clear();
    mMacDesignTokenUIs.clear();

    parseUiFiles(mWinUIFilePathsList, mWinDesignTokenUIs);
    parseUiFiles(mLinuxUIFilePathsList, mLinuxDesignTokenUIs);
    parseUiFiles(mMacUIFilePathsList, mMacDesignTokenUIs);
}

void QTWIDGETStyleGenerator::parseUiFiles(const QStringList& uiFilePathsList,
                                          QVector<QSharedPointer<UIHandler>>& UIClasses)
{
    if (uiFilePathsList.isEmpty())
    {
        return;
    }

    // Check if the vector contains a specific QSharedPointer
    auto containsShared = [&UIClasses](const QSharedPointer<UIHandler>& item) -> bool {
        return std::any_of(UIClasses.begin(), UIClasses.end(), [&item](const QSharedPointer<UIHandler>& ptr) {
            return ptr.data() == item.data();
        });
    };

    foreach (const QString& filePath, uiFilePathsList)
    {
        QSharedPointer<UIHandler> sharedUIClass = QSharedPointer<UIHandler>::create(filePath);

        // Add UIHandler if it has tokens; prevent duplicates
        if (sharedUIClass->containsTokens() && !containsShared(sharedUIClass))
        {
            UIClasses.append(sharedUIClass);
        }
    }
}

bool QTWIDGETStyleGenerator::checkInitialFileConditions()
{
    // Check Token UI File or CSS File has changed
    // Parse Hash file
    auto parsedHashMap = readHashFile();
    if (parsedHashMap.isEmpty())
    {
        qDebug() << "Unable to read Hash file";
    }
    else if (!didTokenUIOrCSSFilesChange(parsedHashMap))
    {
        qDebug() << "QTWIDGETStyleGenerator::checkInitialFileConditions - Hash, Design Token, UI and CSS Stylesheet files did not change; Nothing to do here...";
        return false;
    }

    return true;
}

QMap<QString, QMap<QString, QString>> QTWIDGETStyleGenerator::readHashFile()
{
    return Utilities::readHashesJSONFile(Utilities::resolvePath(mCurrentDir, PathProvider::RELATIVE_HASHES_PATH));
}

bool QTWIDGETStyleGenerator::didTokenUIOrCSSFilesChange(const QMap<QString, QMap<QString, QString>>& parsedHashMap)
{

    const auto mapObjectNameToID = [](const QString& objectName) -> QTWIDGETStyleGenerator::ObjectNamesID
    {
        for (auto it = OBJECT_ID_TO_STRING_MAP.cbegin(); it != OBJECT_ID_TO_STRING_MAP.cend(); ++it)
        {
            if (it.value() == objectName)
            {
                return static_cast<QTWIDGETStyleGenerator::ObjectNamesID>(it.key());
            }
        }
        return QTWIDGETStyleGenerator::ObjectNamesID::INVALID_ID;
    };

    for (auto outerIt = parsedHashMap.cbegin(); outerIt != parsedHashMap.cend(); ++outerIt)
    {
        const QString& objectName = outerIt.key();
        const QMap<QString, QString>& hashMap = outerIt.value();

        // Get the list of files for this objectName
        QTWIDGETStyleGenerator::ObjectNamesID myObjectID = mapObjectNameToID(objectName);

        // Check if all files with a hash are still present
        // If they are, then check if any of them changed
        // If they are not, then that means that that file has since been deleted
        switch (myObjectID)
        {
        case QTWIDGETStyleGenerator::ObjectNamesID::TOKEN:
        {
            if (didFilesChange(mCSSThemeDirectoryNames, hashMap))
            {
                qDebug() << "QTWIDGETStyleGenerator::didTokenUIOrCSSFilesChange - Token file change detected!";
                return true;
            }

            qDebug() << "QTWIDGETStyleGenerator::didTokenUIOrCSSFilesChange - Token files are identical...";
            break;
        }
        case QTWIDGETStyleGenerator::ObjectNamesID::GUI_WIN:
        {
            if (didFilesChange(mWinUIFilePathsList, hashMap))
            {
                qDebug() << "QTWIDGETStyleGenerator::didTokenUIOrCSSFilesChange - Windows UI file change detected!";
                return true;
            }

            qDebug() << "QTWIDGETStyleGenerator::didTokenUIOrCSSFilesChange - Windows UI files are identical...";
            break;
        }
        case QTWIDGETStyleGenerator::ObjectNamesID::GUI_LINUX:
        {
            if (didFilesChange(mLinuxUIFilePathsList, hashMap))
            {
                qDebug() << "QTWIDGETStyleGenerator::didTokenUIOrCSSFilesChange - Linux UI file change detected!";
                return true;
            }

            qDebug() << "QTWIDGETStyleGenerator::didTokenUIOrCSSFilesChange - Linux UI files are identical...";
            break;
        }    
        case QTWIDGETStyleGenerator::ObjectNamesID::GUI_MAC:
        {
            if (didFilesChange(mMacUIFilePathsList, hashMap))
            {
                qDebug() << "QTWIDGETStyleGenerator::didTokenUIOrCSSFilesChange - MAC UI file change detected!";
                return true;
            }

            qDebug() << "QTWIDGETStyleGenerator::didTokenUIOrCSSFilesChange - MAC UI files are identical...";
            break;
        }        
        case QTWIDGETStyleGenerator::ObjectNamesID::CSS_STYLESHEETS:
        {
            if (didFilesChange(mCSSFiles, hashMap))
            {
                qDebug() << "QTWIDGETStyleGenerator::didTokenUIOrCSSFilesChange - CSS file change detected!";
                return true;
            }

            qDebug() << "QTWIDGETStyleGenerator::didTokenUIOrCSSFilesChange - CSS files are identical...";
            break;
        }      
        case QTWIDGETStyleGenerator::ObjectNamesID::INVALID_ID:
        default:
        {
            qDebug() << "QTWIDGETStyleGenerator::didTokenUIOrCSSFilesChange - ERROR! Object not found or Unknown object";
            return true;
        }
        }
    }

    return false;
}

bool QTWIDGETStyleGenerator::didFilesChange(const QStringList& filePathList, const QMap<QString, QString>& hashMap)
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
bool QTWIDGETStyleGenerator::areHashesMatching(const QStringList& filePathList, const QMap<QString, QString>& hashMap)
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

void QTWIDGETStyleGenerator::generateStyleSheet(const ThemedColourMap& fileToColourMap)
{
    // Generate .css stylesheets for all platforms
    for (auto it = fileToColourMap.constBegin(); it != fileToColourMap.constEnd(); ++it)
    {
        const QString& themeName = it.key();
        const ColourMap& colourMap = it.value();
        QString cssDirectoryName = themeName.toLower();

        mWinCSSFiles << generateStylesheets(colourMap,
                                            mWinDesignTokenUIs,
                                            mCurrentDir + PathProvider::RELATIVE_CSS_WIN_PATH + "/" + cssDirectoryName);
        mLinuxCSSFiles << generateStylesheets(colourMap,
                                              mLinuxDesignTokenUIs,
                                              mCurrentDir + PathProvider::RELATIVE_CSS_LINUX_PATH + "/" + cssDirectoryName);
        mMacCSSFiles << generateStylesheets(colourMap,
                                            mMacDesignTokenUIs,
                                            mCurrentDir + PathProvider::RELATIVE_CSS_MAC_PATH + "/" + cssDirectoryName);
    }
}

QStringList QTWIDGETStyleGenerator::generateStylesheets(const ColourMap& colourMap,
                                                          const QVector<QSharedPointer<UIHandler>>& uiClassList,
                                                          const QString& saveDirectory)
{
    QStringList cssPaths;

    if ((colourMap.isEmpty()) || (uiClassList.isEmpty())) { return cssPaths; }

    for (const auto &uiClass : uiClassList)
    {
        if (!uiClass)
        {
            continue;
        }

        QString styleSheet = uiClass->getStyleSheet(colourMap, saveDirectory);
        QString fileName = Utilities::extractFileNameNoExtension(uiClass->getFilePath()) + PathProvider::CSS_FILE_EXTENSION;
        QString fullPath = saveDirectory + "/" + fileName;

        if(Utilities::writeStyleSheetToFile(styleSheet, fullPath))
        {
            cssPaths.append(fullPath);
        }
    }

    return cssPaths;
}

void QTWIDGETStyleGenerator::addStyleSheetToResource()
{
    // Add .css files to .qrc files
    // Note: css path must be relative to the /gui folder
    QString uiDir = mCurrentDir + PathProvider::RELATIVE_UI_PATH + "/";

    auto processCSSFiles =
        [&](const QStringList& cssFiles, const QString& relativeQRCPath)
    {
        for (const QString& css : cssFiles)
        {
            QString strippedCSS = css;
            strippedCSS.remove(0, uiDir.length());
            Utilities::addToResources(strippedCSS, mCurrentDir + relativeQRCPath);
        }
    };

    processCSSFiles(mWinCSSFiles, PathProvider::RELATIVE_QRC_WINDOWS_PATH);
    processCSSFiles(mLinuxCSSFiles, PathProvider::RELATIVE_QRC_LINUX_PATH);
    processCSSFiles(mMacCSSFiles, PathProvider::RELATIVE_QRC_MAC_PATH);
}

bool QTWIDGETStyleGenerator::writeHashFile()
{
    QList<QStringList> listOfStringLists;
    listOfStringLists.append(mCSSThemeDirectoryNames);
    listOfStringLists.append(mWinUIFilePathsList);
    listOfStringLists.append(mLinuxUIFilePathsList);
    listOfStringLists.append(mMacUIFilePathsList);
    listOfStringLists.append(mCSSFiles);

    return Utilities::writeHashesJsonFile(listOfStringLists,
                                          QTWIDGETStyleGenerator::OBJECT_ID_TO_STRING_MAP.values(),
                                          Utilities::resolvePath(mCurrentDir, PathProvider::RELATIVE_HASHES_PATH));
}

