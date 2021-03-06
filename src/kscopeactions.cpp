/***************************************************************************
 *
 * Copyright (C) 2007 Elad Lahav (elad_lahav@users.sourceforge.net)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, m_pWindow list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, m_pWindow list of conditions and the following disclaimer in the
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

#include <QtGui/QDockWidget>
#include <QtGui/QToolBar>

#include <klocale.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kxmlguifactory.h>

#include "kscope.h"
#include "kscopeactions.h"
#include "kscopeconfig.h"
#include "filelist.h"
#include "fileview.h"
#include "editortabs.h"
#include "querywidget.h"

KScopeActions::KScopeActions(KScope* pWindow) : QObject(),
		m_pWindow(pWindow),
		m_pCollection(pWindow->actionCollection())
{
}

KScopeActions::~KScopeActions()
{
}

/**
 * Connects menu bar and toolbar commands with class members. These members
 * handle the actions associated with each command.
 */
void KScopeActions::init()
{
	// File menu
	m_pCollection->addAction(KStandardAction::New, m_pWindow, SLOT(slotNewFile()));
	m_pCollection->addAction(KStandardAction::Open, m_pWindow, SLOT(slotOpenFile()));
	m_pCollection->addAction(KStandardAction::Close, m_pWindow, SLOT(slotCloseEditor()));
	m_pCollection->addAction(KStandardAction::Quit, m_pWindow, SLOT(slotClose()));

	addAction(i18n("Go to File List"),
		NULL,
		"Ctrl+Shift+O",
		m_pWindow->m_pFileList,
		SLOT(slotSetFocus()),
		"file_open_file_from_list",
		SIGNAL(toggleProject(bool)));

	addAction(i18n("Save Al&l"),
		"document-save-all",
		"Ctrl+L",
		m_pWindow->m_pEditTabs,
		SLOT(slotSaveAll()),
		"file_save_all",
		NULL);

	// Edit menu
	m_pExtEditAction = addAction(i18n("Edit in E&xternal Editor"),
		NULL,
		"Ctrl+E",
		m_pWindow,
		SLOT(slotExtEdit()),
		"edit_external_editor",
		SIGNAL(toggleFile(bool)));

	addAction(i18n("Go To Tag"),
		NULL,
		"Ctrl+Shift+T",
		m_pWindow,
		SLOT(slotGotoTag()),
		"edit_goto_tag",
		SIGNAL(toggleFile(bool)));

	addAction(i18n("Complete Symbol"),
		NULL,
		"Ctrl+Space",
		m_pWindow,
		SLOT(slotCompleteSymbol()),
		"edit_comp_symbol",
		SIGNAL(toggleFile(bool)));

	// Project menu
	addAction(i18n("&New Project..."),
		NULL,
		NULL,
		m_pWindow,
		SLOT(slotCreateProject()),
		"project_new",
		NULL);

	addAction(i18n("&Open Project..."),
		"project-open",
		NULL,
		m_pWindow,
		SLOT(slotOpenProject()),
		"project_open",
		NULL);

	addAction(i18n("Open &Cscope.out..."),
		NULL,
		NULL,
		m_pWindow,
		SLOT(slotProjectCscopeOut()),
		"project_cscope_out",
		NULL);

	addAction(i18n("Add/Remove &Files..."),
		NULL,
		NULL,
		m_pWindow,
		SLOT(slotProjectFiles()),
		"project_add_rem_files",
		SIGNAL(toggleProject(bool)));

	addAction(i18n("&Properties..."),
		NULL,
		NULL,
		m_pWindow,
		SLOT(slotProjectProps()),
		"project_properties",
		SIGNAL(toggleProject(bool)));

	addAction(i18n("&Make Project"),
		"run-build",
		"Ctrl+M",
		m_pWindow,
		SLOT(slotProjectMake()),
		"project_make",
		SIGNAL(toggleProject(bool)));

	addAction(i18n("&Remake Project"),
		"run-build",
		"Ctrl+Shift+M",
		m_pWindow,
		SLOT(slotProjectRemake()),
		"project_remake",
		SIGNAL(toggleProject(bool)));

	addAction(i18n("&Close Project"),
		"window-close",
		NULL,
		m_pWindow,
		SLOT(slotCloseProject()),
		"project_close",
		SIGNAL(toggleProject(bool)));

	// Cscope menu
	addAction(i18n("&References..."),
		NULL,
		"Ctrl+0",
		m_pWindow,
		SLOT(slotQueryReference()),
		"cscope_references",
		SIGNAL(toggleProject(bool)));

	addAction(i18n("&Definition..."),
		NULL,
		"Ctrl+1",
		m_pWindow,
		SLOT(slotQueryDefinition()),
		"cscope_definition",
		SIGNAL(toggleProject(bool)));

	addAction(i18n("&Called Functions..."),
		NULL,
		"Ctrl+2",
		m_pWindow,
		SLOT(slotQueryCalled()),
		"cscope_called",
		SIGNAL(toggleProject(bool)));

	addAction(i18n("C&alling Functions..."),
		NULL,
		"Ctrl+3",
		m_pWindow,
		SLOT(slotQueryCalling()),
		"cscope_calling",
		SIGNAL(toggleProject(bool)));

	addAction(i18n("Find &Text..."),
		NULL,
		"Ctrl+4",
		m_pWindow,
		SLOT(slotQueryText()),
		"cscope_text",
		SIGNAL(toggleProject(bool)));

	addAction(i18n("Find &EGrep Pattern..."),
		NULL,
		"Ctrl+5",
		m_pWindow,
		SLOT(slotQueryPattern()),
		"cscope_pattern",
		SIGNAL(toggleProject(bool)));

	addAction(i18n("Find &File..."),
		NULL,
		"Ctrl+7",
		m_pWindow,
		SLOT(slotQueryFile()),
		"cscope_file",
		SIGNAL(toggleProject(bool)));

	addAction(i18n("&Including Files..."),
		NULL,
		"Ctrl+8",
		m_pWindow,
		SLOT(slotQueryIncluding()),
		"cscope_including",
		SIGNAL(toggleProject(bool)));

	addAction(i18n("&Quick Definition"),
		NULL,
		"Ctrl+]",
		m_pWindow,
		SLOT(slotQueryQuickDef()),
		"cscope_quick_def",
		SIGNAL(toggleProject(bool)));

	addAction(i18n("Call &Graph..."),
		NULL,
		"Ctrl+\\",
		m_pWindow,
		SLOT(slotCallTree()),
		"cscope_call_tree",
		SIGNAL(toggleProject(bool)));

	addAction(i18n("Re&build database"),
		"svn-update",
		NULL,
		m_pWindow,
		SLOT(slotRebuildDB()),
		"cscope_rebuild",
		SIGNAL(toggleProject(bool)));

	// View menu
	initPopups();

	// Go menu
	addAction(i18n("P&revious Result"),
		"go-up",
		"Alt+Up", 
		m_pWindow->m_pQueryWidget,
		SLOT(slotPrevResult()),
		"go_prev_result",
		SIGNAL(toggleProject(bool)));

	addAction(i18n("N&ext Result"),
		"go-down",
		"Alt+Down",
		m_pWindow->m_pQueryWidget,
		SLOT(slotNextResult()),
		"go_next_result",
		SIGNAL(toggleProject(bool)));

	addAction(i18n("&Previous Position"),
		"go-previous", 
		"Alt+Left",
		m_pWindow->m_pQueryWidget,
		SLOT(slotHistoryPrev()),
		"go_prev_pos",
		NULL);

	addAction(i18n("&Next Position"),
		"go-next", 
		"Alt+Right",
		m_pWindow->m_pQueryWidget,
		SLOT(slotHistoryNext()),
		"go_next_pos",
		NULL);

	addAction(i18n("Position &History"),
		"view-history",
		"Ctrl+h",
		m_pWindow,
		SLOT(slotHistoryShow()),
		"go_history",
		NULL);

	addAction(i18n("Global &Bookmarks"),
		"bookmark", 
		"Ctrl+Shift+G",
		m_pWindow,
		SLOT(slotShowBookmarks()),
		"go_bookmarks",
		NULL);

	// Window menu
	addAction(i18n("Close &All"),
		"window-close",
		NULL,
		m_pWindow,
		SLOT(slotCloseAllWindows()),
		"window_close_all",
		NULL);

	addAction(i18n("Go &Left"),
		"go-previous",
		"Alt+Shift+Left",
		m_pWindow->m_pEditTabs,
		SLOT(slotGoLeft()),
		"window_go_left",
		NULL);

	addAction(i18n("Go &Right"),
		"go-next",
		"Alt+Shift+Right",
		m_pWindow->m_pEditTabs,
		SLOT(slotGoRight()),
		"window_go_right",
		NULL);

	// Settings menu
	m_pCollection->addAction(KStandardAction::Preferences, m_pWindow, SLOT(slotConfigure()));
	m_pCollection->addAction(KStandardAction::KeyBindings, m_pWindow, SLOT(slotShortcuts()));

	// Help menu
	addAction(i18n("Show &Welcome Message..."),
		NULL,
		NULL,
		m_pWindow,
		SLOT(slotShowWelcome()),
		"help_welcome",
		NULL);

	// Query widget popup menu
	addAction(i18n("&New"),
		"tab-new",
		NULL,
		m_pWindow->m_pQueryWidget,
		SLOT(slotNewQueryPage()),
		"query_new",
		SIGNAL(toggleProject(bool)));

	addAction(i18n("&Refresh"),
		"view-refresh",
		NULL,
		m_pWindow->m_pQueryWidget,
		SLOT(slotRefreshCurrent()),
		"query_refresh",
		SIGNAL(toggleProject(bool)));

	m_pLockAction = addToggle(i18n("&Lock/Unlock"),
		"encrypted",
		NULL,
		m_pWindow->m_pQueryWidget,
		SLOT(slotLockCurrent()),
		"query_toggle_locked",
		SIGNAL(toggleProject(bool)));

	addAction(i18n("&Close"),
		"fileclose",
		NULL,
		m_pWindow->m_pQueryWidget,
		SLOT(slotCloseCurrent()),
		"query_close",
		SIGNAL(toggleProject(bool)));

	m_pExtEditAction->setEnabled(Config().useExtEditor());
}

