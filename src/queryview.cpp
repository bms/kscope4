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

#include <qapplication.h>
#include <qclipboard.h>
//Added by qt3to4:
#include <QMouseEvent>

#include <klocale.h>

#include "queryview.h"
#include "queryresultsmenu.h"
#include "queryviewdlg.h"
#include "cscopefrontend.h"
#include "searchresultsdlg.h"

/**
 * Class constructor.
 * @param	pParent	The parent widget
 * @param	szName	The name of the widget
 */
QueryView::QueryView(QWidget* pParent, const char* szName) :
	QTreeWidget(pParent)
{
	Q_UNUSED(szName);

	// Create the popup-menu
	m_pQueryMenu = new QueryResultsMenu(this);
	setContextMenuPolicy(Qt::CustomContextMenu);

	// Initialise the model & list's columns
	m_pModel = model();
	setColumnCount(4);
	m_pSelectionModel = new QItemSelectionModel(m_pModel);
	setSelectionModel(m_pSelectionModel);
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	setAllColumnsShowFocus(true);
	setRootIsDecorated(false);
	setAlternatingRowColors(true);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);

	// Setup model's header
	m_pModel->setHeaderData(0, Qt::Horizontal, i18n("Function"));
	m_pModel->setHeaderData(1, Qt::Horizontal, i18n("File"));
	m_pModel->setHeaderData(2, Qt::Horizontal, i18n("Line"));
	m_pModel->setHeaderData(3, Qt::Horizontal, i18n("Text"));
	header()->setSortIndicator(1, Qt::AscendingOrder);
	header()->setSortIndicatorShown(true);
	header()->resizeSection(0, 200);
	header()->resizeSection(1, 350);

	// A record is selected if it is either double-clicked, or the ENTER
	// key is pressed while the record is highlighted
	connect(this, SIGNAL(itemActivated(QTreeWidgetItem*, int)), this,
		SLOT(slotRecordSelected(QTreeWidgetItem*)));
		
	// Show the popup-menu when requested
	connect(this, 
		SIGNAL(customContextMenuRequested(const QPoint&)),
		m_pQueryMenu, SLOT(slotShow(const QPoint&)));
		
	// Handle popup-menu commands
	connect(m_pQueryMenu, SIGNAL(viewSource(QTreeWidgetItem*)), this,
		SLOT(slotRecordSelected(QTreeWidgetItem*)));
	connect(m_pQueryMenu, SIGNAL(findDef(const QString&)), this,
		SLOT(slotFindDef(const QString&)));
	connect(m_pQueryMenu, SIGNAL(copy(QTreeWidgetItem*, int)), this,
		SLOT(slotCopy(QTreeWidgetItem*, int)));
	connect(m_pQueryMenu, SIGNAL(filter(int)), this, SLOT(slotFilter(int)));
	connect(m_pQueryMenu, SIGNAL(showAll()), this, SLOT(slotShowAll()));
	connect(m_pQueryMenu, SIGNAL(remove(QTreeWidgetItem*)), this,
		SLOT(slotRemoveItem(QTreeWidgetItem*)));
}

/**
 * Class destructor.
 */
QueryView::~QueryView()
{
}

/**
 * Creates a new list item showing a query result record.
 * @param	sFunc	The name of the function
 * @param	sFile	The file path
 * @param	sLine	The line number in the above file
 * @param	sText	The line's text
 */
void QueryView::addRecord(const QString& sFunc, const QString& sFile,
	const QString& sLine, const QString& sText, QTreeWidgetItem* /* pParent */)
{
	QTreeWidgetItem* pItem;
	
	// Insert new item in "this" after last one
	pItem = new QTreeWidgetItem(this, m_pLastItem);
	pItem->setText(0, sFunc);
	pItem->setText(1, sFile);
	pItem->setText(2, sLine);
	pItem->setTextAlignment(2, Qt::AlignCenter);
	pItem->setText(3, sText);

	m_pLastItem = pItem;
}

/**
 * Selects an item.
 * When an item is selected, it is highlighted and made visible. By
 * definition, the lineRequested() signal is also emitted.
 * This method is used for selecting records programmatically (@see
 * selectNext() for example). It has nothing to do with user selection.
 * @param	pItem	The list item to select
 */
