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

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlistwidget.h>

#include <kurlrequester.h>

#include "openprojectdlg.h"
#include "kscopeconfig.h"

/**
 * Class constructor.
 * @param	pParent	The parent widget
 * @param	szName	The widget's name
 */
OpenProjectDlg::OpenProjectDlg(QWidget* pParent, const char* szName) :
	QDialog(pParent)
{
	setupUi(this);
	setObjectName(szName);

	loadRecent();
	m_pProjPathRequester->setFilter("cscope.proj");
}

/**
 * Class destructor.
 */
OpenProjectDlg::~OpenProjectDlg()
{
}

/**
 * @return	The selected project path
 */
QString OpenProjectDlg::getPath() const
{
	return m_pProjPathRequester->url().pathOrUrl();
}

/**
 * Sets the requester to reflect the selected project's directory, instead of
 * the cscope.proj file.
 * @param	sProjPath	The full path of the selected cscope.proj file
 */
void OpenProjectDlg::slotProjectSelected(const KUrl& url)
{
	QString sProjPath = url.pathOrUrl();
	QFileInfo fi(sProjPath);

	m_pProjPathRequester->setUrl(KUrl(fi.absolutePath()));
}

/**
 * Removes a project from the recent projects list.
 * This slot is connected to the clicked() signal of the "Remove" button.
 */
void OpenProjectDlg::slotRemoveRecent()
{
	QListWidgetItem* pItem;
	QList<QListWidgetItem*>lSelectedItems;

	// Remove the selected item, if any
	lSelectedItems = m_pRecentList->selectedItems();

	QListIterator<QListWidgetItem*> itr(lSelectedItems);

	while (itr.hasNext()) {
		pItem = itr.next();
		Config().removeRecentProject(pItem->text());
		pItem = m_pRecentList->takeItem(m_pRecentList->row(pItem));
		delete pItem;
	}
}

/**
 * Selects a project for opening when an item is highlighted in the recent
 * projects list.
 * This slot is connected to the highlighted() signal of the recent projects
 * list box.
 * @param	pItem	The selected project item
 */
void OpenProjectDlg::slotSelectRecent(QListWidgetItem* pItem)
{
	if (pItem != NULL)
	  m_pProjPathRequester->setUrl(KUrl(pItem->text()));
}

/**
 * Selects a project for opening and closes the dialogue when an item in the
 * recent projects list is double-clicked. 
 * This slot is connected to the doubleClicked() signal of the recent 
 * projects list box.
 * @param	pItem	The selected project item
 */
void OpenProjectDlg::slotOpenRecent(QListWidgetItem* pItem)
{
	if (pItem != NULL) {
		m_pProjPathRequester->setUrl(KUrl(pItem->text()));
		accept();
	}
}

/**
 * Fills the recent projects list box with the project paths read from the 
 * configuration file.
 */
void OpenProjectDlg::loadRecent()
{
	const QStringList& slProjects = Config().getRecentProjects();
	QStringList::const_iterator itr;

	// Create a list item for each project in the list
	for (itr = slProjects.begin(); itr != slProjects.end(); ++itr)
		m_pRecentList->addItem(*itr);
}

#include "openprojectdlg.moc"
