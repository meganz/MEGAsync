#include "Utilities.h"
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

namespace DTI
{
static const QString COLOUR_TOKEN_START = QString::fromLatin1("color-");
static const qint64 FILE_READ_BUFFER_SIZE = 8192;

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
        qDebug() << __func__ << " Directory created successfully: " << dirPath;
        return true;
    }

    qDebug() << __func__ << " ERROR! Failed to create directory: " << dirPath;
    return false;
}

//!
//! \brief Utilities::findFilesInDir
//! \param dirPath: path to directory containing .json files
//! \returns list of full paths to all .json files in @dirPath
//!
QStringList Utilities::findFilesInDir(const QString& dirPath, const QString& nameFilter, bool findInSubfolders)
{
    QStringList files;
    QDir directory(dirPath);

    if (!directory.exists())
    {
        qWarning() << __func__ << " Directory does not exist: " << qPrintable(dirPath);
        return files;
    }

    QStringList nameFilters;
    nameFilters << nameFilter;

    QStringList fileList = directory.entryList(nameFilters, QDir::Files | QDir::NoDotAndDotDot);

    for (const QString& file : fileList)
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
        for (const QString& subdir : subdirectories)
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
        qDebug() << __func__ << " File path is empty.";
        return false;
    }

    // Create a QFileInfo object to check if the file exists
    QFileInfo fileInfo(path);

    // Check if the file exists and is not empty
    if (!fileInfo.exists() || fileInfo.size() == 0)
    {
        qDebug() << __func__ << " File does not exist or is empty: " << path;
        return false;
    }

    return true;
}

QMap<QString, QString> Utilities::parseColorTheme(const QJsonObject& jsonThemeObject, const CoreData& coreMap)
{
    QMap<QString, QString> colourMap;

    const QStringList categoryKeys = jsonThemeObject.keys();
    for (int index = 0; index < categoryKeys.size(); ++index)
    {
        const QString& category = categoryKeys[index];
        QJsonObject categoryObject = jsonThemeObject.value(category).toObject();

        const QStringList tokenKeys = categoryObject.keys();
        for (int index = 0; index < tokenKeys.size(); ++index)
        {
            const QString& token = tokenKeys[index];

            QJsonObject tokenObject = categoryObject[token].toObject();
            QJsonValue jType = tokenObject["type"];
            QJsonValue jValue = tokenObject["value"];
            //QJsonValue jAlpha = tokenObject["alpha"];

            if (!jType.isNull() && !jValue.isNull()) // && !jAlpha.isNull())
            {
                QString type = jType.toString();

                if (type == "color")
                {
                    QString value = jValue.toString();
                    value.remove("{").remove("}");
                    //float alpha = jAlpha.toString().toFloat();

                    if (coreMap.contains(value))
                    {
                        QString coreColor = coreMap[value];
                        //QString alphaString = QString::number(static_cast<uint>(alpha * 255), 16).rightJustified(2, '0');
                        //QString color = "#" + alphaString + coreColor;
                        QString color = "#" + coreColor;

                        // Strip "--color-" or "-color-" from beginning of token
                        int indexPrefix = token.indexOf(COLOUR_TOKEN_START);
                        if (indexPrefix != -1)
                        {
                            colourMap.insert(token.mid(indexPrefix + COLOUR_TOKEN_START.size()), color);
                        }
                    }
                    else
                    {
                        qDebug() << __func__ << " Core map doesn't contain the color id " << value;
                    }
                }
            }
        }
    }

    return colourMap;
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

Utilities::Theme Utilities::getTheme(const QString& filePath)
{
    Theme theme = Theme::LIGHT;

    if (filePath.contains("Dark"))
    {
        theme = Theme::DARK;
    }

    return theme;
}

QString Utilities::themeToString(Theme theme)
{
    static const QMap<Utilities::Theme, QLatin1String> themeMap = {
        {Theme::LIGHT, QLatin1String("Light")},
        {Theme::DARK, QLatin1String("Dark")}
    };

    return themeMap.value(theme, QLatin1String("Light"));
}

QString Utilities::resolvePath(const QString& basePath, const QString& relativePath)
{
    QString combinedPath = QDir::cleanPath(basePath + '/' + relativePath);
    QString resolvedAbsolutePath = QDir(combinedPath).absolutePath();

    return resolvedAbsolutePath;
}

QString Utilities::targetToString(Targets target)
{
    static const QMap<Targets, QLatin1String> targetsMap = {
        {Targets::ColorStyle, QLatin1String("ColorStyle")}
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

