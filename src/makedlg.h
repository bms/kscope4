/***************************************************************************
 *
 * Copyright (C) 2006 Elad Lahav (elad_lahav@users.sourceforge.net)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ***************************************************************************/

#ifndef MAKEDLG_H
#define MAKEDLG_H

#include <KSharedConfig>

#include "ui_makelayout.h"

class QAction;
class QMenu;

class MakeFrontend;
class FrontendToken;

/**
 * A window that displays the output of make-like commands.
 * The window contains a text browser showing errors as links to source
 * locations.
 * The make process is determined by a user-specified command, and is run in
 * a user-specified directory. Controls are provided for modifying these values.
 * @author Elad Lahav
 */
class MakeDlg: public QWidget, public Ui::MakeLayout
{
	Q_OBJECT

public:
	MakeDlg(QWidget* pParent = NULL, const char* szName = NULL);
	virtual ~MakeDlg();

	QString getCommand() const;
	void setCommand(const QString&);
	QString getDir() const;
	void setDir(const QString&);

	/**
	 * Restore command history from a previous sesssion. Arg is the absolute path
	 * to the current project directory */
	void restoreCmdHistory(const QString&);

public slots:
	virtual void slotMake();
	virtual void slotMake(const QString&);

signals:
	void fileRequested(const QString&, uint);

protected:
	virtual void closeEvent(QCloseEvent*);

protected slots:
	virtual void slotStop();
	void slotShowOutput(FrontendToken*);
	void slotFinished(uint);
	void slotBrowserClicked(const QString&);
	void slotAddError(const QString&, const QString&, const QString&);

private slots:
	void slotContextMenuRequested(const QPoint&);
	void slotHistoryItemClicked(QAction *);

private:
	/** Install the given command in history menu; avoid duplicates */
	void addHistoryItem(const QString&);

	/** Save current cmd. history (it may have been cleared during this session) */
	void saveCmdHistory();

	/** Private copy of project's pointer to config object */
	KSharedConfigPtr m_pConf;

	/** Handles the make process. */
	MakeFrontend* m_pMake;

	/** Menu to hold make commands history for current sesion */
	QMenu *m_pHistoryMenu;
};

#endif

/*
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
