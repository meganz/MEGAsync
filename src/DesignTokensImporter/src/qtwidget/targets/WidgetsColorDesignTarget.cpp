#include "WidgetsColorDesignTarget.h"

#include "WidgetsDesignTargetFactory.h"

#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QTextStream>
#include <QStringBuilder>

using namespace DTI;

bool WidgetsColorDesignTarget::registered = WidgetsDesignFactory<WidgetsColorDesignTarget>::Register("widgetsColorTarget");

void WidgetsColorDesignTarget::process(const ThemedColorData& themedColorData)
{
    // TODO : convert to json files with color themed data.
}



