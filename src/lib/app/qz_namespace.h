#ifndef QZ_NAMESPACE_H
#define QZ_NAMESPACE_H

#ifdef QUPZILLA_SHAREDLIBRARY
#define QT_QUPZILLA_EXPORT Q_DECL_EXPORT
#else
#define QT_QUPZILLA_EXPORT Q_DECL_IMPORT
#endif

#include <QFlags>

namespace Qz
{

enum AppMessageType {
    AM_SetAdBlockIconEnabled,
    AM_CheckPrivateBrowsing,
    AM_ReloadSettings,
    AM_HistoryStateChanged,
    AM_BookmarksChanged,
    AM_StartPrivateBrowsing
};

enum BrowserWindow {
    BW_FirstAppWindow,
    BW_OtherRestoredWindow,
    BW_NewWindow
};

enum CommandLineAction {
    CL_NoAction,
    CL_OpenUrl,
    CL_StartWithProfile,
    CL_StartWithoutAddons,
    CL_NewTab,
    CL_NewWindow,
    CL_ShowDownloadManager,
    CL_StartPrivateBrowsing,
    CL_ExitAction
};

enum ObjectName {
    ON_WebView,
    ON_TabWidget,
    ON_TabBar,
    ON_QupZilla
};

enum NewTabPositionFlag {
    NT_SelectedTab = 1,
    NT_NotSelectedTab = 2,
    NT_CleanTab = 4,
    NT_TabAtTheEnd = 8,

    NT_SelectedTabAtTheEnd = NT_SelectedTab | NT_TabAtTheEnd,
    NT_NotSelectedTabAtTheEnd = NT_NotSelectedTab | NT_TabAtTheEnd,
    NT_CleanSelectedTabAtTheEnd = NT_SelectedTab | NT_TabAtTheEnd | NT_CleanTab,
    NT_CleanSelectedTab = NT_CleanTab | NT_SelectedTab,
    NT_CleanNotSelectedTab = NT_CleanTab | NT_NotSelectedTab
};

Q_DECLARE_FLAGS(NewTabPositionFlags, NewTabPositionFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(Qz::NewTabPositionFlags)

}

#endif // QZ_NAMESPACE_H
