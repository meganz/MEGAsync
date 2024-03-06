#include "utilities.h"
#include "PathProvider.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QDir>
#include <QColor>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QStringList>
#include <QTextStream>
#include <QTemporaryFile>

static const QString COLOUR_TOKEN_START = QString::fromLatin1("--color-");
static const qint64 FILE_READ_BUFFER_SIZE = 8192;

namespace DTI
{
//!
//! \brief Utilities::createDirectory
//! \param dirPath: path to directory
//! \returns true if directory @dirPath was created successfully OR if it already existed
//! \returns false otherwise
//!
bool Utilities::createDirectory(const QString& dirPath)
{
    QDir directory(dirPath);

    if (directory.exists())
    {
        return true;
    }

    if (directory.mkpath("."))
    {
        qDebug() << "Utilities::createDirectory - Directory created successfully:" << dirPath;
        return true;
    }

    qDebug() << "Utilities::createDirectory - ERROR! Failed to create directory:" << dirPath;
    return false;
}

//!
//! \brief Utilities::findFilesInDir
//! \param dirPath: path to directory containing .json files
//! \returns list of full paths to all .json files in @dirPath
//!
QStringList Utilities::findFilesInDir(const QString& dirPath,
                                      const QString& nameFilter,
                                      bool findInSubfolders)
{
    QStringList files;
    QDir directory(dirPath);

    if (!directory.exists())
    {
        qWarning("Utilities::findFilesInDir - Directory does not exist: %s", qPrintable(dirPath));
        return files;
    }

    QStringList nameFilters;
    nameFilters << nameFilter;

    QStringList fileList = directory.entryList(nameFilters, QDir::Files | QDir::NoDotAndDotDot);

    for (const QString &file : fileList)
    {
        QString filePath = directory.filePath(file);
        if (isFileValid(filePath))
        {
            files.append(filePath);
        }
    }

    // Check subfolder if necessary
    if (findInSubfolders)
    {
        QStringList subdirectories = directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &subdir : subdirectories)
        {
            QString subdirectoryPath = directory.filePath(subdir);
            QStringList subdirectoryFiles = findFilesInDir(subdirectoryPath, nameFilter, true);
            files.append(subdirectoryFiles);
        }
    }

    return files;
}

//!
//! \brief Utilities::isFileValid
//! \param path: path to file
//! \returns true if @path is not empty AND if the file exists AND is not empty
//! \returns false otherwise
//!
bool Utilities::isFileValid(const QString& path)
{
    // Check if the file path is not empty
    if (path.isEmpty())
    {
        qDebug() << "Utilities::isFileValid - File path is empty.";
        return false;
    }

    // Create a QFileInfo object to check if the file exists
    QFileInfo fileInfo(path);

    // Check if the file exists and is not empty
    if (!fileInfo.exists() || fileInfo.size() == 0)
    {
        qDebug() << "Utilities::isFileValid - File does not exist or is empty: " << path;
        return false;
    }

    return true;
}

