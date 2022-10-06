#include "dlgsetupramp.h"
#include "ui_dlgsetupramp.h"

dlgSetupRamp::dlgSetupRamp(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlgSetupRamp)
{
	ui->setupUi(this);
}

dlgSetupRamp::~dlgSetupRamp()
{
	delete ui;
}
