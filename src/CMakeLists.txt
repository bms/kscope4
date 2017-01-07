include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
)

set(kscope_SRCS 
    bookmarksdlg.cpp
    calltreedlg.cpp
    calltreemanager.cpp
    configfrontend.cpp
    cscopefrontend.cpp
    cscopemsgdlg.cpp
    ctagsfrontend.cpp
    ctagslist.cpp
    dirscanner.cpp
    dotfrontend.cpp
    editormanager.cpp
    editorpage.cpp
    editortabs.cpp
    encoder.cpp
    filelist.cpp
    fileview.cpp
    frontend.cpp
    graphedge.cpp
    graphnode.cpp
    graphprefdlg.cpp
    graphwidget.cpp
    historypage.cpp
    historyview.cpp
    kscopeactions.cpp
    kscopeconfig.cpp
    kscope.cpp
    kscopepixmaps.cpp
    main.cpp
    makedlg.cpp
    makefrontend.cpp
    newprojectdlg.cpp
    openprojectdlg.cpp
    prefcolor.cpp
    preferencesdlg.cpp
    preffont.cpp
    preffrontend.cpp
    prefopt.cpp
    progressdlg.cpp
    projectbase.cpp
    project.cpp
    projectfilesdlg.cpp
    projectmanager.cpp
    querypagebase.cpp
    querypage.cpp
    queryresultsmenu.cpp
    queryview.cpp
    queryviewdlg.cpp
    queryviewdriver.cpp
    querywidget.cpp
    scanprogressdlg.cpp
    searchlist.cpp
    searchresultsdlg.cpp
    setenvdlg.cpp
    symbolcompletion.cpp
    symboldlg.cpp
    tabwidget.cpp
    treewidget.cpp
    welcomedlg.cpp
)

set(kscope_PICS
    bookmark.png
    called_tree.png
    call_graph.png
    calling_tree.png
    file_ro.png
    file_rw.png
    file_save.png
    query_locked.png
    query_unlocked.png
    tab_list.png
)

kde4_add_ui_files(kscope_SRCS
    autocompletionlayout.ui
    bookmarkslayout.ui
    calltreelayout.ui
    cscopemsglayout.ui
    fileviewlayout.ui
    graphpreflayout.ui
    makelayout.ui
    newprojectlayout.ui
    openprojectlayout.ui
    prefcolorlayout.ui
    preffontlayout.ui
    preffrontendlayout.ui
    prefoptlayout.ui
    projectfileslayout.ui
    queryviewlayout.ui
    querywidgetlayout.ui
    scanprogresslayout.ui
    searchresultslayout.ui
    symbollayout.ui
    welcomedlg.ui
)

kde4_add_executable(kscope ${kscope_SRCS})

target_link_libraries(kscope
    ${KDE4_KTEXTEDITOR_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KFILE_LIBS}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${KDE4_KDE3SUPPORT_LIBS}
    ${QT_QT3SUPPORT_LIBRARY}
)

install( TARGETS kscope DESTINATION ${KDE4_BIN_INSTALL_DIR} )
install( FILES kscope.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES kscopeui.rc kscope_config DESTINATION  ${KDE4_DATA_DIR}/kscope )

install( FILES ${kscope_PICS} DESTINATION ${DATA_INSTALL_DIR}/kscope/pics )
kde4_install_icons( ${ICON_INSTALL_DIR} )
kde4_update_iconcache()