void KScopeActions::initPopups()
{
	m_pToggleFileViewAction = addToggle(i18n("Toggle File List"),
		"view-right-close",
		"Ctrl+/",
		m_pWindow->m_pFileViewDock,
		SLOT(setVisible(bool)),
		"view_toggle_filelist_dock",
		NULL);
	connect(m_pWindow->m_pFileViewDock, SIGNAL(visibilityChanged(bool)),
		m_pToggleFileViewAction, SLOT(setChecked(bool)));

	m_pToggleQueryWindowAction = addToggle(i18n("Toggle Query Window"),
		"view-split-top-bottom",
		"Ctrl+.",
		m_pWindow->m_pQueryDock,
		SLOT(setVisible(bool)),
		"view_toggle_query_dock",
		NULL);
	connect(m_pWindow->m_pQueryDock, SIGNAL(visibilityChanged(bool)),
		m_pToggleQueryWindowAction, SLOT(setChecked(bool)));

	m_pToggleTagListAction = addToggle(i18n("Toggle Tag List"),
		"view-left-close",
		"Ctrl+'",
		m_pWindow->m_pEditTabs,
		SLOT(slotToggleTagList(bool)),
		"view_toggle_tag_list",
		NULL);
}

void KScopeActions::initLayoutActions()
{
	m_pToggleFileViewAction->setChecked(! m_pWindow->m_pFileViewDock->isHidden());
	m_pToggleQueryWindowAction->setChecked(! m_pWindow->m_pQueryDock->isHidden());
	m_pToggleTagListAction->setChecked(Config().getShowTagList());
}

