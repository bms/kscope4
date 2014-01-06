/***************************************************************************
 *
 * Copyright (C) 2007 Elad Lahav (elad_lahav@users.sourceforge.net)
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AS IS'' AND ANY EXPRESS OR
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

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <qtextstream.h>

#include <kmessagebox.h>
#include <kconfiggroup.h>
#include <klocale.h>

#include "project.h"
#include "kscopeconfig.h"
#include "cscopefrontend.h"

#define PROJECT_CONFIG_VER 2

inline void flListFromStringList(FileLocationList& fll, const QStringList& sl)
{
	QStringList::ConstIterator itr;
	QString sPath;
	uint nLine, nCol;

	// Transform the string into a list of file locations
	for (itr = sl.begin(); itr != sl.end(); ++itr) {
		sPath = (*itr).section(':', 0, 0);
		nLine = (*itr).section(':', 1, 1).toUInt();
		nCol = (*itr).section(':', 2, 2).toUInt();
		fll.append(new FileLocation(sPath, nLine, nCol));
	}
}

inline void stringListFromFlList(QStringList& sl, const FileLocationList& fll)
{
	FileLocation* pLoc;
	QString sLoc;

	// Nasty...
	sl.clear();

	// Turn the object list into a string list, so that it can be written in
	// the configuration file

	QListIterator<FileLocation*> itr(fll);

	while (itr.hasNext()){
		pLoc = itr.next();
		sLoc = "";
		QTextStream(&sLoc) << pLoc->m_sPath << ":" << pLoc->m_nLine << ":" 
				<< pLoc->m_nCol;
		sl.append(sLoc);
	}
}

/**
 */
Project::Project() : ProjectBase(),
	m_pConf(NULL)
{
}

/**
 */
Project::~Project()
{
	close();
}

/**
 */
bool Project::open(const QString& sPath)
{
	QString sConfFile;
	KConfigGroup pConfigGrp;
	Options opt;
	
	// Associate the object with the project directory
	m_dir.setPath(sPath);
	if (!m_dir.exists()) {
		KMessageBox::error(0, i18n("Project directory does not exist"));
		return false;
	}
	
	// Initialise the file-list file object
	m_fiFileList.setFileName(sPath + "/cscope.files");

	// Open the configuration files
	m_pConf = new KConfig(sPath + "/cscope.proj");

	// Verify the configuration file's version is compatible
	pConfigGrp = m_pConf->group("");
	if (pConfigGrp.readEntry("Version", 0) != PROJECT_CONFIG_VER) {
		KMessageBox::error(0, i18n("Your project is not compatible with this "
				"version of KScope.\nPlease re-create the project."));
		return false;
	}
	
	// Get the project name
	pConfigGrp = m_pConf->group("Project");
	m_sName = pConfigGrp.readEntry("Name");
	if (m_sName == QString::null) {
		KMessageBox::error(0, i18n("Cannot read project name"));
		return false;
	}
	
	// Get stored options
	initOptions();
	
	// Set default make values for new projects (overriden in loadSession(), 
	// which is not called for new projects)
	m_sMakeRoot = getSourceRoot();
	m_sMakeCmd = "make";
	
	return true;
}

/**
 */
void Project::close()
{
	if (m_pConf)
		delete m_pConf;

	m_fiFileList.close();
}

/**
 * Returns a semi-colon separated list of the file types included in the
 * current project.
 */
QString Project::getFileTypes() const
{
	return m_pConf->group("Project").readEntry("FileTypes");
}

/**
 * Reads the project's options from the configuration file.
 * @param	opt	A structure to fill with the read options
 */
void Project::getOptions(Options& opt) const
{
	KConfigGroup pConfGrp;

	// Get project properties
	pConfGrp = m_pConf->group("Project");
	opt.sSrcRootPath = pConfGrp.readEntry("RootPath", "/");
	opt.slFileTypes = pConfGrp.readEntry("FileTypes", QStringList());
	opt.bKernel = pConfGrp.readEntry("Kernel", DEF_IS_KERNEL);
	opt.bInvIndex = pConfGrp.readEntry("InvIndex", DEF_INV_INDEX);
	opt.bNoCompress = pConfGrp.readEntry("NoCompress", DEF_NO_COMPRESS);
	opt.bSlowPathDef = pConfGrp.readEntry("SlowPathDef", DEF_SLOW_PATH);
	opt.nAutoRebuildTime = pConfGrp.readEntry("AutoRebuildTime", 0);
	opt.nTabWidth = pConfGrp.readEntry("TabWidth", 8);
		
	// Get auto-completion options
	pConfGrp = m_pConf->group("AutoCompletion");
	opt.bACEnabled = pConfGrp.readEntry("Enabled", true);
	opt.nACMinChars = pConfGrp.readEntry("MinChars", DEF_AC_MIN_CHARS);
	opt.nACDelay = pConfGrp.readEntry("Delay", DEF_AC_DELAY);
	opt.nACMaxEntries = pConfGrp.readEntry("MaxEntries", DEF_AC_MAX_ENTRIES);
}

