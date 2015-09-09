/***************************************************************************
 *
 * Copyright (C) 2005 Elad Lahav (elad_lahav@users.sourceforge.net)
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

#include <qfile.h>
#include <qtoolbutton.h>
#include <qbuttongroup.h>
#include <qstackedwidget.h>
#include <QCloseEvent>

#include <klocale.h>
#include <kfiledialog.h>
#include <kurl.h>

#include "calltreedlg.h"
#include "graphwidget.h"
#include "treewidget.h"
#include "kscopepixmaps.h"
#include "kscopeconfig.h"
#include "graphprefdlg.h"

/** The currently supported version of saved call-tree files. */
#define FILE_VERSION		5

/** File Name index for the file name generation */
int CallTreeDlg::s_nFileNameIndex = 0;

/**
 * Class constructor.
 * @param	pParent	The parent widget
 * @param	szName	The widget's name
 */
CallTreeDlg::CallTreeDlg(QWidget* pParent, const char* szName) :
	QWidget(pParent, Qt::WindowTitleHint)
{
	setupUi( this );
	setObjectName(szName);
	setAttribute(Qt::WA_DeleteOnClose);

	// Setup Graph & Tree button group.
	m_pGraphButtonGrp = new QButtonGroup(this);
	m_pGraphButtonGrp->addButton(m_pGraphButton, CallTreeDlg::GraphView);
	m_pGraphButtonGrp->addButton(m_pCalledButton, CallTreeDlg::CalledTree);
	m_pGraphButtonGrp->addButton(m_pCallingButton, CallTreeDlg::CallingTree);

	connect(m_pGraphButtonGrp, SIGNAL(buttonClicked(int)),
		this, SLOT(slotViewChanged(int)));
	connect(m_pGraphButtonGrp, SIGNAL(buttonClicked(int)),
		m_pStack, SLOT(setCurrentIndex(int)));

	// Set button pixmaps
	m_pCalledButton->setIcon(static_cast<QIcon>(GET_PIXMAP(CalledTree)));
	m_pCallingButton->setIcon(static_cast<QIcon>(GET_PIXMAP(CallingTree)));
	m_pGraphButton->setIcon(static_cast<QIcon>(GET_PIXMAP(CallGraph)));
	m_pSaveButton->setIcon(static_cast<QIcon>(GET_PIXMAP(ButtonSaveAs)));
	m_pZoomInButton->setIcon(static_cast<QIcon>(GET_PIXMAP(ButtonZoomIn)));
	m_pZoomOutButton->setIcon(static_cast<QIcon>(GET_PIXMAP(ButtonZoomOut)));
	m_pRotateButton->setIcon(static_cast<QIcon>(GET_PIXMAP(ButtonRotate)));
	m_pPrefButton->setIcon(static_cast<QIcon>(GET_PIXMAP(ButtonPref)));

	// Open the location of a call
	connect(m_pGraphWidget, SIGNAL(lineRequested(const QString&, uint)),
		this, SIGNAL(lineRequested(const QString&, uint)));
	connect(m_pCalledWidget, SIGNAL(lineRequested(const QString&, uint)),
		this, SIGNAL(lineRequested(const QString&, uint)));
	connect(m_pCallingWidget, SIGNAL(lineRequested(const QString&, uint)),
		this, SIGNAL(lineRequested(const QString&, uint)));

	m_pCallingWidget->setMode(TreeWidget::Calling);

	// Get the default view from KScope's configuration
	m_nDefView = Config().getDefGraphView();
}

/**
 * Class destructor.
 */
CallTreeDlg::~CallTreeDlg()
{
}

/**
 * @param	sFunc	The function to use as the root of the call tree
 */
void CallTreeDlg::setRoot(const QString& sFunc)
{
	m_sRoot = sFunc;

	// Generate unique file name to save call tree later
	m_sFileName = sFunc;
	m_sFileName.replace(' ', '_');
	m_sFileName += QString::number(++s_nFileNameIndex);

	// Set the root item in all views
	m_pGraphWidget->setRoot(sFunc);
	m_pCalledWidget->setRoot(sFunc);
	m_pCallingWidget->setRoot(sFunc);
}

/**
 * Displays the dialogue.
 */
void CallTreeDlg::show()
{
	QLayoutItem* pLayoutItem;
	QToolButton* pButton;

	// Set the default view.
	pLayoutItem = m_pViewGroup->layout()->itemAt(m_nDefView);
	pButton = dynamic_cast<QToolButton*>(pLayoutItem->widget());
	pButton->setChecked(true);

	m_pStack->setCurrentIndex(m_nDefView);
	slotViewChanged(m_nDefView);

	QWidget::show();
}

/**
 * Informs the call tree manager that this object should be removed from the
 * list of call tree dialogues.
 * The close event is received when the dialogue is explicitly closed by the
 * user. This dialogue will not appear when the project is reopened, and it
 * is therefore safe to delete the graph file at this point.
 * @param	pEvent	Information on the closing event 
 */
void CallTreeDlg::closeEvent(QCloseEvent* pEvent)
{
	if (!m_sFilePath.isEmpty())
		QFile::remove(m_sFilePath);

	emit closed(this);
	QWidget::closeEvent(pEvent);
}

extern void yyinit(CallTreeDlg*, FILE*, Encoder*);
extern int yyparse();

/**
 * Restores a call tree from the given call tree file.
 * NOTE: The call tree file is deleted when loading is complete.
 * @param	sProjPath	The full path of the project directory
 * @param	sFileName	The name of the call tree file to load
 * @return	true if successful, false otherwise
 */
