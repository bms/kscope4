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

#include <QtGui/QMenu>

#include <klocale.h>

#include "queryresultsmenu.h"
#include "queryview.h"

/**
 * Class constructor.
 * @param	pParent	Parent widget
 * @param	szName	Optional object name
 */
QueryResultsMenu::QueryResultsMenu(QWidget* pParent, const char* szName) :
	QMenu(pParent),
	m_pItem(NULL)
{
	Q_UNUSED(szName);

	// Create the menu
	m_pViewSourceAct = addAction(i18n("&View Source"), this, SLOT(slotViewSource()));
	m_pFindDefAct = addAction(i18n("Find &Definition"), this, SLOT(slotFindDef()));
	addSeparator();
	m_pCopyAct = addAction(i18n("&Copy"), this, SLOT(slotCopy()));
	addSeparator();
	addAction(i18n("&Filter..."), this, SLOT(slotFilter()));
	addAction(i18n("&Show All"), this, SIGNAL(showAll()));
	addSeparator();
	m_pRemoveAct = addAction(i18n("&Remove Item"), this, SLOT(slotRemove()));
}

/**
 * Class destructor.
 */
QueryResultsMenu::~QueryResultsMenu()
{
}

/**
 * Displays the popup-menu at the requested coordinates.
 * @param	pItem	The item on which the menu was requested
 * @param	ptPos	The requested position for the menu
 * @param	nCol	The column over which the menu was requested, -1 if no
 *					column is associated with the request
 */
void QueryResultsMenu::slotShow(const QPoint& ptPos)
{
	QueryView* pQueryView = dynamic_cast<QueryView*>(parent());
	QModelIndex index = pQueryView->indexAt(ptPos);

	// Save the requested item and column number to use in signals
	m_pItem = pQueryView->itemAt(ptPos);
	m_nCol = index.column();

	if (! index.isValid()) {
		// No item selected, disable everything but the "Filter" and "Show All" 
		// items
		m_pViewSourceAct->setEnabled(false);
		m_pFindDefAct->setEnabled(false);
		m_pCopyAct->setEnabled(false);
		m_pRemoveAct->setEnabled(false);
	} else {
		// Item selected, enable record-specific actions
		m_pViewSourceAct->setEnabled(true);
		m_pCopyAct->setEnabled(true);
		m_pRemoveAct->setEnabled(true);

		// The "Find Definition" item should only be enabled if the mouse
		// was clicked over a valid function name 
		m_pFindDefAct->setEnabled((m_nCol == 0) && 
			(m_pItem->text(0) != "<global>"));

		// Set menu contents according to the column number
		switch (m_nCol) {
		case 0:
			m_pCopyAct->setText("&Copy Function");
			break;

		case 1:
			m_pCopyAct->setText("&Copy File");
			break;

		case 2:
			m_pCopyAct->setText("&Copy Line Number");
			break;

		case 3:
			m_pCopyAct->setText("&Copy Text");
			break;

		default:
			m_nCol = 0;
		}
	}

	// Get global position & show the menu
	QWidget* pViewPort = pQueryView->viewport();
	popup(pViewPort->mapToGlobal(ptPos));
}

/**
 * Emits the viewSource() signal.
 * This slot is activated when the "View Source" item is selected.
 */
void QueryResultsMenu::slotViewSource()
{
	if (m_pItem != NULL)
		emit viewSource(m_pItem);
}

/**
 * Emits the findDef() signal.
 * This slot is activated when the "Find Definition" item is selected.
 */
void QueryResultsMenu::slotFindDef()
{
	if (m_pItem != NULL)
		emit findDef(m_pItem->text(0));
}

/**
 * Emits the copy() signal.
 * This slot is activated when the "Copy [Column]" item is selected.
 */
void QueryResultsMenu::slotCopy()
{
	if (m_pItem != NULL)
		emit copy(m_pItem, m_nCol);
}

/**
 * Emits the filter() signal.
 * This slot is activated when the "Filter..." item is selected.
 */
void QueryResultsMenu::slotFilter()
{
	emit filter(m_nCol);
}

/**
 * Emits the remove() signal.
 * This slot is activated when the "Remove" item is selected.
 */
void QueryResultsMenu::slotRemove()
{
	if (m_pItem != NULL)
		emit remove(m_pItem);
} 

#include "queryresultsmenu.moc"
