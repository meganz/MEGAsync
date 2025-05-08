#ifndef DESIGN_TOKEN_IMPORTER_H
#define DESIGN_TOKEN_IMPORTER_H

#include <QString>

namespace DTI
{
namespace DesignTokensImporter
{
bool initialize(const QString& megaSyncPath = QString(),
                const QString& designTokensFilePath = QString());
void run();
}
}

#endif