/**
 * Sets project options.
 * @param	opt	A structure containing the new parameters to set
 */
void Project::setOptions(const Options& opt)
{
	// Write the options to the configuration nfile
	writeOptions(m_pConf, opt);
				
	// Update project parameters
	initOptions();
}

/**
 */
void Project::loadSession(Session& sess)
{
	QStringList slEntry;
	KConfigGroup pConfGrp;
	
	pConfGrp = m_pConf->group("Session");
	
	// Read the list of open file locations
	slEntry = pConfGrp.readEntry("OpenFiles", QStringList());
	flListFromStringList(sess.fllOpenFiles, slEntry);
	
	// Get the path of the last viewed file
	sess.sLastFile = pConfGrp.readEntry("LastOpenFile", QString());
	
	// Read the lists of locked query files and call-tree/graph files
	sess.slQueryFiles = pConfGrp.readEntry("QueryFiles", QStringList());
	sess.slCallTreeFiles = pConfGrp.readEntry("CallTreeFiles", QStringList());
	
	// Read the list of bookmarks
	slEntry = pConfGrp.readEntry("Bookmarks", QStringList());
	flListFromStringList(sess.fllBookmarks, slEntry);
	
	// Read make-related information
	sess.sMakeCmd = pConfGrp.readEntry("MakeCommand", "make");
	sess.sMakeRoot = pConfGrp.readEntry("MakeRoot", getSourceRoot());
	
	// Cache make values
	m_sMakeCmd = sess.sMakeCmd;
	m_sMakeRoot = sess.sMakeRoot;
}

/**
 * Saves session-related information in the project's configuration file.
 * @param	sess	Session parameters
 */
void Project::storeSession(const Session& sess)
{
	QStringList slEntry;
	KConfigGroup pConfGrp;
	
	pConfGrp = m_pConf->group("Session");
	
	// Write the list of open file locations
	stringListFromFlList(slEntry, sess.fllOpenFiles);
	pConfGrp.writeEntry("OpenFiles", slEntry);
	
	// Write the path of the last viewed file
	pConfGrp.writeEntry("LastOpenFile", sess.sLastFile);
	
	// Write the lists of locked query files and call-tree/graph files
	pConfGrp.writeEntry("QueryFiles", sess.slQueryFiles);
	pConfGrp.writeEntry("CallTreeFiles", sess.slCallTreeFiles);
	
	// Write the list of bookmarks
	stringListFromFlList(slEntry, sess.fllBookmarks);
	pConfGrp.writeEntry("Bookmarks", slEntry);
	
	// Write make-related information
	// Be careful not to write empty strings, as they may occur if the make
	// dialogue was not invoked during this session
	if (!sess.sMakeCmd.isEmpty())
		pConfGrp.writeEntry("MakeCommand", sess.sMakeCmd);
	if (!sess.sMakeRoot.isEmpty())
		pConfGrp.writeEntry("MakeRoot", sess.sMakeRoot);
}

/**
 * Fills a list object with all files in the project.
 * List items are created by reading and parsing all file name entries from
 * the project's 'cscope.files' file.
 * Note that the file may contain option lines, beginning with a dash. These
 * should be ignored.
 * @param	pList	Pointer to the object to fill
 */
bool Project::loadFileList(FileListTarget* pList)
{
	QString sFilePath;
	
	// Open the 'cscope.files' file
	if (!m_fiFileList.open(IO_ReadOnly))
		return false;

	// Read all file names from the file
	QTextStream str(&m_fiFileList);
	while ((sFilePath = str.readLine()) != QString::null) {
		// Skip option lines
		if (sFilePath.at(0) == '-')
			continue;

		// Set the new list item
		pList->addItem(sFilePath);
	}

	m_fiFileList.close();
	return true;
}

/**
 * Writes all file entries in a list view widget to the project's
 * 'cscope.files' file (replacing current file contents.)
 * @param	pList	Pointer to the object from which to take the new entries
 */