QMap<QString, QString> Utilities::parseTokenJSON(const QString& path)
{
    QMap<QString, QString> colourMap;

    if (!isFileValid(path))
    {
        return colourMap;
    }

    auto stringToColor = [](const QString& str) -> QColor {
        // Static regular expression to match RGBA color format
        static const QRegularExpression regex("^rgba\\((?<red>\\d+),\\s*(?<green>\\d+),\\s*(?<blue>\\d+),\\s*(?<alpha>\\d+(\\.\\d+)?)\\)$");

        QRegularExpressionMatch match = regex.match(str);

        // Check if the string matches the expected format
        if (match.hasMatch())
        {
            // Extract color components from the named capture groups
            int red = match.captured("red").toInt();
            int green = match.captured("green").toInt();
            int blue = match.captured("blue").toInt();
            int alpha = static_cast<int>(match.captured("alpha").toDouble() * 255);

            // Create and return a QColor
            return QColor(red, green, blue, alpha);
        }

        // Format does not match, return an invalid QColor
        return QColor();
    };

    QFile inputFile(path);

    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Utilities::convertJSON - Error opening input file " << path;
        return colourMap;
    }

    // Parse the input JSON document
    QJsonDocument jsonDocument = QJsonDocument::fromJson(inputFile.readAll());
    inputFile.close();

    if (jsonDocument.isNull())
    {
        qDebug() << "Utilities::convertJSON - Error parsing JSON document";
        return colourMap;
    }

    // Process the JSON document and convert RGBA values to HEX
    QJsonObject jsonObject = jsonDocument.object();

    // Use an index-based loop to avoid detachment warning
    const QStringList categoryKeys = jsonObject.keys();
    for (int i = 0; i < categoryKeys.size(); ++i)
    {
        const QString& category = categoryKeys[i];
        QJsonObject categoryObject = jsonObject.value(category).toObject();

        // Use an index-based loop to avoid detachment warning
        const QStringList tokenKeys = categoryObject.keys();
        for (int i = 0; i < tokenKeys.size(); ++i)
        {
            const QString& token = tokenKeys[i];

            QJsonObject tokenObject = categoryObject[token].toObject();
            QJsonValue typeValue = tokenObject["$type"];
            QJsonValue valueValue = tokenObject["$value"];

            if (typeValue.isString() && valueValue.isString())
            {
                QString type = typeValue.toString();
                QString value = valueValue.toString();

                if (type == "color")
                {
                    QColor color = stringToColor(value);
                    // Strip "--color-" from beginning of token
                    colourMap.insert(token.mid(COLOUR_TOKEN_START.size()),
                                     color.name(QColor::HexArgb));
                }
            }
        }
    }

    return colourMap;
}

bool Utilities::writeColourMapToJSON(const QMap<QString, QString>& colourMap,
                                     const QString& filePath)
{
    QJsonObject dataObject;

    // Iterate over the QMap and add key-value pairs to the JSON object
    for (auto it = colourMap.begin(); it != colourMap.end(); ++it)
    {
        dataObject.insert(it.key(), it.value());
    }

    QJsonObject jsonObject;
    jsonObject.insert("data", dataObject);

    return writeJSONToFile(QJsonDocument(jsonObject), filePath);
}

bool Utilities::createNewQrcFile(const QString &qrcPath)
{
    QFile qrcFile(qrcPath);

    if (qrcFile.exists()) {
        qDebug() << "QRC file already exists:" << qrcPath;
        return true;
    }

    if (!qrcFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Unable to open file for writing:" << qrcPath;
        return false;
    }

    QTextStream stream(&qrcFile);
    stream << "<RCC>\n";
    stream << "    <qresource prefix=\"/\">\n";
    stream << "    </qresource>\n";
    stream << "</RCC>\n";

    qrcFile.close();
    qDebug() << "QRC file created successfully at" << qrcPath;
    return true;
}

void Utilities::traverseDirectory(const QString &directoryPath, const QStringList &filters, QStringList &filePaths)
{
    QDir dir(directoryPath);
    foreach (const QFileInfo& fileInfo, dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot))
    {
        if (fileInfo.isDir())
        {
            traverseDirectory(fileInfo.absoluteFilePath(), filters, filePaths);
        } else
        {
            for (const QString& filter : filters)
            {
                QString modifiedFilter = filter;
                modifiedFilter.remove('*');

                if (fileInfo.fileName().endsWith(modifiedFilter, Qt::CaseInsensitive))
                {
                    filePaths << fileInfo.absoluteFilePath();
                    break;
                }
            }
        }
    }
}