void QueryView::select(QTreeWidgetItem* pItem)
{
	scrollToItem(pItem, QAbstractItemView::EnsureVisible);
	pItem->setSelected(true);
 	setCurrentItem(pItem, 0, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
	slotRecordSelected(pItem);
}

/**
 * Selects the next record in the list (if one exists).
 * The function selects the next item as follows:
 * - The first item in the list, if there is no current item
 * - The current item, if it is not selected
 * - The item immediately below the current item, otherwise
 */
void QueryView::selectNext()
{
	QTreeWidgetItem* pItem;
	
	// Do nothing if the list is empty
	if (invisibleRootItem()->childCount() == 0)
		return;

	// Find the next record
	pItem = currentItem();
	if (pItem == NULL) {
		pItem = invisibleRootItem()->child(0);
	} else if (pItem->isSelected()) {
		pItem = itemBelow(pItem);
		if (pItem == NULL)
			return;
	}

	// Select the new item
	select(pItem);
}

/**
 * Selects the previous record in the list (if one exists).
 * The function selects the previous item as follows:
 * - The first item in the list, if there is no current item
 * - The current item, if it is not selected
 * - The item immediately above the current item, otherwise
 */
void QueryView::selectPrev()
{
	QTreeWidgetItem* pItem;

	// Do nothing if the list is empty
	if (invisibleRootItem()->childCount() == 0)
		return;

	// Find the item immediately above this one
	pItem = currentItem();
	if (pItem == NULL) {
		pItem = invisibleRootItem()->child(0);
	} else if (pItem->isSelected()) {
		pItem = itemAbove(pItem);
		if (pItem == NULL)
			return;
	}

	// Select the new item
	select(pItem);
}

/**
 * Informs the view that query progress information has been received.
 * The view emits the needToShow() signal telling its parent that the widget
 * should become visible (if not already so).
 */
void QueryView::queryProgress()
{
	if (!isVisible())
		emit needToShow();
}

/**
 * Called when a query using this view terminates.
 * @param	nRecords	Number of records generated by the query
 */
void QueryView::queryFinished(uint nRecords, QTreeWidgetItem* /* pParent */)
{
	// Auto-select a single record (no need to emit the show() signal in
	// that case)
	if (nRecords == 1) {
		emit select(invisibleRootItem()->child(0));
		return;
	}
	
	// Report a query that has returned an empty record set
	if (nRecords == 0)
		addRecord(i18n("No results"), "", "", "");
	
	// Data is available, instruct the owner object to show the view
	emit needToShow();
}

#if 0
/**
 * Creates an iterator and initialises it to point to the first _visible_
 * item in the list.
 * @return	A new iterator initialised to the beginning of the list
 */
QueryView::Iterator QueryView::getIterator()
{
	QTreeWidgetItem* pItem;
	
	for (pItem = firstChild(); pItem != NULL && !pItem->isVisible(); 
		pItem = pItem->nextSibling());
		
	return Iterator(pItem);
}

/**
 * Handles double-click events over the view.
 * NOTE: We override this method since the QListView implementation opens
 * expandable items. This is undesired for tree-like query views (such as the
 * call tree.
 * @param	pEvent	Event description object
 */
void QueryView::contentsMouseDoubleClickEvent(QMouseEvent* pEvent)
{
	QTreeWidgetItem* pItem;
	
	// Handle only left button double-clicks
	if (pEvent == NULL || pEvent->button() != Qt::LeftButton)
		return;

	// Find the clicked item
	pItem = itemAt(contentsToViewport(pEvent->pos()));
	if (pItem == NULL)
		return;

	// Emit the doubleClicked() signal
	emit doubleClicked(pItem);
}
#endif

/**
 * Emits the lineRequested() signal when a list item is selected.
 * This slot is connected to the doubleClicked() and returnPressed()
 * signals of the list view.
 * @param	pItem	The selected item
 */
void QueryView::slotRecordSelected(QTreeWidgetItem* pItem)
{
	QString sFileName, sLine;

	// Get the file name and line number
	sFileName = pItem->text(1);
	sLine = pItem->text(2);

	//	KMessageBox::informationList(0, QString("QueryView::slotRecordSelected"),
	//		QStringList() << sFileName << sLine);

	// Do not process the "No results" item
	if (!sLine.isEmpty())
		emit lineRequested(sFileName, sLine.toUInt());
}

/**
 * Looks up the definition of a given function.
 * Results are displayed in a popup window.
 * This slot is connected to the findDef() signal emitted by the results menu.
 * @param	sFunc	The function to look for
 */
void QueryView::slotFindDef(const QString& sFunc)
{
	QueryViewDlg* pDlg;

	// Create a query view dialogue
	pDlg = new QueryViewDlg(QueryViewDlg::DestroyOnSelect, this);

	// Display a line when it is selected in the dialogue
	connect(pDlg, SIGNAL(lineRequested(const QString&, uint)), this,
		SIGNAL(lineRequested(const QString&, uint)));

	// Start the query
	pDlg->query(CscopeFrontend::Definition, sFunc);
}

/**
 * Copies the text of the requested field (item+column) to the clipboard.
 * This slot is connected to the copy() signal of the QueryResultsMenu object.
 * @param	pItem	The item from which to copy
 * @param	nCol	The index of the item field to copy
 */
void QueryView::slotCopy(QTreeWidgetItem* pItem, int nCol)
{
	QApplication::clipboard()->setText(pItem->text(nCol), QClipboard::Clipboard);
}

/**
 * Hides all items in the page that do not meet the given search criteria.
 * This slot is connected to the search() signal of the QueryResultsMenu
 * object.
 * The search is incremental: only visible items are checked, so that a new
 * search goes over the results of the previous one.
 * @param	nCol	The list column to search in
 */
void QueryView::slotFilter(int nCol)
{
	SearchResultsDlg dlg(this);
	QRegExp re;
	QString str;
	bool bNegate;

	// Prepare the dialogue
	dlg.setColumn(nCol);

	// Show the dialogue
	if (dlg.exec() != QDialog::Accepted)
		return;

	// Get the selected regular expression
	dlg.getPattern(re);
	bNegate = dlg.isNegated();

	// Disable visual updates while search is in progress
	setUpdatesEnabled(false);

	// Iterate over all items in the list : Filter visible items only
	QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::NotHidden);
	while (*it) {
		str = (*it)->text(nCol);
		if ((! str.isEmpty()) && ((re.indexIn(str) == -1) != bNegate))
			(*it)->setHidden(true);
		*it++;
	}

	// Redraw the list
	setUpdatesEnabled(true);
}

