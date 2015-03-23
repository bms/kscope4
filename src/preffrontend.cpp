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

#include <qcheckbox.h>

#include <kurlrequester.h>
#include <klineedit.h>
#include <klocale.h>

#include "preffrontend.h"
#include "kscopeconfig.h"
#include "configfrontend.h"

/**
 * Class constructor.
 * @param	pParent	The parent widget
 * @param	szName	The widget's name
 */
PrefFrontend::PrefFrontend(QWidget* pParent, const char* szName) :
        QWidget(pParent)
{
	setupUi(this);
	setObjectName(szName);

	// Set initial values
	load();

	// Attempt to guess paths based on the user's PATH environment variable
	connect(m_pGuessButton, SIGNAL(clicked()), this,
		SLOT(slotGuessPaths()));	

	// Emit the modified() signal when a new path is set
	connect(m_pCscopeURL, SIGNAL(textChanged(const QString&)), this,
		SIGNAL(modified()));
	connect(m_pCtagsURL, SIGNAL(textChanged(const QString&)), this,
		SIGNAL(modified()));
	connect(m_pDotURL, SIGNAL(textChanged(const QString&)), this,
		SIGNAL(modified()));
}

/**
 * Class destructor.
 */
PrefFrontend::~PrefFrontend()
{
}

/**
 * Reads the current settings from the configuration object, and applies them
 * the the page's widget.
 */
void PrefFrontend::load()
{
	m_pCscopeURL->lineEdit()->setText(Config().getCscopePath());
	m_pCtagsURL->lineEdit()->setText(Config().getCtagsPath());
	m_pDotURL->lineEdit()->setText(Config().getDotPath());
}

/**
 * Commits settings changes to the configuration object.
 */
void PrefFrontend::apply()
{
	Config().setCscopePath(m_pCscopeURL->url().pathOrUrl());
	Config().setCtagsPath(m_pCtagsURL->url().pathOrUrl());
	Config().setDotPath(m_pDotURL->url().pathOrUrl());
}

/**
 * Emits the modified() signal whenever any of the paths edit widgets changes
 * its contents.
 * This slot is connected to the textChanged() signal of each of the path
 * edit widgets. By emitting the modified() signal, the widget notifies the
 * parent dialog it should enable the "Apply" button.
 */
void PrefFrontend::slotChanged(const QString&)
{
	emit modified();
}

/**
 * Checks the user's PATH environment variable for a Cscope and Ctags
 * executables.
 * This is done by running the kscope_config script.
 */
void PrefFrontend::slotGuessPaths()
{
	ConfigFrontend* pConf;

	// Start with an empty results text widget
	m_pScriptText->clear();

	// Create a frontend object for the script
	pConf = new ConfigFrontend(true);

	// Show tests and results in the text widget
	connect(pConf, SIGNAL(test(uint)), this,
		SLOT(slotAutoConfigTest(uint)));
	connect(pConf, SIGNAL(result(uint, const QString&)), this,
		SLOT(slotAutoConfigResult(uint, const QString&)));

	// Run the script
	pConf->run(m_pCscopeURL->url().pathOrUrl(), m_pCtagsURL->url().pathOrUrl(),
		m_pDotURL->url().pathOrUrl());
}

/**
 * Shows the test being executed by the script in the text widget.
 * This slot is connected to the test() signal of the ConfigFrontend object.
 * @param	nType	The type of test being executed
 */
void PrefFrontend::slotAutoConfigTest(uint nType)
{
	const char* msg;

	switch (nType) {
	case ConfigFrontend::CscopePath:
		msg = "Looking for Cscope...";
		break;

	case ConfigFrontend::CscopeVersion:
		msg = "Checking Cscope version...";
		break;

	case ConfigFrontend::CscopeVerbose:
		msg = "Cscope support for line mode verbose output...";
		break;

	case ConfigFrontend::CscopeSlowPath:
		msg = "Cscope support slow path definitions... ";
		break;

	case ConfigFrontend::CtagsPath:
		msg = "\nLooking for Ctags...";
		break;

	case ConfigFrontend::CtagsExub:
		msg = "Ctags compatibilty with ctags-exuberant...";
		break;

	case ConfigFrontend::DotPath:
		msg = "\nLooking for Dot...";
		break;

	case ConfigFrontend::DotPlain:
		msg = "Checking -Tplain...";
		break;

	case ConfigFrontend::END:
		return;
	}

	m_pScriptText->textCursor().insertText(QString(msg));
}

/**
 * Shows the result of a test executed by the configuration script, and
 * adjusts the configuration widgets accordingly.
 * @param	nType	The type of test that was executed
 * @param	sResult	The test's result
 */
void PrefFrontend::slotAutoConfigResult(uint nType, const QString& sResult)
{
	switch (nType) {
	case ConfigFrontend::CscopePath:
	case ConfigFrontend::CtagsPath:
	case ConfigFrontend::DotPath:
		m_pCscopeURL->lineEdit()->setText((sResult == "ERROR") ? "" : sResult);
		break;

	case ConfigFrontend::CscopeVersion:
	case ConfigFrontend::CtagsExub:
	case ConfigFrontend::DotPlain:
		if (sResult == "ERROR")
			m_pCscopeURL->lineEdit()->setText("");
		break;

	case ConfigFrontend::CscopeVerbose:
	case ConfigFrontend::CscopeSlowPath:
		break;
	}

	m_pScriptText->textCursor().insertText(sResult + "\n");
}

#include "preffrontend.moc"
