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

#include <QAction>
#include <QDebug>
#include <QMenu>

#include <ktextbrowser.h>
#include <kcombobox.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "makedlg.h"
#include "makefrontend.h"
#include "queryview.h"

/**
 * Class constructor.
 * @param	pParent	The parent widget
 * @param	szName	The widget's name
 */
MakeDlg::MakeDlg(QWidget* pParent, const char* szName) :
	QWidget(pParent, Qt::WindowTitleHint),
	m_pConf(NULL),
	m_pMake(NULL),
	m_pHistoryMenu(NULL)
{
	setupUi(this);
	setObjectName(szName);

	// Don't show the "Function" column
	m_pErrorView->setColumnWidth(0, 0);

	// Create a new make front-end
	m_pMake = new MakeFrontend();
	connect(m_pMake, SIGNAL(dataReady(FrontendToken*)),
		this,SLOT(slotShowOutput(FrontendToken*)));
	connect(m_pMake, SIGNAL(finished(uint)), this, SLOT(slotFinished(uint)));
	connect(m_pMake, SIGNAL(error(const QString&, const QString&, const QString&)),
		this, SLOT(slotAddError(const QString&, const QString&, const QString&)));

	// The Root URL control should browse directories
	m_pRootURL->setMode(KFile::Directory);

	// Handle URL links in the browser
	m_pOutputBrowser->setNotifyClick(true);
	connect(m_pOutputBrowser, SIGNAL(urlClick(const QString&)), this,
		SLOT(slotBrowserClicked(const QString&)));

	// Handle selections in the error view
	connect(m_pErrorView, SIGNAL(lineRequested(const QString& , uint)), this,
		SIGNAL(fileRequested(const QString&, uint)));

	m_pMakeCommand->setCompletionMode(KGlobalSettings::CompletionPopup);
	m_pMakeCommand->setToolTip("<p style='white-space:pre'><qt>Right click to popup the history containing all commands<br/>entered during the current session.<br/>Click on the desired item to enter it into the command line</qt>");

	connect(m_pMakeCommand, SIGNAL(customContextMenuRequested(const QPoint&)),
		this, SLOT(slotContextMenuRequested(const QPoint&)));

	// Create the (permanent) history menu. The first item (index 0) is the
	// "Clear history" action
	QAction *pAct;

	m_pHistoryMenu = new QMenu("Make Command History", this);
	pAct = m_pHistoryMenu->addAction("Clear history");
	pAct->setData(QVariant(0));
	m_pHistoryMenu->addSeparator();

	connect(m_pHistoryMenu, SIGNAL(triggered(QAction *)),
		this, SLOT(slotHistoryItemClicked(QAction *)));
}

/**
 * Class destructor.
 */
MakeDlg::~ MakeDlg()
{
	saveCmdHistory();
	delete m_pMake;
}

/**
 * Restore command history from a previous session
 */
void MakeDlg::restoreCmdHistory(const QString& sPath)
{
	if (m_pConf == (KSharedConfigPtr)NULL) {
		m_pConf = KSharedConfig::openConfig(sPath + "/cscope.proj");
	}

	int i = 0;
	QString mkCmd;
	KConfigGroup confGrp = m_pConf->group("CommandHistory");

	// Commands from the previous session are added to the completion object & to
	// the history menu (text is squeezed if needed)
	while (confGrp.hasKey(QString("HistoryItem%1").arg(i))) {
		mkCmd = confGrp.readEntry(QString("HistoryItem%1").arg(i++), QString());
		addHistoryItem(mkCmd);
	}
}

/**
 * Save current command history
 */
void MakeDlg::saveCmdHistory()
{
    KConfigGroup confGrp = m_pConf->group("CommandHistory");
    QStringList mkCmdHistory = m_pMakeCommand->completionObject()->items();
    QStringListIterator it(mkCmdHistory);
    int i = 0;

    while (it.hasNext()) {
	confGrp.writeEntry(QString("HistoryItem%1").arg(i++), it.next());
    }
    confGrp.sync();
}

/**
 * @return	The currently set make command
 */
QString MakeDlg::getCommand() const
{
	return m_pMakeCommand->text();
}

/**
 * @param	sCmd	The new make command to use
 */
void MakeDlg::setCommand(const QString& sCmd)
{
	m_pMakeCommand->setText(sCmd);
	addHistoryItem(sCmd);
}

/**
 * @param	sCmd	The command to be added to history menu
 */
void MakeDlg::addHistoryItem(const QString& sCmd)
{
	QStringList mkCmdHistory = m_pMakeCommand->completionObject()->items();

	// Do not allow duplicates in the command history.
	// If command is more than 50 chars. long squeeze it manually before installing
	// in history menu KLineEdit object is editable; so automatic text squeezing is
	// not allowed.
	if (! mkCmdHistory.contains(sCmd)) {
		QString sHistItem = sCmd;
		QAction *pAct;
		bool isSqueezed = false;
		int n = mkCmdHistory.size() + 1;

		m_pMakeCommand->completionObject()->addItem(sCmd);
		if (sCmd.size() > 50) {
			isSqueezed = true;
			sHistItem = sCmd.left(30) + " ... " + sCmd.right(15);
		}
		pAct = m_pHistoryMenu->addAction(sHistItem);

		// `n' is the index in completion object of the text being installed
		// starting with 1 (a negative value means `text was squeezed')
		pAct->setData(QVariant(isSqueezed ? (-n) : n));
	}
}

/**
 * @return	The directory in which to run the make command
 */
QString MakeDlg::getDir() const
{
	return m_pRootURL->url().pathOrUrl();
}

/**
 * @param	sURL	The new root directory to use
 */
