#ifndef QZSYSINFO_H
#define QZSYSINFO_H

#include <QObject>

#include "qzcommon.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#ifdef Q_OS_WIN
namespace WindowsVersion {
    enum WindowsVersion{
        Windows, Windows32s, Windows95, Windows95OSR2, Windows98, Windows98SE, WindowsMillennium,
        WindowsNT351, WindowsNT40, WindowsNT40Server, Windows2000, WindowsXP,
        WindowsXPProfessionalx64, WindowsHomeServer, WindowsServer2003, WindowsServer2003R2,
        WindowsVista, WindowsServer2008, WindowsServer2008R2, Windows7, WindowsServer2012,
        WindowsServer2012R2, Windows8, Windows81, Windows10, WindowsServer2016
    };
}

namespace WindowsEdition {
    enum WindowsEdition{
        EditionUnknown, Workstation, Server, AdvancedServer, Home, Ultimate, HomeBasic,
        HomePremium, Enterprise, HomeBasicN, Business, StandardServer, DatacenterServer,
        SmallBusinessServer, EnterpriseServer, Starter, DatacenterServerCore, StandardServerCore,
        EnterpriseServerCore, EnterpriseServerIA64, BusinessN, WebServer, ClusterServer, HomeServer,
        StorageExpressServer, StorageStandardServer, StorageWorkgroupServer, StorageEnterpriseServer,
        ServerForSmallBusiness, SmallBusinessServerPremium, HomePremiumN, EnterpriseN, UltimateN,
        WebServerCore, MediumBusinessServerManagement, MediumBusinessServerSecurity,
        MediumBusinessServerMessaging, ServerFoundation, HomePremiumServer, ServerForSmallBusinessV,
        StandardServerV, DatacenterServerV, EnterpriseServerV, DatacenterServerCoreV,
        StandardServerCoreV, EnterpriseServerCoreV, HyperV, StorageExpressServerCore,
        StorageStandardServerCore, StorageWorkgroupServerCore, StorageEnterpriseServerCore,
        StarterN, Professional, ProfessionalN, SBSolutionServer, ServerForSBSolutions,
        StandardServerSolutions, StandardServerSolutionsCore, SBSolutionServerEM,
        ServerForSBSolutionsEM, SolutionEmbeddedServer, SolutionEmbeddedServerCore,
        EssentialBusinessServerMGMT, EssentialBusinessServerADDL, EssentialBusinessServerMGMTSVC,
        EssentialBusinessServerADDLSVC, SmallBusinessServerPremiumCore, ClusterServerV, Embedded,
        StarterE, HomeBasicE, HomePremiumE, ProfessionalE, EnterpriseE, UltimateE,
        EnterpriseEvaluation, MultipointStandardServer, MultipointPremiumServer,
        StandardEvaluationServer, DatacenterEvaluationServer, EnterpriseNEvaluation,
        StorageWorkgroupEvaluationServer, StorageStandardEvaluationServer, CoreN, CoreCountrySpecific,
        CoreSingleLanguage, Core, ProfessionalWindowsMediaCenter
    };
}
#endif

namespace OSVersion {
    enum OSVersion{
        Linux, Unix, BSD, MacCheetah, MacPuma, MacJaguar, MacPanther, MacTiger, MacLeopard,
        MacSnowLeopard, Win2000, WinXP, Win2003, WinVista, Win7, Win8, Win81, Win10
    };
}

class QUPZILLA_EXPORT QzSysInfo : public QObject
{
        Q_OBJECT

#ifdef Q_OS_WIN
        WindowsVersion::WindowsVersion  m_nWindowsVersion;
        WindowsEdition::WindowsEdition  m_nWindowsEdition;
        char                            m_sServicePack[128];
        OSVERSIONINFOEX                 m_osvi;
        SYSTEM_INFO                     m_SysInfo;
        bool                            m_bOsVersionInfoEx;

    private:
        void DetectWindowsVersion();
        void DetectWindowsEdition();
        void DetectWindowsServicePack();
        DWORD DetectProductInfo();
#endif

    public:
        explicit QzSysInfo(QObject *parent = 0);
        OSVersion::OSVersion osVersion();
        QString osVersionToString();

#ifdef Q_OS_WIN
        WindowsVersion::WindowsVersion GetWindowsVersion() const;  // returns the Windows version
        WindowsEdition::WindowsEdition GetWindowsEdition() const;  // returns the Windows edition
        bool IsNTPlatform() const;                                 // true if NT platform
        bool IsWindowsPlatform() const;                            // true is Windows platform
        bool IsWin32sPlatform() const;                             // true is Win32s platform
        DWORD GetMajorVersion() const;                             // returns major version
        DWORD GetMinorVersion() const;                             // returns minor version
        DWORD GetBuildNumber() const;                              // returns build number
        DWORD GetPlatformID() const;                               // returns platform ID
        QString GetServicePackInfo() const;                        // additional information about service pack
        bool Is32bitPlatform() const;                              // true if platform is 32-bit
        bool Is64bitPlatform() const;                              // true if platform is 64-bit
#endif
};

#endif // QZSYSINFO_H