//!
//! \brief Utilities::addToResources
//! \param filePath Path to file that needs to be added to .qrc
//! \param qrcPath Path to .qrc file
//! \returns true if path is successfully added to the .qrc file
//! OR if path was already present in that file
//! \returns false otherwise
//!
bool Utilities::addToResources(const QString& filePath,
                               const QString& qrcPath)
{
    QFile qrcFile(qrcPath);

    // Precondition check: qrcFile must exist and be writable
    if (!qrcFile.exists() || !qrcFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        return false;
    }

    QTextStream stream(&qrcFile);
    QString qrcContent = stream.readAll();

    // Precondition check: path must not already be added to the resources
    if (qrcContent.contains(filePath))
    {
        qDebug() << "Utilities::addToResources - " << filePath << " is already added to " << qrcPath;
        return true;
    }

    // Find the position to insert the new file entry
    QRegularExpression re(QString("(\\s*)") + QRegularExpression::escape("</qresource>"));
    auto match = re.match(qrcContent);

    if (!match.isValid())
    {
        // Failed to find the correct position to insert the new file entry
        return false;
    }

    // Insert the new file entry before </qresource>
    QString newFileEntry = match.captured(1) + "    <file>" + filePath + "</file>";
    qrcContent.insert(match.capturedStart(), newFileEntry);

    // Write the modified content back to the .qrc file
    qrcFile.resize(0);
    stream << qrcContent;

    qrcFile.close();

    qDebug() << "Utilities::addToResources - Successfully added " << filePath << " to " << qrcPath;

    return true;
}

bool Utilities::addToResourcesBatch(const QStringList &filePaths, const QString &qrcPath, const QString &directoryPath)
{
    QFile qrcFile(qrcPath);
    if (!qrcFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        return false;
    }

    QTextStream stream(&qrcFile);
    QString qrcContent = stream.readAll();
    qrcFile.close();

    bool modified = false;
    QDir baseDir(QFileInfo(qrcPath).absolutePath());
    for (const QString& filePath : filePaths)
    {
        QString relativePath = baseDir.relativeFilePath(filePath);

        if (!qrcContent.contains(relativePath))
        {
            QRegularExpression re(QString("(\\s*)") + QRegularExpression::escape("</qresource>"));
            auto match = re.match(qrcContent);
            if (!match.isValid())
            {
                return false;
            }
            QString newFileEntry = match.captured(1) + "    <file>" + relativePath + "</file>";
            qrcContent.insert(match.capturedStart(), newFileEntry);
            modified = true;
        }
    }

    if (modified)
    {
        if (!qrcFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            return false;
        }
        stream << qrcContent;
        qrcFile.close();
    }

    return true;
}

bool Utilities::includeQrcInPriFile(const QString &priFilePath, const QString &qrcRelativePath)
{
    QFile priFile(priFilePath);
    if (!priFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Unable to open PRI file for reading:" << priFilePath;
        return false;
    }

    bool lineExists = false;
    QString content;
    QTextStream in(&priFile);
    QString lineToInclude = "RESOURCES += $$PWD/" + qrcRelativePath;

    while (!in.atEnd())
    {
        QString line = in.readLine();
        if (line.trimmed() == lineToInclude.trimmed())
        {
            lineExists = true;
        }
        content += line + "\n";
    }
    priFile.close();

    if (!lineExists)
    {
        if (!priFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qDebug() << "Unable to open PRI file for writing:" << priFilePath;
            return false;
        }
        QTextStream out(&priFile);
        out << content << lineToInclude << "\n";
        priFile.close();
    }

    return true;
}

