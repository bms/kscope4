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

#include <QtGui/QHeaderView>
#include <QKeyEvent>

#include "kscopeconfig.h"
#include "searchlist.h"

/**
 * Intercepting additional key events of QLineEdit to browse the list
 * @param	pKey	The pressed key event
 */
void SearchLineEdit::keyPressEvent(QKeyEvent* pKey) 
{
	switch(pKey->key()) {
	case Qt::Key_Up:
	case Qt::Key_Down:
	case Qt::Key_PageUp:
	case Qt::Key_PageDown:
		emit keyPressed(pKey); 
		break;

	default:
		QLineEdit::keyPressEvent(pKey);
		break;
	}
}

/**
 * Class constructor.
 * @param	nSearchCol	The list column on which to perform string look-ups
 * @param	pParent		The parent widget
 * @param	szName		The widget's name
 */
SearchList::SearchList(int nSearchCol, QWidget* pParent, const char* szName) :
	QWidget(pParent),
	m_nSearchCol(nSearchCol)
{
	Q_UNUSED(szName);

	// Create the child widgets
	QVBoxLayout *pLayout = new QVBoxLayout;

	m_pEdit = new SearchLineEdit();
	m_pList = new QTreeWidget();
	m_pList->setAllColumnsShowFocus(true);
	m_pList->setRootIsDecorated(false);
	m_pList->setAlternatingRowColors(true);
	m_pList->setColumnCount(3);

	m_pList->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pList->setSelectionMode(QAbstractItemView::SingleSelection);

	// Set up the tooltip generator
	m_pList->setToolTip(QString());
	m_pToolTip = new QString();

	pLayout->addWidget(m_pEdit);
	pLayout->addWidget(m_pList);
	setLayout(pLayout);

	connect(m_pEdit, SIGNAL(textChanged(const QString&)), this,
		SLOT(slotFindItem(const QString&)));
	connect(m_pEdit, SIGNAL(returnPressed()), this,
		SLOT(slotItemSelected()));
	connect(m_pEdit, SIGNAL(keyPressed(QKeyEvent*)), this,
		SLOT(slotKeyPressed(QKeyEvent*)));

	connect(m_pList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this,
		SLOT(slotItemSelected(QTreeWidgetItem*, int)));
}

/**
 * Class destructor.
 */
SearchList::~SearchList()
{
	delete m_pToolTip;
}

bool SearchList::event(QEvent *e)
{
	if (e->type() == QEvent::ToolTip) {
		QHelpEvent *hEvent = static_cast<QHelpEvent *>(e);
		QPoint pt = m_pList->mapFromParent(hEvent->pos());

		pt.setY(pt.y() -  m_pList->header()->height());

		QTreeWidgetItem* pItem = m_pList->itemAt(pt);

		if (pItem) {
			QString sTip;
			getTip(pItem, sTip);
			QToolTip::showText(hEvent->globalPos(), sTip);
		} else {
			QToolTip::hideText();
			e->ignore();
		}
		return true;
	}
	return QWidget::event(e);
}

/**
 * Sets the keyboad focus to the search box.
 */
void SearchList::slotSetFocus()
{
	m_pEdit->setFocus();
}

/**
 * Selects a list item whose string begins with the text entered in the edit
 * widget.
 * This slot is connected to the textChanged() signal of the line edit widget.
 * @param	sText	The new text in the edit widget
 */
void SearchList::slotFindItem(const QString& sText)
{
	QList<QTreeWidgetItem*> itemsList;

	// To avoid selecting everything when clearing QLineEdit text
	if (sText.isEmpty())
		return;

	// Find & select all matching items
	itemsList = m_pList->findItems(sText, Qt::MatchStartsWith, m_nSearchCol);
	if (! itemsList.isEmpty()) {
		QTreeWidgetItem* item = itemsList.first();

		m_pList->setCurrentItem(item);
	}
}

/**
 * Lets inheriting classes process an item selection made through the list 
 * widget.
 * This slot is connected to the doubleClicked() and returnPressed()
 * signals of the list widget.
 */
void SearchList::slotItemSelected(QTreeWidgetItem* pItem, int column)
{
	Q_UNUSED(column);

	processItemSelected(pItem);
	m_pEdit->setText("");
}

/**
 * Lets inheriting classes process an item selection made through the edit 
 * widget.
 * This slot is connected to the returnPressed() signal of the edit widget.
 */
void SearchList::slotItemSelected()
{
	QList<QTreeWidgetItem*> selectedItemsList = m_pList->selectedItems();

	if (!selectedItemsList.isEmpty()){
		QListIterator<QTreeWidgetItem*> it(selectedItemsList);

		while (it.hasNext())
			processItemSelected(it.next());
	}

	m_pEdit->setText("");
}

#define SEARCH_MATCH(pItem) \
	pItem->text(m_nSearchCol).startsWith(m_pEdit->text())

/**
 * Sets a new current item based on key events in the edit box.
 * This slot is connected to the keyPressed() signal of the edit widget.
 * @param	pKey	The key evant passed by the edit box
 */
void SearchList::slotKeyPressed(QKeyEvent* pKey)
{
	QTreeWidgetItem *pItem, *pNewItem;
	QModelIndex index;
	int nPageSize;
	int key = pKey->key();

	// Select the current item, or the first one if there is no current item
	pItem = m_pList->currentItem();

	if ((key == Qt::Key_PageUp) || (key == Qt::Key_PageDown)){
		nPageSize = m_pList->contentsRect().height() / m_pList->sizeHintForRow(0);
		index = m_pList->currentIndex();
	}

	// Set a new current item based on the pressed key
	switch (key) {
	case  Qt::Key_Up:
		if (pItem) {
			for (pNewItem = m_pList->itemAbove(pItem); 
				pNewItem && !SEARCH_MATCH(pNewItem);
				pNewItem = m_pList->itemAbove(pNewItem));

			if (pNewItem)
				pItem = pNewItem;
		}
		break;

	case  Qt::Key_Down:
		if (pItem) {
			for (pNewItem = m_pList->itemBelow(pItem); 
				pNewItem && !SEARCH_MATCH(pNewItem);
				pNewItem = m_pList->itemBelow(pNewItem));

			if (pNewItem)
				pItem = pNewItem;
		}
		break;

	case  Qt::Key_PageUp:
		pNewItem = m_pList->topLevelItem(index.row() - (nPageSize - 1));
		if ( pNewItem != NULL ){
			m_pList->scrollToItem(pNewItem, QAbstractItemView::EnsureVisible);
			pItem = pNewItem;
		} else {
			m_pList->scrollToTop();
			pItem = m_pList->topLevelItem(0);
		}
		break;

	case  Qt::Key_PageDown:
		pNewItem = m_pList->topLevelItem(index.row() + (nPageSize - 1));
		if ( pNewItem != NULL ){
			m_pList->scrollToItem(pNewItem, QAbstractItemView::EnsureVisible);
			pItem = pNewItem;
		} else {
			m_pList->scrollToBottom();
			pItem = m_pList->topLevelItem(m_pList->topLevelItemCount() - 1);
		}
		break;

	default:
		pKey->ignore();
		return;
	}

	// Select the first item if no other item was selected
	if (pItem == NULL)
		pItem = m_pList->topLevelItem(0);

	// Select the new item
	if (pItem) {
		pItem->setSelected(true);
		pItem->setHidden(false);
		m_pList->setCurrentItem(pItem);
	}
}

#include "searchlist.moc"

/*
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
