#ifndef MSVC2008_H
#define MSVC2008_H

//Neccessary declarations to use new Windows 7 API with Microsoft Visual C++ 2008 compiler
//However, it is still needed to link against newer libraries from Visual C++ 2010

#if (_MSC_VER >= 1500 && _MSC_VER < 1600) //Microsoft Visual C++ 2008

#include "rpc.h"
#include "rpcndr.h"

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __objectarray_h__
#define __objectarray_h__

#pragma once

/* Forward Declarations */
#ifndef __IObjectArray_FWD_DEFINED__
#define __IObjectArray_FWD_DEFINED__
typedef interface IObjectArray IObjectArray;
#endif  /* __IObjectArray_FWD_DEFINED__ */


#ifndef __IObjectCollection_FWD_DEFINED__
#define __IObjectCollection_FWD_DEFINED__
typedef interface IObjectCollection IObjectCollection;
#endif  /* __IObjectCollection_FWD_DEFINED__ */

/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C" {
#endif
/*
 ****************************************************************************************************
                                         IObjectArray

                                     <from ObjectArray.h>
 ****************************************************************************************************
*/
#ifndef __IObjectArray_INTERFACE_DEFINED__
#define __IObjectArray_INTERFACE_DEFINED__

/* interface IObjectArray */
/* [unique][object][uuid][helpstring] */


EXTERN_C const IID IID_IObjectArray;

MIDL_INTERFACE("92CA9DCD-5622-4bba-A805-5E9F541BD8C9")
IObjectArray : public IUnknown {
public:
    virtual HRESULT STDMETHODCALLTYPE GetCount(
        /* [out] */ __RPC__out UINT * pcObjects) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetAt(
        /* [in] */ UINT uiIndex,
        /* [in] */ __RPC__in REFIID riid,
        /* [iid_is][out] */ __RPC__deref_out_opt void** ppv) = 0;

};
#endif  /* __IObjectArray_INTERFACE_DEFINED__ */


#ifndef __IObjectCollection_INTERFACE_DEFINED__
#define __IObjectCollection_INTERFACE_DEFINED__

/* interface IObjectCollection */
/* [unique][object][uuid] */


EXTERN_C const IID IID_IObjectCollection;

MIDL_INTERFACE("5632b1a4-e38a-400a-928a-d4cd63230295")
IObjectCollection : public IObjectArray {
public:
    virtual HRESULT STDMETHODCALLTYPE AddObject(
        /* [in] */ __RPC__in_opt IUnknown * punk) = 0;

    virtual HRESULT STDMETHODCALLTYPE AddFromArray(
        /* [in] */ __RPC__in_opt IObjectArray * poaSource) = 0;

    virtual HRESULT STDMETHODCALLTYPE RemoveObjectAt(
        /* [in] */ UINT uiIndex) = 0;

    virtual HRESULT STDMETHODCALLTYPE Clear(void) = 0;

};
#endif  /* __IObjectCollection_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */
/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
/*
 ****************************************************************************************************
                                         ICustomDestinationList

                                           <from ShObjIdl.h>
 ****************************************************************************************************
*/
typedef interface ICustomDestinationList ICustomDestinationList;

/* interface ICustomDestinationList */
/* [unique][object][uuid] */

typedef /* [v1_enum] */
enum KNOWNDESTCATEGORY {
    KDC_FREQUENT    = 1,
    KDC_RECENT  = (KDC_FREQUENT + 1)
}   KNOWNDESTCATEGORY;


EXTERN_C const IID IID_ICustomDestinationList;

