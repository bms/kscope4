/****************************************************************************
** Form interface generated from reading ui file './welcomedlg.ui'
**
** Created: Tue Feb 23 11:07:43 2010
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.8   edited Jan 11 14:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef WELCOMEDLG_H
#define WELCOMEDLG_H

#include "ui_welcomedlg.h"

class WelcomeDlg : public QDialog, Ui::WelcomeDlg
{
    Q_OBJECT

public:
    WelcomeDlg( QWidget* parent = 0, const char* name = 0, bool modal = FALSE,
		Qt::WFlags fl = 0 );
    ~WelcomeDlg();
};

#endif // WELCOMEDLG_H