bool Project::storeFileList(FileListSource* pList)
{
	QString sFilePath;
	
	// Open the 'cscope.files' file
	if (!m_fiFileList.open(IO_WriteOnly | IO_Truncate))
		return false;

	QTextStream str(&m_fiFileList);

	// Write all file names
	if (pList->firstItem(sFilePath)) {
		do {
			str << sFilePath << "\n";
		} while (pList->nextItem(sFilePath));
	}

	m_fiFileList.close();
	return true;
}

/**
 * Adds a single file to the file list.
 * @param	sPath	The path of the file to add
 * @return	true if successful, false otherwise
 */
bool Project::addFile(const QString& sPath)
{
	// Open the 'cscope.files' file
	if (!m_fiFileList.open(IO_WriteOnly | IO_Append))
		return false;
	
	// Write the file path
	QTextStream str(&m_fiFileList);
	str << sPath << "\n";

	m_fiFileList.close();
	return true;	
}

/**
 * Determines whether the project includes any files.
 * Reads the 'cscope.files' file and looks for any file names in it. If none
 * is present, then the project is considered empty.
 * Note that the file may contain option lines, beginning with a dash. These
 * should be ignored.
 * @return	true if no files are included in the project, false otherwise
 */
bool Project::isEmpty()
{
	QString sPath, sFileName;
	bool bResult = true;
	
	// Open the 'cscope.files' file
	if (!m_fiFileList.open(IO_ReadOnly))
		return true;

	// Find at least one file name entry in the file
	QTextStream str(&m_fiFileList);
	while ((sPath = str.readLine()) != QString::null) {
		if (sPath.at(0) != '-') {
			bResult = false;
			break;
		}
	}

	m_fiFileList.close();
	return bResult;
}

/**
 * Copies the list of previously queried symbols to the target object.
 * @param	slSymHistory	The list object to copy into
 */
void Project::getSymHistory(QStringList& slSymHistory) const
{
	slSymHistory = m_slSymHistory;
}

/**
 * Copies the list of previously queried symbols from the target object.
 * @param	slSymHistory	The list object to copy from
 */
void Project::setSymHistory(QStringList& slSymHistory)
{
	m_slSymHistory = slSymHistory;
}

void Project::getMakeParams(QString& sCmd, QString& sDir) const
{
	sCmd = m_sMakeCmd;
	sDir = m_sMakeRoot;
}

/**
 * Creates a project by writing a configuration file inside the given
 * directory.
 * @param	sName	The project's name
 * @param	sPath	The full path of the project's directory
 * @param	opt		Project options
 */
bool Project::create(const QString& sName, const QString& sPath,
	const Options& opt)
{
	// Prepare the project's files
	KConfig conf(sPath + "/cscope.proj");
	KConfigGroup pConfGrp;

	// Write the configuration file version
	pConfGrp = conf.group("");
	pConfGrp.writeEntry("Version", PROJECT_CONFIG_VER);
	
	// Write project properties in the configuration file
	pConfGrp = conf.group("Project");
	pConfGrp.writeEntry("Name", sName);
	writeOptions(&conf, opt);
	
	// Flush the config file data, so the project is created even if KScope
	// crashes...
	conf.sync();

	return true;
}

void Project::writeOptions(KConfig* pConf, const Options& opt)
{
	KConfigGroup pConfGrp;

	pConfGrp = pConf->group("Project");
	pConfGrp.writeEntry("RootPath", opt.sSrcRootPath);
	pConfGrp.writeEntry("FileTypes", opt.slFileTypes.join(" "));
	pConfGrp.writeEntry("Kernel", opt.bKernel);
	pConfGrp.writeEntry("InvIndex", opt.bInvIndex);		
	pConfGrp.writeEntry("NoCompress", opt.bNoCompress);		
	pConfGrp.writeEntry("SlowPathDef", opt.bSlowPathDef);		
	pConfGrp.writeEntry("AutoRebuildTime", opt.nAutoRebuildTime);
	pConfGrp.writeEntry("TabWidth", opt.nTabWidth);
	
	// Set auto-completion options
	pConfGrp = pConf->group("AutoCompletion");
	pConfGrp.writeEntry("Enabled", opt.bACEnabled);
	pConfGrp.writeEntry("MinChars", opt.nACMinChars);
	pConfGrp.writeEntry("Delay", opt.nACDelay);
	pConfGrp.writeEntry("MaxEntries", opt.nACMaxEntries);
}
