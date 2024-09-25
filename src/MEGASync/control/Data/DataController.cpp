#include "DataController.h"

DataController::DataController() {}

void DataController::updateModel(int row, QVector<int> roles)
{
    emit dataChanged(row, roles);
}