bool Utilities::insertQRCPathInCMakeListsFile(const QString &fileDirPath, const QString &newQRCPath)
{
    QString fileName = "CMakeLists.txt";
    QDir dir(fileDirPath);
    QString filePath = dir.filePath(fileName);

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Failed to open the CMakeListsText file for reading:" << filePath;
        return false;
    }

    QStringList lines;
    QTextStream in(&file);
    while (!in.atEnd())
    {
        lines.append(in.readLine());
    }
    file.close();

    // Check if newQRCPath already exists in the file
    QString lineToInclude = "${MEGAsyncDir}" + newQRCPath;
    for (const QString &line : lines)
    {
        if (line.contains(lineToInclude))
        {
            qInfo() << "The QRC path already exists in the CMakeLists.txt file:" << lineToInclude;
            return true;
        }
    }

    // Determine where to insert qrc path in CMakeListsText file
    QRegularExpression regexSetSrcs(R"(set\s*\(\s*SRCS)", QRegularExpression::CaseInsensitiveOption);
    int insertPosition =  calculateQRCPathInsertPosition(lines, regexSetSrcs);

    // Create and Open a CMakeLists.txt temporary file for writing
    QString tempFilePattern = QDir(fileDirPath).absoluteFilePath("XXXXXXCMakeLists.txt");
    QTemporaryFile tempFile(tempFilePattern);
    tempFile.setAutoRemove(false);

    if (!tempFile.open())
    {
        qWarning() << "Failed to open a created CMakeListsText temporary file for writing.";
        return false;
    }


    // Copy content to the CMakeLists.txt temporary file with the qrc path inserted at the determined position
    QString lineEnding = "\n";
    int tabWidth = 4;
    QString fourSpaces = QString(tabWidth, ' ');
    QTextStream out(&tempFile);

    for (int i = 0; i < lines.size(); ++i)
    {
        if (i == insertPosition)
        {
            out << fourSpaces << lineToInclude << lineEnding;
        }
        out << lines[i] << lineEnding;
    }

    // Handle the case where the insert position is at the end of the file
    if (insertPosition == lines.size())
    {
        out  << fourSpaces << lineToInclude << lineEnding;
    }

    // Replace the original CMakeLists.txt file with the temporary CMakeListsText file
    tempFile.close();
    if (!QFile::remove(filePath))
    {
        qWarning() << "Failed to remove the original CMakeLists.txt file  file:" << filePath;
        return false;
    }
    if (!tempFile.rename(filePath))
    {
        qWarning() << "Failed to rename the temporary created CMakeLists.txt file to the original CMakeLists.txt file location:" << filePath;

        // Fallback: Manually copy the temporary file's content to the new CMakeLists.txt
        QFile::copy(tempFile.fileName(), filePath); // Attempt to copy directly as a fallback
        return false;
    }

    qDebug() << "Successfully updated CMakeLists.txt with new qrc path in location: " << filePath;
    return true;
}

int Utilities::calculateQRCPathInsertPosition(const QStringList &lines, const QRegularExpression &sectionStartRegex)
{
    bool inSectionBlock = false;
    int lastQRCIndex = -1, firstCPPAfterQRCIndex = -1, insertPosition = -1;

    for (int i = 0; i < lines.size(); ++i)
    {
        const QString& line = lines.at(i);
        QRegularExpressionMatch match = sectionStartRegex.match(line);
        if (match.hasMatch())
        {
            inSectionBlock = true;
        }
        else if (inSectionBlock && line.contains(')'))
        {
            break;
        }
        else if (inSectionBlock)
        {
            if (line.contains(".qrc"))
            {
                lastQRCIndex = i;
                firstCPPAfterQRCIndex = -1;
            }
            else if (line.contains(".cpp") && firstCPPAfterQRCIndex == -1 && lastQRCIndex != -1)
            {
                insertPosition = lastQRCIndex + 1;
            }
        }
    }

    return insertPosition;
}

//!
//! \brief FileHandler::extractFileName
//! \param file_path
//! \returns the file name extracted from @file_path
//!
QString Utilities::extractFileName(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.fileName();
}

//!
//! \brief FileHandler::extractFileNameNoExtension
//! \param file_path
//! \returns the file name extracted from @file_path, without the extension
//!
QString Utilities::extractFileNameNoExtension(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.baseName();
}


