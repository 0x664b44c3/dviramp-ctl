#ifndef DLGSETUPRAMP_H
#define DLGSETUPRAMP_H

#include <QDialog>

namespace Ui {
class dlgSetupRamp;
}

class dlgSetupRamp : public QDialog
{
	Q_OBJECT

public:
	explicit dlgSetupRamp(QWidget *parent = 0);
	~dlgSetupRamp();

private:
	Ui::dlgSetupRamp *ui;
};

#endif // DLGSETUPRAMP_H
