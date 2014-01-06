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

#include <qtreewidget.h>
#include <qpainter.h>
//Added by qt3to4:
#include <QPixmap>

#include <kfontdialog.h>
#include <klocale.h>

#include "preffont.h"
#include "kscopeconfig.h"

/**
 * A list view item that shows the name of a GUI element and the font
 * associated with it.
 * The font is presented in the form of a sample text drawn using this font.
 * @author	Elad Lahav
 */
class FontListItem : public QTreeWidgetItem
{
public:
	/**
	 * Class constructor.
	 * @param	pList	The owner list view
	 * @param	fe		The GUI element shown by this item
	 */
	FontListItem(QTreeWidget* pList, KScopeConfig::FontElement fe) : 
		QTreeWidgetItem(pList, QStringList() << Config().getFontName(fe) << ""),
		m_fe(fe) {
		setText(1, QString("Sample"));
		setTextAlignment(1, Qt::AlignLeft);
		setFont(Config().getFont(fe));
	}

	/**
	 * @return	The GUI element shown by this item
	 */
	KScopeConfig::FontElement getElement() { return m_fe; }

	/**
	 * Changes the font associated with this item.
	 * @param	font	The font to set
	 */
	void setFont(QFont font) {
		QTreeWidgetItem::setFont(1, font);
	}
	
	/**
	 * @return	The font associated with this item
	 */
	QFont getFont() { return m_font; }
	
private:
	/** The GUI element shown by this item. */
	KScopeConfig::FontElement m_fe;
	
	/** The font associated with this item. */
	QFont m_font;
};

/**
 * Class constructor.
 * @param	pParent	The parent widget
 * @param	szName	The widget's name
 */
PrefFont::PrefFont(QWidget* pParent, const char* szName) :
	QWidget(pParent)
{
	Q_UNUSED(szName);

	setupUi(this);
	m_pList->setRootIsDecorated(false);
	m_pList->header()->setDefaultSectionSize(125);

	// Set initial values
	load();
}

/**
 * Class destructor.
 */
PrefFont::~PrefFont()
{
}

/**
 * Reads the current settings from the configuration object, and applies them
 * the the page's widget.
 */
void PrefFont::load()
{
	uint i;
	FontListItem* pItem;
	
	// Create a list item for every GUI element
	for (i = 0; i <= KScopeConfig::LAST_FONT; i++)
		pItem = new FontListItem(m_pList, (KScopeConfig::FontElement)i);
}

/**
 * Commits settings changes to the configuration object.
 */
void PrefFont::apply()
{
	QTreeWidgetItemIterator itr(m_pList);
	FontListItem* pItem;
	
	// Create a list item for every GUI element
	while (*itr){
		pItem = (FontListItem*)(*itr++);
		Config().setFont(pItem->getElement(), pItem->getFont());
	}
}

/**
 * Displays a font selection dialogue when an item is selected.
 * If the user chooses a new font, it is set to the selected item.
 * This slot is connected to both the doubleClicked() and the returnPressed()
 * signals of the list view.
 * @param	pItem	The selected item
 *		column  Not used
 */
void PrefFont::slotItemSelected(QTreeWidgetItem* pItem, int /* column */)
{
	FontListItem* pFontItem;
	QFont font;
	
	pFontItem = (FontListItem*)pItem;
	font = pFontItem->getFont();
	if (KFontDialog::getFont(font) == QDialog::Accepted) {
		pFontItem->setFont(font);
		emit modified();
	}
}

#include "preffont.moc"
