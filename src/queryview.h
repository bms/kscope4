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

#ifndef QUERYVIEW_H
#define QUERYVIEW_H

#include <QtGui/QTreeWidget>
#include <qregexp.h>
//Added by qt3to4:
#include <QMouseEvent>

class QueryResultsMenu;

/**
 * A list view widget for displaying locations in the source code. Each record
 * (list item) represents a single line of code.
 * The main purpose of this class is for showing query results (@see
 * QueryViewDriver), but is can also serve as a base class for any widget
 * which needs to refer to locations in the source code (@see
 * HistoryView).
 * The view has 4 columns, for showing the file path, function name, line
 * number and line text of a code location.
 * The widget owns a popup menu which allows users to copy information
 * from records, filter records, and more.
 * @author Elad Lahav
 */
class QueryView : public QTreeWidget
{
	Q_OBJECT
	
public:
	QueryView(QWidget* pParent = 0, const char* szName = 0);
	~QueryView();
	
	virtual void addRecord(const QString&, const QString&, const QString&,
		const QString&, QTreeWidgetItem* pParent = NULL);
	virtual void select(QTreeWidgetItem*);
	virtual void selectNext();
	virtual void selectPrev();
	virtual void queryProgress();
	virtual void queryFinished(uint, QTreeWidgetItem* pParent = NULL);

signals:
	/**
	 * Notifies the owner widget that it needs to be visible since some
	 * information is available to display.
	 * This information may be an advancement of the progress bar,
	 * availability of query results, etc.
	 */
	void needToShow();

	/**
	 * Emitted when a list record is selected. 
	 * Selection is done by either double-clicking a query or by highlighting
	 * it and then pressing the ENTER key.
	 * @param	sFile	The "File" field of the selected record
	 * @param	nLine	The "Line" field of the selected record
	 */
	void lineRequested(const QString& sFile, uint nLine);	

protected:	
	/** A popup-menu for manipulating query result items. */
	QueryResultsMenu* m_pQueryMenu;

	/** Pointers to data & selection models used by the view */
	QAbstractItemModel* m_pModel;
	QItemSelectionModel* m_pSelectionModel;

	/** A pointer to the last item (used for appending results). */
	QTreeWidgetItem* m_pLastItem;

	void contentsMouseDoubleClickEvent(QMouseEvent*);

protected slots:
	virtual void slotRecordSelected(QTreeWidgetItem*);
	virtual void slotFindDef(const QString&);
	virtual void slotCopy(QTreeWidgetItem*, int);
	virtual void slotFilter(int);
	virtual void slotShowAll();
	virtual void slotRemoveItem(QTreeWidgetItem*);
};

#endif