/**
 * Enables/disables the "Edit in External Editor" command.
 * @param	bEnable	true to enable the command, false to disable it
 */
void KScopeActions::enableExtEditor(bool bEnable)
{
	m_pExtEditAction->setEnabled(bEnable);
}

void KScopeActions::slotQueryDockToggled(bool bVisible)
{
	m_pToggleQueryWindowAction->setChecked(bVisible);
}

/**
 * Enables/disables all actions related to open projects.
 * This slot should be called whenever a project is opened or closed.
 * @param	bEnable	true to enable actions, false to disable
 */
void KScopeActions::slotEnableProjectActions(bool bEnable)
{
	emit toggleProject(bEnable);
}

/**
 * Enables/disables all actions related to open files.
 * This slot should be called the first file is opened, or when the last one
 * is closed.
 * @param	bEnable	true to enable actions, false to disable
 */
void KScopeActions::slotEnableFileActions(bool bEnable)
{
	emit toggleFile(bEnable);
}

/**
 * Creates a new action.
 * @param	sCaption	The text to display in the menu item
 * @param	szIcon		Optional icon associated with the action
 * @param	szShortcut	Optional key-combination string
 * @param	pReceiver	The widget to receive the action's signal
 * @param	szSlot		The widget's slot that connect to the signal
 * @param	szName		The XML entry corresponding to the action
 * @param	szSignal	Optional signal to connect to the setEnabled() slot of
 *						the action
 * @return	The newly created action object
 */
