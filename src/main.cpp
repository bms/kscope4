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

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocalizedstring.h>

#include "kscope.h"
#include "kscopeconfig.h"

static KLocalizedString description =
	ki18n("KScope\nA source-editing environment for KDE, based on Cscope");

KCmdLineOptions options;

/**
 * Defines the programme's entry point.
 * Creates KScope's main window, and starts the event loop.
 * @param	argc	Number of command line arguments
 * @param	argv	Command line arguments
 * @return	Programme's exit value
 */
int main(int argc, char *argv[])
{
	// Create the "About" dialogue
	KAboutData aboutData( QByteArray("kscope"), QByteArray(), ki18n("KScope"),
		QByteArray(VERSION), description, KAboutData::License_BSD,
		ki18n("(c) 2003-2007, Elad Lahav"), KLocalizedString(),
		QByteArray("http://kscope.sourceforge.net"),
		QByteArray("elad_lahav@users.sf.net"));
	aboutData.addAuthor(ki18n("Elad Lahav"),
			ki18n("Developer"),
			QByteArray("elad_lahav@users.sf.net"));
	aboutData.addCredit(ki18n("Albert Yosher"),
			ki18n("Code completion, patches and bug fixes"),
			QByteArray("ayosher@users.sf.net"));
	aboutData.addCredit(ki18n("Gabor Fekete"),
			ki18n("Bug fixes and patches"),		
			QByteArray("feketgai@index.hu"));

	// Initialise command-line argument parsing
	KCmdLineArgs::init(argc, argv, &aboutData);

	options.add("+[CSCOPE.OUT path]", 
		ki18n("Opens a cscope.out file in a temporary project"), QByteArray());
	options.add("+[CSCOPE.PROJ path | KScope project directory path]", 
		ki18n("Opens a KScope project"), QByteArray());

	KCmdLineArgs::addCmdLineOptions(options);

	// Parse command line arguments
	KCmdLineArgs* pArgs = KCmdLineArgs::parsedArgs();

	// Create the main window
	KApplication a;
	KScope* pKScope = new KScope();

	// Display the main window
	pKScope->show();  

	// Handle command-line arguments
	if (pArgs->count() > 0) {
		pKScope->parseCmdLine(pArgs);
	} else if (Config().getLoadLastProj()) {
		// No arguments given, load the most recent project
		pKScope->openLastProject();
	}

	// Make sure Cscope is properly installed
	pKScope->verifyCscope();

	// Start the event loop
	return a.exec();
}
