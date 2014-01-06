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

#include <QTextStream>

#include "treewidget.h"
#include "queryviewdriver.h"

/**
 * Class constructor.
 * @param	pParent	Parent widget
 * @param	szName	The name of the widget
 */
TreeWidget::TreeWidget(QWidget* pParent, const char* szName) :
	QueryView(pParent, szName),
	m_nQueryType(CscopeFrontend::Called)
{
	setRootIsDecorated(true);
	setItemsExpandable(true);
	
	// Create a driver object
	m_pDriver = new QueryViewDriver(this, this);
	
	// Query a tree item when it is expanded for the first time
	connect(this, SIGNAL(itemExpanded(QTreeWidgetItem*)), this,
		SLOT(slotQueryItem(QTreeWidgetItem*)));
}

/**
 * Class destructor.
 */
TreeWidget::~TreeWidget()
{
}

/**
 * Determines the mode of the tree.
 * @param	mode	The new mode (@see Mode)
 */
void TreeWidget::setMode(Mode mode)
{
	m_nQueryType = (mode == Called) ? CscopeFrontend::Called :
		CscopeFrontend::Calling;
}

/**
 * Sets a new root item for the tree.
 * @param	sFunc	The name of the function to serve as root
 */
void TreeWidget::setRoot(const QString& sFunc)
{
	QTreeWidgetItem* pRoot;
	
	// Remove the current root, if any
	if ((pRoot = invisibleRootItem()->child(0)) != NULL)
		invisibleRootItem()->removeChild(pRoot);
	
	// Create a new root item
	pRoot = new QTreeWidgetItem(this, QStringList(sFunc));
	pRoot->setExpanded(true);
}

/**
 * Runs a query on the root item.
 */
void TreeWidget::queryRoot()
{
	QTreeWidgetItem* pRoot;
	
	if ((pRoot = invisibleRootItem()->child(0)) != NULL)
		slotQueryItem(pRoot);
}

/**
 * Stores the tree contents in the given file.
 * @param	pFile	An open file to write to
 */
void TreeWidget::save(FILE* pFile)
{
	QTextStream str(pFile, QIODevice::WriteOnly);
	QTreeWidgetItem* pRoot;
	Encoder enc;
	
	if (m_nQueryType == CscopeFrontend::Called)
		str << "calltree {" << endl;
	else
		str << "callingtree {" << endl;

	// Write the tree to the file
	pRoot = invisibleRootItem()->child(0);
	str << pRoot->text(0) << endl;
	str << '{' << endl;
	saveItems(pRoot, str, enc);
	str << '}' << endl;
	str << '}' << endl;
}

/**
 * Recursively writes tree items to a file.
 * Given an item, the method writes this item and all of its siblings.
 * Child items are written recursively.
 * @param	pItem	The first item to write
 * @param	str		An initialised text stream to use for writing
 * @param	enc		An encoder for free-text strings
 */
void TreeWidget::saveItems(QTreeWidgetItem* pParent, QTextStream& str, Encoder& enc)
{
	QTreeWidgetItem* pItem;

	// Iterate over all items in this level
	pItem = pParent->child(0);
	for (int i = 0; i < pParent->childCount(); pItem = pParent->child(++i)) {
		// Write function parameters
		str << pItem->text(0) << " [ "
		    << "kscope_file=\"" << pItem->text(1) << "\", "
		    << "kscope_line=" << pItem->text(2) << ", "
		    << "kscope_text=\"" << enc.encode(pItem->text(3)) << "\""
		    << "]" << endl;

		// Write child items
		if (pItem->childCount()) {
			str << "{" << endl;
			saveItems(pItem, str, enc);
			str << "}" << endl;
		}
	}
}

/**
 * Creates a new tree item showing a query result record.
 * @param	sFunc	The name of the function
 * @param	sFile	The file path
 * @param	sLine	The line number in the above file
 * @param	sText	The line's text
 * @param	pParent	The parent for the new item
 */
void TreeWidget::addRecord(const QString& sFunc, const QString& sFile,
	const QString& sLine, const QString& sText, QTreeWidgetItem* pParent)
{
	QTreeWidgetItem* pItem;

	pItem = new QTreeWidgetItem(pParent, m_pLastItem);
	pItem->setText(0, sFunc);
	pItem->setText(1, sFile);
	pItem->setText(2, sLine);
	pItem->setTextAlignment(2, Qt::AlignCenter);
	pItem->setText(3, sText);
	pItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

	m_pLastItem = pItem;
}

/**
 * Called when a query running on the tree terminates.
 * If there were no results, the item becomes non-expandable.
 * NOTE: On top of its current behaviour, this function is required in order to
 * override the default behaviour, as specified by QueryView.
 * @param	nResults	Number of results
 * @param	pParent		The item for which the query was executed
 */
void TreeWidget::queryFinished(uint nResults, QTreeWidgetItem* pParent)
{
	if (nResults == 0)
		pParent->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
}

/**
 * Runs a query on the given item, unless it was queried before.
 * This slot is connected to the expanded() signal of the list view.
 * @param	pItem	The item to query
 */
void TreeWidget::slotQueryItem(QTreeWidgetItem* pItem)
{
	// Do nothing if the item was already queried
	// An item has been queried if it has children or marked as non-expandable
	if (pItem->childCount())
		return;
		
	// Run the query
	m_pDriver->query(m_nQueryType, pItem->text(0), pItem);
}

/**
 * Hides all descendant that do not meet the given search criteria.
 * This slot is connected to the search() signal of the QueryResultsMenu
 * object.
 * The search is incremental: only visible items are checked, so that a new
 * search goes over the results of the previous one.
 * @param	pParent	The parent item whose child are searched
 * @param	re		The pattern to search
 * @param	nCol	The list column to search in
 */
void TreeWidget::slotSearch(QTreeWidgetItem* pParent, const QRegExp& re, 
	int nCol)
{
	QTreeWidgetItem* pItem;
	
	// Get the first child
	pItem = (pParent != NULL) ? pParent->child(0) : invisibleRootItem()->child(0);
	
	// Iterate over all child items : filter visible items only
	QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::NotHidden);
	while (*it) {
		if (re.indexIn(pItem->text(nCol)) == -1)
			(*it)->setHidden(true);
		
		// Search recursively child items ( if any )
		if ((*it)->childCount()) slotSearch((*it), re, nCol);
		*it++;
	}
}

/**
 * Makes all descendants of the given item visible.
 * This slot is connected to the showAll() signal of the QueryResultsMenu
 * object.
 */
void TreeWidget::slotShowAll(QTreeWidgetItem* pParent)
{
	QTreeWidgetItem* pItem;

	// Get the first child
	pItem = (pParent != NULL) ? pParent->child(0) : invisibleRootItem()->child(0);

	// Iterate over all child items
	QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Hidden);
	while (*it) {
		(*it)->setHidden(false);

		// Show child items recursively
		if ((*it)->childCount()) slotShowAll(*it);
	}
}

#include "treewidget.moc"