MIDL_INTERFACE("6332debf-87b5-4670-90c0-5e57b408a49e")
ICustomDestinationList : public IUnknown {
public:
    virtual HRESULT STDMETHODCALLTYPE SetAppID(
        /* [string][in] */ __RPC__in_string LPCWSTR pszAppID) = 0;

    virtual HRESULT STDMETHODCALLTYPE BeginList(
        /* [out] */ __RPC__out UINT * pcMinSlots,
        /* [in] */ __RPC__in REFIID riid,
        /* [iid_is][out] */ __RPC__deref_out_opt void** ppv) = 0;

    virtual HRESULT STDMETHODCALLTYPE AppendCategory(
        /* [string][in] */ __RPC__in_string LPCWSTR pszCategory,
        /* [in] */ __RPC__in_opt IObjectArray * poa) = 0;

    virtual HRESULT STDMETHODCALLTYPE AppendKnownCategory(
        /* [in] */ KNOWNDESTCATEGORY category) = 0;

    virtual HRESULT STDMETHODCALLTYPE AddUserTasks(
        /* [in] */ __RPC__in_opt IObjectArray * poa) = 0;

    virtual HRESULT STDMETHODCALLTYPE CommitList(void) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetRemovedDestinations(
        /* [in] */ __RPC__in REFIID riid,
        /* [iid_is][out] */ __RPC__deref_out_opt void** ppv) = 0;

    virtual HRESULT STDMETHODCALLTYPE DeleteList(
        /* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszAppID) = 0;

    virtual HRESULT STDMETHODCALLTYPE AbortList(void) = 0;

};
/*
 ****************************************************************************************************
                    CLSID_EnumerableObjectCollection + CLSID_DestinationList

                                       <from ShObjIdl.h>
 ****************************************************************************************************
*/
EXTERN_C const CLSID CLSID_EnumerableObjectCollection;

#ifdef __cplusplus

class DECLSPEC_UUID("2d3468c1-36a7-43b6-ac24-d3f02fd9607a")
        EnumerableObjectCollection;
#endif

EXTERN_C const CLSID CLSID_DestinationList;

#ifdef __cplusplus

class DECLSPEC_UUID("77f10cf0-3db5-4966-b520-b7c54fd35ed6")
        DestinationList;
#endif
/*
 ****************************************************************************************************
                                           ITaskbarList3

                                         <from ShObjIdl.h>
 ****************************************************************************************************
*/
#define DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE) \
    extern "C++" { \
    inline ENUMTYPE operator | (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((int)a) | ((int)b)); } \
    inline ENUMTYPE &operator |= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((int &)a) |= ((int)b)); } \
    inline ENUMTYPE operator & (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((int)a) & ((int)b)); } \
    inline ENUMTYPE &operator &= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((int &)a) &= ((int)b)); } \
    inline ENUMTYPE operator ~ (ENUMTYPE a) { return ENUMTYPE(~((int)a)); } \
    inline ENUMTYPE operator ^ (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((int)a) ^ ((int)b)); } \
    inline ENUMTYPE &operator ^= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((int &)a) ^= ((int)b)); } \
    }

#ifdef MIDL_PASS
typedef IUnknown* HIMAGELIST;

#endif
typedef /* [v1_enum] */
enum THUMBBUTTONFLAGS {
    THBF_ENABLED    = 0,
    THBF_DISABLED   = 0x1,
    THBF_DISMISSONCLICK = 0x2,
    THBF_NOBACKGROUND   = 0x4,
    THBF_HIDDEN = 0x8,
    THBF_NONINTERACTIVE = 0x10
}   THUMBBUTTONFLAGS;

DEFINE_ENUM_FLAG_OPERATORS(THUMBBUTTONFLAGS)
typedef /* [v1_enum] */
enum THUMBBUTTONMASK {
    THB_BITMAP  = 0x1,
    THB_ICON    = 0x2,
    THB_TOOLTIP = 0x4,
    THB_FLAGS   = 0x8
}   THUMBBUTTONMASK;

DEFINE_ENUM_FLAG_OPERATORS(THUMBBUTTONMASK)
#include <pshpack8.h>
typedef struct THUMBBUTTON {
    THUMBBUTTONMASK dwMask;
    UINT iId;
    UINT iBitmap;
    HICON hIcon;
    WCHAR szTip[ 260 ];
    THUMBBUTTONFLAGS dwFlags;
}   THUMBBUTTON;

