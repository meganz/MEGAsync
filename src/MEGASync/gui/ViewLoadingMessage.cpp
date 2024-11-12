#include "ViewLoadingMessage.h"
#include "ui_ViewLoadingMessage.h"

ViewLoadingMessage::ViewLoadingMessage(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ViewLoadingMessage)
{
    ui->setupUi(this);
}

ViewLoadingMessage::~ViewLoadingMessage()
{
    delete ui;
}