/**
 * Makes all list items visible.
 * This slot is connected to the showAll() signal of the QueryResultsMenu
 * object.
 */
void QueryView::slotShowAll()
{
	// Iterate over all hidden items in the list
	QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Hidden);
	while (*it) {
		(*it++)->setHidden(false);
	}
}

/**
 * Deletes the item on which a popup-menu has been invoked.
 * This slot is connected to the remove() signal of the QueryResultsMenu
 * object.
 * @param	pItem	The item to remove
 */
void QueryView::slotRemoveItem(QTreeWidgetItem* pItem)
{
	invisibleRootItem()->removeChild(pItem);
}

#if 0
/**
 * Moves the iterator to the next _visible_ item in the list.
 */
void QueryView::Iterator::next()
{
	if (m_pItem == NULL)
		return;
		
	do {
		m_pItem = m_pItem->nextSibling();
	} while (m_pItem != NULL && !m_pItem->isVisible());
}

/**
 * @return	The function associated with the list item the iterator points to
 */
QString QueryView::Iterator::getFunc()
{
	if (m_pItem == NULL)
		return "";
		
	return m_pItem->text(0);
}

/**
 * @return	The file associated with the list item the iterator points to
 */
QString QueryView::Iterator::getFile()
{
	if (m_pItem == NULL)
		return "";
		
	return m_pItem->text(1);
}

/**
 * @return	The line number associated with the list item the iterator points
 *			to
 */
QString QueryView::Iterator::getLine()
{
	if (m_pItem == NULL)
		return "";
		
	return m_pItem->text(2);
}

/**
 * @return	The text associated with the list item the iterator points to
 */
QString QueryView::Iterator::getText()
{
	if (m_pItem == NULL)
		return "";
		
	return m_pItem->text(3);
}
#endif
#include "queryview.moc"