typedef struct THUMBBUTTON* LPTHUMBBUTTON;

#include <poppack.h>
#define THBN_CLICKED        0x1800


extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0000_0093_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shobjidl_0000_0093_v0_0_s_ifspec;

/* interface ITaskbarList3 */
/* [object][uuid] */

typedef /* [v1_enum] */
enum TBPFLAG {
    TBPF_NOPROGRESS = 0,
    TBPF_INDETERMINATE  = 0x1,
    TBPF_NORMAL = 0x2,
    TBPF_ERROR  = 0x4,
    TBPF_PAUSED = 0x8
}   TBPFLAG;

DEFINE_ENUM_FLAG_OPERATORS(TBPFLAG)

EXTERN_C const IID IID_ITaskbarList3;


MIDL_INTERFACE("ea1afb91-9e28-4b86-90e9-9e9f8a5eefaf")
ITaskbarList3 : public ITaskbarList2 {
public:
virtual HRESULT STDMETHODCALLTYPE SetProgressValue(
    /* [in] */ __RPC__in HWND hwnd,
    /* [in] */ ULONGLONG ullCompleted,
    /* [in] */ ULONGLONG ullTotal) = 0;

virtual HRESULT STDMETHODCALLTYPE SetProgressState(
    /* [in] */ __RPC__in HWND hwnd,
    /* [in] */ TBPFLAG tbpFlags) = 0;

virtual HRESULT STDMETHODCALLTYPE RegisterTab(
    /* [in] */ __RPC__in HWND hwndTab,
    /* [in] */ __RPC__in HWND hwndMDI) = 0;

virtual HRESULT STDMETHODCALLTYPE UnregisterTab(
    /* [in] */ __RPC__in HWND hwndTab) = 0;

virtual HRESULT STDMETHODCALLTYPE SetTabOrder(
    /* [in] */ __RPC__in HWND hwndTab,
    /* [in] */ __RPC__in HWND hwndInsertBefore) = 0;

virtual HRESULT STDMETHODCALLTYPE SetTabActive(
    /* [in] */ __RPC__in HWND hwndTab,
    /* [in] */ __RPC__in HWND hwndMDI,
    /* [in] */ DWORD dwReserved) = 0;

virtual HRESULT STDMETHODCALLTYPE ThumbBarAddButtons(
    /* [in] */ __RPC__in HWND hwnd,
    /* [in] */ UINT cButtons,
    /* [size_is][in] */ __RPC__in_ecount_full(cButtons) LPTHUMBBUTTON pButton) = 0;

virtual HRESULT STDMETHODCALLTYPE ThumbBarUpdateButtons(
    /* [in] */ __RPC__in HWND hwnd,
    /* [in] */ UINT cButtons,
    /* [size_is][in] */ __RPC__in_ecount_full(cButtons) LPTHUMBBUTTON pButton) = 0;

virtual HRESULT STDMETHODCALLTYPE ThumbBarSetImageList(
    /* [in] */ __RPC__in HWND hwnd,
    /* [in] */ __RPC__in_opt HIMAGELIST himl) = 0;

virtual HRESULT STDMETHODCALLTYPE SetOverlayIcon(
    /* [in] */ __RPC__in HWND hwnd,
    /* [in] */ __RPC__in HICON hIcon,
    /* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszDescription) = 0;

virtual HRESULT STDMETHODCALLTYPE SetThumbnailTooltip(
    /* [in] */ __RPC__in HWND hwnd,
    /* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszTip) = 0;

virtual HRESULT STDMETHODCALLTYPE SetThumbnailClip(
    /* [in] */ __RPC__in HWND hwnd,
    /* [in] */ __RPC__in RECT * prcClip) = 0;
};

#endif  //_MSC_VER >= 1500 && _MSC_VER < 1600
#endif // MSVC2008_H
