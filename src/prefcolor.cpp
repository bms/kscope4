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

#include <QtGui/QTreeView>
#include <qpainter.h>
#include <QPixmap>

#include <kcolordialog.h>

#include "prefcolor.h"
#include "kscopeconfig.h"

/**
 * Class constructor.
 * @param	pParent	The parent widget
 * @param	szName	The widget's name
 */
PrefColor::PrefColor(QWidget* pParent, const char* szName) :
	QWidget(pParent)
{
	Q_UNUSED(szName);

	setupUi(this);

	// Setup model's column count & header
	m_pModel = new QStandardItemModel();
	m_pModel->setColumnCount(2);
	m_pModel->setHeaderData(0, Qt::Horizontal, i18n("Gui Element"));
	m_pModel->setHeaderData(1, Qt::Horizontal, i18n("Colour"));
	m_pModel->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);

	m_pList->setModel(m_pModel);
	m_pList->setRootIsDecorated(false);
	m_pList->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_pList->header()->setStretchLastSection(false);
	m_pList->header()->setResizeMode(0, QHeaderView::ResizeToContents);
	m_pList->header()->setResizeMode(1, QHeaderView::Fixed);
	m_pList->header()->resizeSection(1, 65);

	m_pSelectionModel = new QItemSelectionModel(m_pModel);
	m_pList->setSelectionModel(m_pSelectionModel);

	// Set initial values
	load();
}

/**
 * Class destructor.
 */
PrefColor::~PrefColor()
{
}

/**
 * Reads the current settings from the configuration object, and applies them
 * the the page's widget.
 */
void PrefColor::load()
{
	uint i;
	QList<QStandardItem*> lItems;
	QModelIndex index;
	KScopeConfig::ColorElement ce;

	// Create a list item for every GUI element
	for (i = 0; i <= KScopeConfig::LAST_COLOR; i++){
		ce = (KScopeConfig::ColorElement)i;
		lItems << new QStandardItem(Config().getColorName(ce) + "\t") << new QStandardItem("");
		lItems[1]->setTextAlignment(Qt::AlignLeft);
		lItems[1]->setBackground(QBrush(Config().getColor(ce)));
		m_pModel->appendRow(lItems);
		index = m_pModel->indexFromItem(lItems[0]);
		lItems.clear();
		m_pSelectionModel->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
	}
}

/**
 * Commits settings changes to the configuration object.
 */
void PrefColor::apply()
{
	QStandardItem* pRoot = m_pModel->invisibleRootItem();
	QStandardItem* pItem;
	int rowCount = pRoot->rowCount();

	// Create a list item for every GUI element
	for (int i = 0; i < rowCount; i++){
		pItem = pRoot->child(i, 1);
		Config().setColor((KScopeConfig::ColorElement)i, (pItem->background()).color());
	}
}

/**
 * Displays a colour selection dialogue when an item is selected.
 * If the user chooses a new colour, it is set to the selected item.
 * This slot is connected to both the doubleClicked() and the returnPressed()
 * signals of the list view.
 * @param	pItem	The selected item
 */
void PrefColor::slotItemSelected(const QModelIndex& index)
{
	QStandardItem* pItem;
	QColor clr;

	pItem = m_pModel->item(index.row(), 1);
	if (KColorDialog::getColor(clr, (pItem->background()).color()) == QDialog::Accepted) {
		pItem->setBackground(QBrush(clr));
		emit modified();
	}
}

#include "prefcolor.moc"