KAction* KScopeActions::addAction(const QString& sCaption, const char* szIcon,
	const char* szShortcut, QWidget* pReceiver, const char* szSlot,
	const char* szName, const char* szSignal)
{
	KAction* pAction = m_pCollection->addAction(QString(szName));;

	// Create the action
	pAction->setText(sCaption);
	if (szIcon != NULL)
		pAction->setIcon(KIcon(szIcon));
	pAction->setShortcut(szShortcut == NULL ? KShortcut() : KShortcut(szShortcut));
	connect(pAction, SIGNAL(triggered()), pReceiver, szSlot);

	// Add to the given action list, if any
	if (szSignal)
		connect(this, szSignal, pAction, SLOT(setEnabled(bool)));

	return pAction;
}

/**
 * Creates a new toggle action.
 * @param	sCaption	The text to display in the menu item
 * @param	szIcon		Optional icon associated with the action
 * @param	szShortcut	Optional key-combination string
 * @param	pReceiver	The widget to receive the action's signal
 * @param	szSlot		The widget's slot that connect to the signal
 * @param	szName		The XML entry corresponding to the action
 * @param	szSignal	Optional signal to connect to the setEnabled() slot of
 *						the action
 * @return	The newly created action object
 */
KToggleAction* KScopeActions::addToggle(const QString& sCaption,
	const char* szIcon, const char* szShortcut, QWidget* pReceiver,
	const char* szSlot, const char* szName, const char* szSignal)
{
	KToggleAction* pAction;

	// Create the action in m_pCollection
	pAction = new KToggleAction(((szIcon != NULL) ? KIcon(szIcon) : KIcon()),
			sCaption, pReceiver);
	pAction->setShortcut(szShortcut == NULL ? KShortcut() : KShortcut(szShortcut));
	m_pCollection->addAction(QString(szName), pAction);
	connect(pAction, SIGNAL(triggered(bool)), pReceiver, szSlot);

	// Add to the given action list, if any
	if (szSignal)
		connect(this, szSignal, pAction, SLOT(setEnabled(bool)));

	return pAction;
}
#include "kscopeactions.moc"

/*
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