//!
//! \brief Utilities::getFileHash
//! \param filePath
//! \returns the hash of file @filePath
//! \returns an empty string if file @filePath does not exist or cannot be opened
//!
QString Utilities::getFileHash(const QString& filePath)
{
    QFile file(filePath);

    // Verify precondition: the file must exist and be readable
    if (!QFile::exists(filePath) || (!file.open(QIODevice::ReadOnly)))
    {
        // Opening failed
        return QString::fromLatin1("");
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    QByteArray buffer;

    while (!file.atEnd())
    {
        buffer = file.read(FILE_READ_BUFFER_SIZE);
        hash.addData(buffer);
    }

    file.close();

    QByteArray resultHash = hash.result();
    QString hashString = QString::fromUtf8(resultHash.toHex());

    return hashString;
}

QMap<QString, QMap<QString, QString>> Utilities::readHashesJSONFile(const QString& filePath)
{
    QMap<QString, QMap<QString, QString>> result;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Utilities::readHashesJSONFile - Error opening file" << filePath;
        return result;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (!jsonDoc.isObject())
    {
        qDebug() << "Utilities::readHashesJSONFile - ERROR! Invalid JSON format";
        return result;
    }

    QJsonObject rootObj = jsonDoc.object();


    for (auto it = rootObj.begin(); it != rootObj.end(); ++it)
    {
        QString objectName = it.key();
        QJsonObject innerObj = it.value().toObject();
        QMap<QString, QString> fileToHashMap;

        for (auto innerIt = innerObj.begin(); innerIt != innerObj.end(); ++innerIt)
        {
            fileToHashMap.insert(innerIt.key(), innerIt.value().toString());
        }

        result.insert(objectName, fileToHashMap);
    }

    return result;
}

QMap<QString, QString> Utilities::readColourMapJSONFile(const QString &filePath)
{
    QMap<QString, QString> colourMap;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Utilities::readColourMapJSONFile - Error opening file" << filePath;
        return colourMap;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (!jsonDoc.isObject()) {
        qDebug() << "Utilities::readColourMapJSONFile - ERROR! Invalid JSON format";
        return colourMap;
    }

    QJsonObject rootObj = jsonDoc.object();
    QJsonObject dataObj = rootObj.value("data").toObject();

    for (auto it = dataObj.begin(); it != dataObj.end(); ++it) {
        colourMap.insert(it.key(), it.value().toString());
    }

    return colourMap;
}

bool Utilities::writeHashesJsonFile(const QList<QStringList> &filePaths,
                                    const QStringList &jsonObjectNames,
                                    const QString &outputFilePath)
{
    // Check if the number of filePaths and names is the same
    if (filePaths.size() != jsonObjectNames.size())
    {
        qDebug() << "Utilities::writeHashesJsonFile - ERROR: The number of filePaths and JSON object names must be the same.";
        return false;
    }

    // Verify that all files exist
    for (const QStringList& pathList : filePaths)
    {
        for (const QString& filePath : pathList)
        {
            if (!isFileValid(filePath))
            {
                qDebug() << "Utilities::writeHashesJsonFile - ERROR: Invalid filePath: " << filePath;
                return false;
            }
        }
    }

    // Create a JSON object
    QJsonObject mainJsonObject;

    // Iterate over each set of filePaths and values
    for (int i = 0; i < filePaths.size(); ++i)
    {
        QJsonObject jsonObject;

        // Add data to the JSON object
        for (const QString& filePath : filePaths[i])
        {
            jsonObject[filePath] = getFileHash(filePath);
        }

        // Add the JSON object to the main JSON object with the specified name
        mainJsonObject[jsonObjectNames[i]] = jsonObject;
    }

    return writeJSONToFile(QJsonDocument(mainJsonObject), outputFilePath);
}

QJsonObject Utilities::createWidgetStyleSheet(const QString& objectName,
                                              const QMap<QString, QString>& properties)
{
    QJsonObject widget;
    widget["objectName"] = objectName;

    QStringList propertiesValues;
    for (auto it = properties.constBegin(); it != properties.constEnd(); ++it)
    {
        propertiesValues << QString("%1: %2").arg(it.key(), it.value());
    }

    widget["styleSheet"] = QString("* {\n  %1;\n}\n").arg(propertiesValues.join(";\n  "));

    return widget;
}

//!
//! \brief Utilities::writeJSONToFile
//! \param jsonDoc JSON content
//! \param filePath Full path where the file needs to be written to
//! \returns true if file @filePath was written successfully
//! \returns false otherwise
//!
bool Utilities::writeJSONToFile(const QJsonDocument& jsonDoc,
                                const QString& filePath)
{
    // Open the file for writing
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Utilities::writeJSONToFile - ERROR: Could not open file " << filePath << " for writing";
        return false;
    }

    // Check if the write operation is successful
    if (file.write(jsonDoc.toJson()) == -1)
    {
        qDebug() << "Utilities::writeJSONToFile - ERROR: Failed to write JSON to file " << filePath;
        file.close();
        return false;
    }

    file.close();

    qDebug() << "Utilities::writeJSONToFile - JSON file " << filePath << " was written successfully!";
    return true;
}