bool CallTreeDlg::load(const QString& sProjPath, const QString& sFileName)
{
	QString sPath;
	FILE* pFile;
	int nVersion, nView, nResult;
	Encoder enc;

	// Create the full path name
	sPath = sProjPath + "/" + sFileName;

	// Open the file for reading
	pFile = fopen(sPath.toLatin1(), "r");
	if (pFile == NULL)
		return false;

	// Check file version
	if ((fscanf(pFile, "VERSION=%d\n", &nVersion) != 1) || 
		(nVersion != FILE_VERSION)) {
		fclose(pFile);
		return false;
	}

	// Get default view
	if ((fscanf(pFile, "View=%d\n", &nView) == 1) &&
		(nView >= 0) &&
		(nView <= 2)) {
		m_nDefView = nView;
	}

	// Read the call trees and the graph stored on this file
	//	yyinit(this, pFile, &enc);
	//	nResult = yyparse();
	nResult = 0;

	// Close the file
	fclose(pFile);

	// Check the result returned by the parser
	if (nResult != 0)
		return false;

	// Store the file name
	m_sFileName = sFileName;
	m_sFilePath = sPath;

	// Draw the graph
	m_pGraphWidget->draw();

	return true;
}

/**
 * Writes the contents of the call tree dialog to a call tree file.
 * This method is called for call trees before the owner project is
 * closed.
 * @param	sProjPath	The full path of the project directory
 */
void CallTreeDlg::store(const QString& sProjPath)
{
	QString sPath;
	FILE* pFile;

	// Create the full file path
	sPath = sProjPath + "/" + m_sFileName;
	m_sFilePath = sPath;

	// Open a file for writing (create if necessary)
	QByteArray ba = sPath.toLatin1();

	pFile = fopen(ba.constData(), "w+");
	if (pFile == NULL)
		return;

	// Write header
	fprintf(pFile, "VERSION=%d\n", FILE_VERSION);
	fprintf(pFile, "View=%d\n", m_pGraphButtonGrp->checkedId());

	// Save the contents of all widgets
	m_pCalledWidget->save(pFile);
	m_pCallingWidget->save(pFile);
	m_pGraphWidget->save(pFile);

	// Close the file
	fclose(pFile);
}

/**
 * Saves the graph to a dot file.
 * The user is prompted for a name to use for the file, and then graph
 * widget writes its information to this file (using the dot language).
 * This slot is connected to the clicked() signal of the "Save As..." button.
 */
void CallTreeDlg::slotSaveClicked()
{
	QString sFile;

	// Prompt the user for a file name
	sFile = KFileDialog::getSaveFileName(KUrl(":kscope"));

	// Save the graph to a file (unless the user did not give a file name)
	if (!sFile.isEmpty())
		m_pGraphWidget->save(sFile);
}

/**
 * Increases the zoom factor of the graph.
 * This slot is connected to the clicked() signal of the "Zoom In" button.
 */
void CallTreeDlg::slotZoomInClicked()
{
	m_pGraphWidget->zoom(true);
	m_pGraphWidget->draw();
}

/**
 * Decreases the zoom factor of the graph.
 * This slot is connected to the clicked() signal of the "Zoom Out" button.
 */
void CallTreeDlg::slotZoomOutClicked()
{
	m_pGraphWidget->zoom(false);
	m_pGraphWidget->draw();
}

/**
 * Changes the graph's layout direction.
 * This slot is connected to the clicked() signal of the "Rotate" button.
 */
void CallTreeDlg::slotRotateClicked()
{
	m_pGraphWidget->rotate();
	m_pGraphWidget->draw();
}

/**
 * Opens the call graph preferences dialogue.
 * This slot is connected to the clicked() signal of the "Preferences" button.
 */
void CallTreeDlg::slotPrefClicked()
{

	GraphPrefDlg dlg(this);
	int nMaxNodeDegree;

	if (dlg.exec() == QDialog::Accepted) {
		nMaxNodeDegree = dlg.getMaxNodeDegree();
		Config().setGraphMaxNodeDegree(nMaxNodeDegree);
		m_pGraphWidget->setMaxNodeDegree(nMaxNodeDegree);
	}
}

/**
 * Prepares the selected view.
 * This slot is called when the user chooses a different view through the
 * toggle buttons in the dialogue's toolbar.
 * @param	nView	Identifies the selected view
 */
void CallTreeDlg::slotViewChanged(int nView)
{
	switch (nView) {
	case CallTreeDlg::GraphView:
		// Call graph
		setWindowTitle(i18n("Call Graph"));
		m_pGraphGroup->setEnabled(true);
		m_pHelpLabel->setText(i18n("Right-click a function node or an arrow "
			"head for more options."));
		break;

	case CallTreeDlg::CalledTree:
		// Called functions tree
		setWindowTitle(i18n("Called Functions Tree"));
		m_pGraphGroup->setEnabled(false);
		m_pHelpLabel->setText(i18n("Right-click a tree item for more "
			"options."));
		m_pCalledWidget->queryRoot();
		break;

	case CallTreeDlg::CallingTree:
		// Calling functions tree
		setWindowTitle(i18n("Calling Functions Tree"));
		m_pGraphGroup->setEnabled(false);
		m_pHelpLabel->setText(i18n("Right-click a tree item for more "
			"options."));
		m_pCallingWidget->queryRoot();
		break;
	}

	Config().setDefGraphView(nView);
}

#include "calltreedlg.moc"
