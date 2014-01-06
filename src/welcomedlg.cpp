#include "welcomedlg.h"

/*
 *  Constructs a WelcomeDlg as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
WelcomeDlg::WelcomeDlg( QWidget* parent, const char* name, bool modal, Qt::WFlags fl )
	: QDialog( parent, fl )
{
	setupUi(this);
	setObjectName(name);
	setModal(modal);
}

/*
 *  Destroys the object and frees any allocated resources
 */
WelcomeDlg::~WelcomeDlg()
{
}

#include "welcomedlg.moc"