void MakeDlg::setDir(const QString& sURL)
{
	m_pRootURL->setUrl(sURL);
}

/**
 * Overrides the default close behaviour.
 * Makes sure that a window is not closed while a make process is running,
 * unless the user explicitly requests it. In this case, the make process
 * is killed.
 * @param	pEvent	The close event descriptor
 */
void MakeDlg::closeEvent(QCloseEvent* pEvent)
{
	// Check if a process is currently running
	if (m_pMake->state() == QProcess::Running) {
		// Prompt the user
		switch (KMessageBox::questionYesNoCancel(this, 
			i18n("A make process is running. Would you like to stop it first?"),
			i18n("Close Make Window"))) {
		case KMessageBox::Yes:
			// Stop the process first
			m_pMake->kill();
			break;

		case KMessageBox::No:
			// Do nothing
			break;

		case KMessageBox::Cancel:
			// Abort closing
			pEvent->ignore();
			return;
		}
	}

	QWidget::closeEvent(pEvent);
}

void MakeDlg::slotContextMenuRequested(const QPoint& pt)
{
	m_pHistoryMenu->popup(m_pMakeCommand->mapToGlobal(pt));
}

/**
 * Retrieve a command from the item (i.e. action) that was clicked in the history menu.
 * The action's text may have been manually squeezed: `unsqueeze' it using the index in
 * completion object associated to the KLineEdit
 */
void MakeDlg::slotHistoryItemClicked(QAction *pAct)
{
    QString txt = pAct->text();
    QString makeCmd;
    QStringList mkCmdHistory = m_pMakeCommand->completionObject()->items();

#ifndef NDEBUG
    qDebug() << QString("MakeDialog::slotHistoryItemClicked: clicked on \"%1\"").arg(txt);
#endif

    int n = pAct->data().toInt();

    // First action in the menu is `Clear the history'; others are real commands
    // Clearing the history implies destroying the "CommandHistory" group in config
    if (n == 0) {
	m_pConf->group("CommandHistory").deleteGroup();
	m_pMakeCommand->clear();
	m_pMakeCommand->completionObject()->clear();
	return;
    }

    // `-1' because of the `Clear' action 
    makeCmd = mkCmdHistory[((n < 0) ? (-n) : n) - 1];

#ifndef NDEBUG
    qDebug() << QString("MakeDialog::slotHistoryItemClicked: retrieved %1").arg(makeCmd);
#endif

    m_pMakeCommand->setText(makeCmd);
}

/**
 * Starts a make process using the user-supplied command.
 * This slot is connected to the returnPressed(const QString) signal of the "KLineEdit"
 * widget.
 */
void MakeDlg::slotMake(const QString& sCommand)
{
	// Clear the current contents
	int rowCount = m_pErrorView->model()->rowCount();
	m_pOutputBrowser->clear();
	m_pErrorView->model()->removeRows(0, rowCount);

	// Run the command entered or selected in history menu using a `shell process'
	// See `setUseShell()' in makefrontend.cpp
	if (!m_pMake->run("sh",
			  (sCommand.isEmpty() ? QStringList() : sCommand.split(" ", QString::SkipEmptyParts)),
			  m_pRootURL->url().pathOrUrl())){
		KMessageBox::error(this, m_pMake->getRunError());
		return;
	}

	// Install command text in KCompletion object & in history menu
	addHistoryItem(sCommand);

	// Disable the make button
	m_pMakeButton->setEnabled(false);
	m_pStopButton->setEnabled(true);
}

/**
 * Starts a make process using the user-supplied command.
 * This slot is connected to the clicked() signal of the "Make" button.
 */
void MakeDlg::slotMake()
{
	QString mkCmd = m_pMakeCommand->text();

	slotMake(mkCmd);
}

/**
 * Terminates the current make process.
 * This slot is connected to the clicked() signal of the stop button.
 */
void MakeDlg::slotStop()
{
	m_pMake->kill();
}

/**
 * Displays the parsed output, as generated by the MakeFrontend object.
 * This slot is connected to the dataReady() signal of the make front-end.
 * @param	pToken	Holds the parsed data
 */
void MakeDlg::slotShowOutput(FrontendToken* pToken)
{
	m_pOutputBrowser->append(pToken->getData());
}

/**
 * Displays the results of the make command.
 * This slot is connected to the finished() signal of the make front-end.
 */
void MakeDlg::slotFinished(uint)
{
	// Add "Success" or "Error" at the end of the output
	if (m_pMake->exitStatus() == 0) {
		m_pOutputBrowser->append("<font color=\"#008000\"><b>Success</b>"
			"</font>");
	}
	else {
		m_pOutputBrowser->append("<font color=\"#ff0000\"><b>Error</b></font>");
	}

	// Re-enable the "Make" button
	m_pMakeButton->setEnabled(true);
	m_pStopButton->setEnabled(false);
}

/**
 * Emits the fileRequested() signal when a browser link is clicked.
 * This slot is connected to the urlClick() signal of the browser.
 * @param	sURL	The requested URL
 */
void MakeDlg::slotBrowserClicked(const QString& sURL)
{
	QString sFile;
	QString sLine;

	// Exract the file name and the line number from the URL
	sFile = sURL.section('&', 0, 0);
	sLine = sURL.section('&', 1, 1);

	// Add root path for relative paths
	if (!sFile.startsWith("/"))
	  sFile = m_pRootURL->url().pathOrUrl() + "/" + sFile;

	// Emit the signal
	emit fileRequested(sFile, sLine.toUInt());
}

void MakeDlg::slotAddError(const QString& sFile, const QString& sLine,
	const QString& sText)
{
	m_pErrorView->addRecord("", sFile, sLine, sText);
}

#include "makedlg.moc"

/*
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
