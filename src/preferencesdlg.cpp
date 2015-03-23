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

#include <qlayout.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <kcolorbutton.h>
#include <kmessagebox.h>
#include <kfontrequester.h>
#include <kvbox.h>

#include "preferencesdlg.h"
#include "preffrontend.h"
#include "prefcolor.h"
#include "preffont.h"
#include "prefopt.h"
#include "kscopeconfig.h"
#include "cscopefrontend.h"
#include "ctagsfrontend.h"
#include "dotfrontend.h"

/**
 * Class constructor.
 * @param	nPage	The initial page to show
 * @param	pParent	The parent widget
 * @param	szName	The widget's name
 */
PreferencesDlg::PreferencesDlg(uint nPage, QWidget* pParent, 
	const char* szName) :
	KPageDialog(pParent),
	m_lDlgPageItems(QList<KPageWidgetItem*>())
{
	KVBox* pFrame;
	KPageWidgetItem *item;

	// Setup KPageDialog sub-widget
	setFaceType(KPageDialog::List);
	setCaption(i18n("Preferences"));
	setButtons(KDialog::Default | KDialog::Ok | KDialog::Apply | KDialog::Cancel);
	setDefaultButton(KDialog::Ok);
	setObjectName(szName);
	QDialog::setModal(false);

	// Create and add the "Frontend" page
	pFrame = new KVBox();
	item = addPage(pFrame, i18n("Programs"));
	item->setHeader(i18n("Paths to back-end programmes"));
	item->setIcon(KIcon("preferences-desktop-default-applications", KIconLoader::global()));
	m_lDlgPageItems << item;
	m_pPrefFrontend = new PrefFrontend(pFrame);

	// Create and add the "Colours" page
	pFrame = new KVBox();
	item = addPage(pFrame, i18n("Colours"));
	item->setHeader(i18n("Window colours"));
	item->setIcon(KIcon("preferences-desktop-color", KIconLoader::global()));
	m_lDlgPageItems << item;
	m_pPrefColor = new PrefColor(pFrame);

	// Create and add the "Fonts" page
	pFrame = new KVBox();
	item = addPage(pFrame, i18n("Fonts"));
	item->setHeader(i18n("Window fonts"));
	item->setIcon(KIcon("preferences-desktop-font", KIconLoader::global()));
	m_lDlgPageItems << item;
	m_pPrefFont = new PrefFont(pFrame);

	// Create and add the "Options" page
	pFrame = new KVBox();
	item = addPage(pFrame, i18n("Options"));
	item->setHeader(i18n("Misc. Options"));
	item->setIcon(KIcon("configure", KIconLoader::global()));
	m_lDlgPageItems << item;
	m_pPrefOpt = new PrefOpt(pFrame);

	// Make sure the "Apply" button is initially disabled
	enableButtonApply(false);

	// Enable the "Apply" button when a parameter changes its value
	connect(m_pPrefFrontend, SIGNAL(modified()), this, SLOT(slotModified()));
	connect(m_pPrefColor, SIGNAL(modified()), this, SLOT(slotModified()));
	connect(m_pPrefFont, SIGNAL(modified()), this, SLOT(slotModified()));
	connect(m_pPrefOpt, SIGNAL(modified()), this, SLOT(slotModified()));

	// Show the ( new ) current page
	setCurrentPage(m_lDlgPageItems[nPage]);
}

/**
 * Class destructor.
 */
PreferencesDlg::~PreferencesDlg()
{
}

/**
 * Updates the dialog's widgets with the current configuration parameters.
 */
void PreferencesDlg::loadConfig()
{
	m_pPrefFrontend->load();
	m_pPrefColor->load();
	m_pPrefFont->load();
	m_pPrefOpt->load();
}

/**
 * Sets the configured parameters to the global configuration object.
 * This method is called when either the "OK" or the "Apply" button are
 * clicked. Before the new settings are applied, their values are verified.
 * @return	true if the new parameters were applied successfully, false
 *			otherwise
 */
bool PreferencesDlg::updateConfig()
{
	// Verify configured paths lead to the executables
	if (!verifyPaths())
		return false;

	// Apply the changes
	m_pPrefFrontend->apply();
	m_pPrefColor->apply();
	m_pPrefFont->apply();
	m_pPrefOpt->apply();

	emit applyPref();
	return true;
}

/**
 * Tests whether the paths set for Cscope and Ctags lead to executables.
 * Cscope is verified by a different process.
 */
bool PreferencesDlg::verifyPaths()
{
	return (CtagsFrontend::verify(m_pPrefFrontend->m_pCtagsURL->url().pathOrUrl()));
}

/**
 * - The user clicks the dialogue's "OK" button : updates the global configuration based
 *   on the values given in the preferences dialogue, and then closes the dialogue.
 * - The user clicks the dialogue's "Apply" : updates the global configuration based on
 *   the values given in the preferences dialogue, leaving the dialogue open.
 * - The user clicks the "Default" button : resets all configuration parameters to their
 *   default values.
 */
void PreferencesDlg::slotButtonClicked(int button)
{
	switch (button){
		case KDialog::Ok :
		case KDialog::Apply :
			if (updateConfig())
				enableButtonApply(false);
			accept();
			break;

		case KDialog::Default:
			// Prompt the user before applying default values
			if (KMessageBox::questionYesNo(0,
				i18n("This would reset all your "
				     "configuration settings! Continue?")) == KMessageBox::Yes) {
				// Load the default values
				Config().loadDefault();
				loadConfig();

				// Apply the default values
				if (updateConfig())
					enableButtonApply(false);
			}
			accept();
			break;

		default:
			KDialog::slotButtonClicked(button);
			break;
	}
}

/**
 * Enables the "Apply" button.
 */
void PreferencesDlg::slotModified()
{
	enableButtonApply(true);
}

#include "preferencesdlg.moc"