//!
//! \brief Utilities::getSubStringBetweenMarkers
//! \param str Input string
//! \param startMarker Start delimiter
//! \param stopMarker Stop delimiter
//! \returns substring from @str between @startMarker and @stopMarker
//!
//! Example, if str = "/*token_background-color: {{surface-1}};*/"
//!             startMarker = "{{"
//!             stopMarker = "}}"
//! then: "surface-1" is returned
//!
QString Utilities::getSubStringBetweenMarkers(const QString& str,
                                              const QString& startMarker,
                                              const QString& stopMarker)
{
    QRegularExpression regex(startMarker + "(.*?)" + stopMarker);
    QRegularExpressionMatch match = regex.match(str);

    return match.hasMatch() ? match.captured(1) : QString();
};

//!
//! \brief Utilities::areAllStringsPresent
//! \param list1 First list of strings
//! \param list2 Second list of strings
//! \returns true if all strings from @list1 are present in @list2
//! \returns false otherwise
//!
bool Utilities::areAllStringsPresent(const QStringList& list1,
                                     const QStringList& list2)
{
    // Convert the lists to sets for efficient comparison
    QSet<QString> set1(list1.begin(), list1.end());
    QSet<QString> set2(list2.begin(), list2.end());

    // Check if all strings from list1 are present in list2
    return (set1 - set2).isEmpty();
}


QMap<Utilities::Theme, QMap<QString, QString>> Utilities::getColourMapInfo()
{
    static QMap<Utilities::Theme, QMap<QString, QString>> cachedColourMap;
    static bool isCached = false;

    if (!isCached)
    {
        QString currentDir = QDir::currentPath();
        QDir dir(resolvePath(currentDir, PathProvider::RELATIVE_GENERATED_PATH));

        QStringList tokenFilePathsList = Utilities::findFilesInDir(resolvePath(currentDir, PathProvider::RELATIVE_TOKENS_PATH), PathProvider::JSON_NAME_FILTER);

        QStringList filePathsColourMap;
        foreach (const QString& filePath, tokenFilePathsList)
        {
            QString fileName = Utilities::extractFileName(filePath);
            filePathsColourMap.append(dir.filePath(fileName));
        }

        foreach (const QString& filePath, filePathsColourMap)
        {
            QMap<QString, QString> colourMap = Utilities::readColourMapJSONFile(filePath);
            cachedColourMap.insert(Utilities::getTheme(filePath), colourMap);
        }
        isCached = true;
    }

    return cachedColourMap;
}

Utilities::Theme Utilities::getTheme(const QString& filePath)
{
    if (filePath.contains("semantic_tokens_dark_tokens.json"))
    {
        return Theme::DARK;
    } else if (filePath.contains("semantic_tokens_light_tokens.json"))
    {
        return Theme::LIGHT;
    } else
    {
        return Theme::LIGHT;  //Default
    }
}

QString Utilities::themeToString(Theme theme)
{
    static const QMap<Utilities::Theme, QLatin1String> themeMap = {
        {Theme::LIGHT, QLatin1String("Light")},
        {Theme::DARK, QLatin1String("Dark")}
    };

    return themeMap.value(theme, QLatin1String("Light"));
}

QString Utilities::resolvePath(const QString &basePath, const QString &relativePath)
{
    QString combinedPath = QDir::cleanPath(basePath + '/' + relativePath);
    QString resolvedAbsolutePath = QDir(combinedPath).absolutePath();

    return resolvedAbsolutePath;
}

QString Utilities::targetToString(Targets target)
{
    static const QMap<Targets, QLatin1String> targetsMap = {
        {Targets::ColorStyle, QLatin1String("ColorStyle")},
        {Targets::ImageStyle, QLatin1String("ImageStyle")}
    };

    return targetsMap.value(target);
}

bool Utilities::writeStyleSheetToFile(const QString& css, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file for writing:" << filePath;
        return false;
    }

    QTextStream out(&file);
    out << css;
    file.close();

    return true;
}

} // namespace DTI

