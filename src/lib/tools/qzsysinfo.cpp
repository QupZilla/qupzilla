#include "qzsysinfo.h"

#include <QProcess>
#include <QFile>
#include <QDebug>

#ifdef Q_OS_WIN
#include <tchar.h>
#include <QSettings>

// Preventing redefinition warnings on Windows using MinGW toolchain.
// The QT_VERSION check is used as a substitute for checking against MinGW >= v4.7, as the Qt4.8 SDK
// is delivered bundled with MinGW4.4 by default. This is a result of __MINGW32_MAJOR_VERSION and
// __MINGW32_MINOR_VERSION being set to useless values for some reason.
// [cedric] mingw xcompiler toolchain gives still problems though
#if !(defined(Q_OS_WIN) && defined(Q_CC_GNU) && QT_VERSION >= 0x050000) || !defined(PRODUCT_UNDEFINED)

#ifndef Q_CC_MSVC
#define VER_SUITE_WH_SERVER 0x8000
#endif

//Windows product definitions
#define PRODUCT_UNDEFINED                           0x00000000
#define PRODUCT_ULTIMATE                            0x00000001
#define PRODUCT_HOME_BASIC                          0x00000002
#define PRODUCT_HOME_PREMIUM                        0x00000003
#define PRODUCT_ENTERPRISE                          0x00000004
#define PRODUCT_HOME_BASIC_N                        0x00000005
#define PRODUCT_BUSINESS                            0x00000006
#define PRODUCT_STANDARD_SERVER                     0x00000007
#define PRODUCT_DATACENTER_SERVER                   0x00000008
#define PRODUCT_SMALLBUSINESS_SERVER                0x00000009
#define PRODUCT_ENTERPRISE_SERVER                   0x0000000A
#define PRODUCT_STARTER                             0x0000000B
#define PRODUCT_DATACENTER_SERVER_CORE              0x0000000C
#define PRODUCT_STANDARD_SERVER_CORE                0x0000000D
#define PRODUCT_ENTERPRISE_SERVER_CORE              0x0000000E
#define PRODUCT_ENTERPRISE_SERVER_IA64              0x0000000F
#define PRODUCT_BUSINESS_N                          0x00000010
#define PRODUCT_WEB_SERVER                          0x00000011
#define PRODUCT_CLUSTER_SERVER                      0x00000012
#define PRODUCT_HOME_SERVER                         0x00000013
#define PRODUCT_STORAGE_EXPRESS_SERVER              0x00000014
#define PRODUCT_STORAGE_STANDARD_SERVER             0x00000015
#define PRODUCT_STORAGE_WORKGROUP_SERVER            0x00000016
#define PRODUCT_STORAGE_ENTERPRISE_SERVER           0x00000017
#define PRODUCT_SERVER_FOR_SMALLBUSINESS            0x00000018
#define PRODUCT_SMALLBUSINESS_SERVER_PREMIUM        0x00000019
#define PRODUCT_HOME_PREMIUM_N                      0x0000001A
#define PRODUCT_ENTERPRISE_N                        0x0000001B
#define PRODUCT_ULTIMATE_N                          0x0000001C
#define PRODUCT_WEB_SERVER_CORE                     0x0000001D
#define PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT    0x0000001E
#define PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY      0x0000001F
#define PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING     0x00000020
#define PRODUCT_SERVER_FOUNDATION                   0x00000021
#define PRODUCT_HOME_PREMIUM_SERVER                 0x00000022
#define PRODUCT_SERVER_FOR_SMALLBUSINESS_V          0x00000023
#define PRODUCT_STANDARD_SERVER_V                   0x00000024
#define PRODUCT_DATACENTER_SERVER_V                 0x00000025
#define PRODUCT_ENTERPRISE_SERVER_V                 0x00000026
#define PRODUCT_DATACENTER_SERVER_CORE_V            0x00000027
#define PRODUCT_STANDARD_SERVER_CORE_V              0x00000028
#define PRODUCT_ENTERPRISE_SERVER_CORE_V            0x00000029
#define PRODUCT_HYPERV                              0x0000002A
#define PRODUCT_STORAGE_EXPRESS_SERVER_CORE         0x0000002B
#define PRODUCT_STORAGE_STANDARD_SERVER_CORE        0x0000002C
#define PRODUCT_STORAGE_WORKGROUP_SERVER_CORE       0x0000002D
#define PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE      0x0000002E
#define PRODUCT_STARTER_N                           0x0000002F
#define PRODUCT_PROFESSIONAL                        0x00000030
#define PRODUCT_PROFESSIONAL_N                      0x00000031
#define PRODUCT_SB_SOLUTION_SERVER                  0x00000032
#define PRODUCT_SERVER_FOR_SB_SOLUTIONS             0x00000033
#define PRODUCT_STANDARD_SERVER_SOLUTIONS           0x00000034
#define PRODUCT_STANDARD_SERVER_SOLUTIONS_CORE      0x00000035
#define PRODUCT_SB_SOLUTION_SERVER_EM               0x00000036
#define PRODUCT_SERVER_FOR_SB_SOLUTIONS_EM          0x00000037
#define PRODUCT_SOLUTION_EMBEDDEDSERVER             0x00000038
#define PRODUCT_SOLUTION_EMBEDDEDSERVER_CORE        0x00000039
#define PRODUCT_ESSENTIALBUSINESS_SERVER_MGMT       0x0000003B
#define PRODUCT_ESSENTIALBUSINESS_SERVER_ADDL       0x0000003C
#define PRODUCT_ESSENTIALBUSINESS_SERVER_MGMTSVC    0x0000003D
#define PRODUCT_ESSENTIALBUSINESS_SERVER_ADDLSVC    0x0000003E
#define PRODUCT_SMALLBUSINESS_SERVER_PREMIUM_CORE   0x0000003F
#define PRODUCT_CLUSTER_SERVER_V                    0x00000040
#define PRODUCT_EMBEDDED                            0x00000041
#define PRODUCT_STARTER_E                           0x00000042
#define PRODUCT_HOME_BASIC_E                        0x00000043
#define PRODUCT_HOME_PREMIUM_E                      0x00000044
#define PRODUCT_PROFESSIONAL_E                      0x00000045
#define PRODUCT_ENTERPRISE_E                        0x00000046
#define PRODUCT_ULTIMATE_E                          0x00000047

#endif

#define PRODUCT_ENTERPRISE_EVALUATION               0x00000048
#define PRODUCT_MULTIPOINT_STANDARD_SERVER          0x0000004C
#define PRODUCT_MULTIPOINT_PREMIUM_SERVER           0x0000004D
#define PRODUCT_STANDARD_EVALUATION_SERVER          0x0000004F
#define PRODUCT_DATACENTER_EVALUATION_SERVER        0x00000050
#define PRODUCT_ENTERPRISE_N_EVALUATION             0x00000054
#define PRODUCT_STORAGE_WORKGROUP_EVALUATION_SERVER 0x0000005F
#define PRODUCT_STORAGE_STANDARD_EVALUATION_SERVER  0x00000060
#define PRODUCT_CORE_N                              0x00000062
#define PRODUCT_CORE_COUNTRYSPECIFIC                0x00000063
#define PRODUCT_CORE_SINGLELANGUAGE                 0x00000064
#define PRODUCT_CORE                                0x00000065
#define PRODUCT_PROFESSIONAL_WMC                    0x00000067

#define PROCESSOR_ARCHITECTURE_AMD64 9
#define SM_SERVERR2 89

typedef void (WINAPI *PGetNativeSystemInfo)(LPSYSTEM_INFO);
typedef bool (WINAPI *PGetProductInfo)(DWORD, DWORD, DWORD, DWORD, PDWORD);
#endif

QzSysInfo::QzSysInfo(QObject *parent) :
    QObject(parent)
{
#ifdef Q_OS_WIN
    bool canDetect = true;
    PGetNativeSystemInfo pGNSI = 0;

    m_bOsVersionInfoEx = false;
    m_nWindowsVersion = WindowsVersion::Windows;
    m_nWindowsEdition = WindowsEdition::EditionUnknown;
    memset(m_sServicePack, 0, sizeof(m_sServicePack));

    // Try calling GetVersionEx using the OSVERSIONINFOEX structure.
    ZeroMemory(&m_osvi, sizeof(OSVERSIONINFOEX));
    m_osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if (!(m_bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *) &m_osvi)))
    {
        // If that fails, try using the OSVERSIONINFO structure.
        m_osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if (!GetVersionEx((OSVERSIONINFO *) &m_osvi))
            canDetect = false;
    }

    pGNSI = (PGetNativeSystemInfo) GetProcAddress(GetModuleHandle(L"kernel32.dll"), "GetNativeSystemInfo");

    if (pGNSI != 0)
        pGNSI(&m_SysInfo);
    else
        GetSystemInfo(&m_SysInfo);

    if (canDetect)
    {
        DetectWindowsVersion();
        DetectWindowsEdition();
        DetectWindowsServicePack();
    }
#endif
}

OSVersion::OSVersion QzSysInfo::osVersion()
{
#ifdef Q_OS_LINUX
    return OSVersion::Linux;
#endif
#ifdef Q_OS_FREEBSD
    return OSVersion::BSD;
#endif
#ifdef Q_OS_NETBSD
    return OSVersion::BSD;
#endif
#ifdef Q_OS_OPENBSD
    return OSVersion::BSD;
#endif
#ifdef Q_OS_UNIX
    return OSVersion::Unix;
#endif
#ifdef Q_OS_MAC
    switch (QSysInfo::MacintoshVersion) {
        case QSysInfo::MV_CHEETAH:
            return OSVersion::MacCheetah;
        case QSysInfo::MV_PUMA:
            return OSVersion::MacPuma;
        case QSysInfo::MV_JAGUAR:
            return OSVersion::MacJaguar;
        case QSysInfo::MV_PANTHER:
            return OSVersion::MacPanther;
        case QSysInfo::MV_TIGER:
            return OSVersion::MacTiger;
        case QSysInfo::MV_LEOPARD:
            return OSVersion::MacLeopard;
        case QSysInfo::MV_SNOWLEOPARD:
            return OSVersion::MacSnowLeopard;
        default:
            return OSVersion::MacSnowLeopard;
    }
#endif
#ifdef Q_OS_WIN
    switch (QSysInfo::windowsVersion()) {
        case QSysInfo::WV_2000:
            return OSVersion::Win2000;
        case QSysInfo::WV_XP:
            return OSVersion::WinXP;
        case QSysInfo::WV_2003:
            return OSVersion::Win2003;
        case QSysInfo::WV_VISTA:
            return OSVersion::WinVista;
        case QSysInfo::WV_WINDOWS7:
            return OSVersion::Win7;
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
        case QSysInfo::WV_WINDOWS8: // not yet defined in Qt4.8
            return OSVersion::Win8;
        case QSysInfo::WV_WINDOWS8_1:
            return OSVersion::Win81;
        case QSysInfo::WV_WINDOWS10:
            return OSVersion::Win10;
#endif
        default:
            return OSVersion::Win7;
    }
#endif
}

QString QzSysInfo::osVersionToString()
{
    QString operatingSystemString = "";

#if defined(Q_OS_WIN)
    QString sVersion;
    QString sEdition;
    QString sServicePack;
    QString sMachine;

    switch (GetWindowsVersion()) {
        case WindowsVersion::Windows:
            sVersion = "Windows";
            break;
        case WindowsVersion::Windows32s:
            sVersion = "Windows 32s";
            break;
        case WindowsVersion::Windows95:
            sVersion = "Windows 95";
            break;
        case WindowsVersion::Windows95OSR2:
            sVersion = "Windows 95 SR2";
            break;
        case WindowsVersion::Windows98:
            sVersion = "Windows 98";
            break;
        case WindowsVersion::Windows98SE:
            sVersion = "Windows 98 SE";
            break;
        case WindowsVersion::WindowsMillennium:
            sVersion = "Windows Me";
            break;
        case WindowsVersion::WindowsNT351:
            sVersion = "Windows NT 3.51";
            break;
        case WindowsVersion::WindowsNT40:
            sVersion = "Windows NT 4.0";
            break;
        case WindowsVersion::WindowsNT40Server:
            sVersion = "Windows NT 4.0 Server";
            break;
        case WindowsVersion::Windows2000:
            sVersion = "Windows 2000";
            break;
        case WindowsVersion::WindowsXP:
            sVersion = "Windows XP";
            break;
        case WindowsVersion::WindowsXPProfessionalx64:
            sVersion = tr("Windows XP Professional x64");
            break;
        case WindowsVersion::WindowsHomeServer:
            sVersion = tr("Windows Home Server");
            break;
        case WindowsVersion::WindowsServer2003:
            sVersion = "Windows Server 2003";
            break;
        case WindowsVersion::WindowsServer2003R2:
            sVersion = "Windows Server 2003 R2";
            break;
        case WindowsVersion::WindowsVista:
            sVersion = "Windows Vista";
            break;
        case WindowsVersion::WindowsServer2008:
            sVersion = "Windows Server 2008";
            break;
        case WindowsVersion::WindowsServer2008R2:
            sVersion = "Windows Server 2008 R2";
            break;
        case WindowsVersion::Windows7:
            sVersion = "Windows 7";
            break;
        case WindowsVersion::WindowsServer2012:
            sVersion = "Windows Server 2012";
            break;
        case WindowsVersion::WindowsServer2012R2:
            sVersion = "Windows Server 2012 R2";
            break;
        case WindowsVersion::Windows8:
            sVersion = "Windows 8";
            break;
        case WindowsVersion::Windows81:
            sVersion = "Windows 8.1";
            break;
        case WindowsVersion::Windows10:
            sVersion = "Windows 10";
            break;
        case WindowsVersion::WindowsServer2016:
            sVersion = "Windows Server 2016";
            break;
        default:
            sVersion = tr("Unknown Windows Version");
            break;
    }

    switch (GetWindowsEdition()) {
        case WindowsEdition::EditionUnknown:
            sEdition = tr("Unknown Edition");
            break;
        case WindowsEdition::Workstation:
            sEdition = tr("Workstation");
            break;
        case WindowsEdition::Server:
            sEdition = tr("Server");
            break;
        case WindowsEdition::AdvancedServer:
            sEdition = tr("Advanced Server");
            break;
        case WindowsEdition::Home:
            sEdition = tr("Home");
            break;
        case WindowsEdition::Ultimate:
            sEdition = tr("Ultimate");
            break;
        case WindowsEdition::HomeBasic:
            sEdition = tr("Home Basic");
            break;
        case WindowsEdition::HomePremium:
            sEdition = tr("Home Premium");
            break;
        case WindowsEdition::Enterprise:
            sEdition = tr("Enterprise");
            break;
        case WindowsEdition::HomeBasicN:
            sEdition = tr("Home Basic N");
            break;
        case WindowsEdition::Business:
            sEdition = tr("Business");
            break;
        case WindowsEdition::StandardServer:
            sEdition = tr("Server Standard");
            break;
        case WindowsEdition::DatacenterServer:
            sEdition = tr("Server Datacenter (full installation)");
            break;
        case WindowsEdition::SmallBusinessServer:
            sEdition = tr("Small Business Server");
            break;
        case WindowsEdition::EnterpriseServer:
            sEdition = tr("Server Enterprise (full installation)");
            break;
        case WindowsEdition::Starter:
            sEdition = tr("Starter");
            break;
        case WindowsEdition::DatacenterServerCore:
            sEdition = tr("Server Datacenter (core installation)");
            break;
        case WindowsEdition::StandardServerCore:
            sEdition = tr("Server Standard (core installation)");
            break;
        case WindowsEdition::EnterpriseServerCore:
            sEdition = tr("Server Enterprise (core installation)");
            break;
        case WindowsEdition::EnterpriseServerIA64:
            sEdition = tr("Server Enterprise for Itanium-based Systems");
            break;
        case WindowsEdition::BusinessN:
            sEdition = tr("Business N");
            break;
        case WindowsEdition::WebServer:
            sEdition = tr("Web Server (full installation)");
            break;
        case WindowsEdition::ClusterServer:
            sEdition = tr("High Performance Computing Edition");
            break;
        case WindowsEdition::HomeServer:
            sEdition = tr("Storage Server 2008 R2 Essentials");
            break;
        case WindowsEdition::StorageExpressServer:
            sEdition = tr("Storage Server Express");
            break;
        case WindowsEdition::StorageStandardServer:
            sEdition = tr("Storage Server Standard");
            break;
        case WindowsEdition::StorageWorkgroupServer:
            sEdition = tr("Storage Server Workgroup");
            break;
        case WindowsEdition::StorageEnterpriseServer:
            sEdition = tr("Storage Server Enterprise");
            break;
        case WindowsEdition::ServerForSmallBusiness:
            sEdition = tr("for Windows Essential Server Solutions");
            break;
        case WindowsEdition::SmallBusinessServerPremium:
            sEdition = tr("Small Business Server Premium");
            break;
        case WindowsEdition::HomePremiumN:
            sEdition = tr("Home Premium N");
            break;
        case WindowsEdition::EnterpriseN:
            sEdition = tr("Enterprise N");
            break;
        case WindowsEdition::UltimateN:
            sEdition = tr("Ultimate N");
            break;
        case WindowsEdition::WebServerCore:
            sEdition = tr("Web Server (core installation)");
            break;
        case WindowsEdition::MediumBusinessServerManagement:
            sEdition = tr("Essential Business Server Management Server");
            break;
        case WindowsEdition::MediumBusinessServerSecurity:
            sEdition = tr("Essential Business Server Security Server");
            break;
        case WindowsEdition::MediumBusinessServerMessaging:
            sEdition = tr("Essential Business Server Messaging Server");
            break;
        case WindowsEdition::ServerFoundation:
            sEdition = tr("Server Foundation");
            break;
        case WindowsEdition::HomePremiumServer:
            sEdition = tr("Home Server 2011");
            break;
        case WindowsEdition::ServerForSmallBusinessV:
            sEdition = tr("without Hyper-V for Windows Essential Server Solutions");
            break;
        case WindowsEdition::StandardServerV:
            sEdition = tr("Server Standard without Hyper-V");
            break;
        case WindowsEdition::DatacenterServerV:
            sEdition = tr("Server Datacenter without Hyper-V (full installation)");
            break;
        case WindowsEdition::EnterpriseServerV:
            sEdition = tr("Server Enterprise without Hyper-V (full installation)");
            break;
        case WindowsEdition::DatacenterServerCoreV:
            sEdition = tr("Server Datacenter without Hyper-V (core installation)");
            break;
        case WindowsEdition::StandardServerCoreV:
            sEdition = tr("Server Standard without Hyper-V (core installation)");
            break;
        case WindowsEdition::EnterpriseServerCoreV:
            sEdition = tr("Server Enterprise without Hyper-V (core installation)");
            break;
        case WindowsEdition::HyperV:
            sEdition = tr("Hyper-V Server");
            break;
        case WindowsEdition::StorageExpressServerCore:
            sEdition = tr("Storage Express Server Core");
            break;
        case WindowsEdition::StorageStandardServerCore:
            sEdition = tr("Storage Standard Server Core");
            break;
        case WindowsEdition::StorageWorkgroupServerCore:
            sEdition = tr("Storage Workgroup Server Core");
            break;
        case WindowsEdition::StorageEnterpriseServerCore:
            sEdition = tr("Storage Enterprise Server Core");
            break;
        case WindowsEdition::StarterN:
            sEdition = tr("Starter N");
            break;
        case WindowsEdition::Professional:
            sEdition = tr("Professional");
            break;
        case WindowsEdition::ProfessionalN:
            sEdition = tr("Professional N");
            break;
        case WindowsEdition::SBSolutionServer:
            sEdition = tr("Windows Small Business Server 2011 Essentials");
            break;
        case WindowsEdition::ServerForSBSolutions:
            sEdition = tr("Server For Small Business Solutions");
            break;
        case WindowsEdition::StandardServerSolutions:
            sEdition = tr("Server Solutions Premium");
            break;
        case WindowsEdition::StandardServerSolutionsCore:
            sEdition = tr("Server Solutions Premium (core installation)");
            break;
        case WindowsEdition::SBSolutionServerEM:
            sEdition = tr("Server For Small Business Solutions EM");
            break;
        case WindowsEdition::ServerForSBSolutionsEM:
            sEdition = tr("Server For Small Business Solutions EM");
            break;
        case WindowsEdition::SolutionEmbeddedServer:
            sEdition = tr("MultiPoint Server");
            break;
        case WindowsEdition::SolutionEmbeddedServerCore:
            sEdition = tr("MultiPoint Server (core installation)");
            break;
        case WindowsEdition::EssentialBusinessServerMGMT:
            sEdition = tr("Essential Server Solution Management");
            break;
        case WindowsEdition::EssentialBusinessServerADDL:
            sEdition = tr("Essential Server Solution Additional");
            break;
        case WindowsEdition::EssentialBusinessServerMGMTSVC:
            sEdition = tr("Essential Server Solution Management Service");
            break;
        case WindowsEdition::EssentialBusinessServerADDLSVC:
            sEdition = tr("Essential Server Solution Additional Service");
            break;
        case WindowsEdition::SmallBusinessServerPremiumCore:
            sEdition = tr("Small Business Server Premium (core installation)");
            break;
        case WindowsEdition::ClusterServerV:
            sEdition = tr("Server Hyper Core V");
            break;
        case WindowsEdition::Embedded:
            sEdition = tr("Embedded Standard");
            break;
        case WindowsEdition::StarterE:
            sEdition = tr("Starter E");
            break;
        case WindowsEdition::HomeBasicE:
            sEdition = tr("Home Basic E");
            break;
        case WindowsEdition::HomePremiumE:
            sEdition = tr("Home Premium E");
            break;
        case WindowsEdition::ProfessionalE:
            sEdition = tr("Professional E");
            break;
        case WindowsEdition::EnterpriseE:
            sEdition = tr("Enterprise E");
            break;
        case WindowsEdition::UltimateE:
            sEdition = tr("Ultimate E");
            break;
        case WindowsEdition::EnterpriseEvaluation:
            sEdition = tr("Server Enterprise (evaluation installation)");
            break;
        case WindowsEdition::MultipointStandardServer:
            sEdition = tr("MultiPoint Server Standard (full installation)");
            break;
        case WindowsEdition::MultipointPremiumServer:
            sEdition = tr("MultiPoint Server Premium (full installation)");
            break;
        case WindowsEdition::StandardEvaluationServer:
            sEdition = tr("Server Standard (evaluation installation)");
            break;
        case WindowsEdition::DatacenterEvaluationServer:
            sEdition = tr("Server Datacenter (evaluation installation)");
            break;
        case WindowsEdition::EnterpriseNEvaluation:
            sEdition = tr("Enterprise N (evaluation installation)");
            break;
        case WindowsEdition::StorageWorkgroupEvaluationServer:
            sEdition = tr("Storage Server Workgroup (evaluation installation)");
            break;
        case WindowsEdition::StorageStandardEvaluationServer:
            sEdition = tr("Storage Server Standard (evaluation installation)");
            break;
        case WindowsEdition::CoreN:
            sEdition = tr("Core N");
            break;
        case WindowsEdition::CoreCountrySpecific:
            sEdition = tr("Core China");
            break;
        case WindowsEdition::CoreSingleLanguage:
            sEdition = tr("Core Single Language");
            break;
        case WindowsEdition::Core:
            sEdition = tr("Core");
            break;
        case WindowsEdition::ProfessionalWindowsMediaCenter:
            sEdition = tr("Professional Windows Media Center");
            break;
        default:
            sEdition = tr("Unknown Edition");
    }

    sServicePack = GetServicePackInfo();

    if (Is64bitPlatform())
        sMachine = tr("64-bit");
    else
        sMachine = tr("32-bit");

    operatingSystemString = QString(sVersion + " ");
    if (!sEdition.isEmpty())
        operatingSystemString.append(sEdition + " ");
    if (!sServicePack.isEmpty())
        operatingSystemString.append(tr("with %1 ").arg(sServicePack));
    operatingSystemString.append(sMachine);

#elif defined(Q_OS_MAC)
    switch (QSysInfo::MacintoshVersion) {
        case QSysInfo::MV_9 :
            operatingSystemString = tr("Mac OS 9 (unsupported)");
            break;
        case QSysInfo::MV_10_0 :
            operatingSystemString = tr("Mac OS X 10.0 Cheetah (unsupported)");
            break;
        case QSysInfo::MV_10_1 :
            operatingSystemString = tr("Mac OS X 10.1 Puma (unsupported)");
            break;
        case QSysInfo::MV_10_2 :
            operatingSystemString = tr("Mac OS X 10.2 Jaguar (unsupported)");
            break;
        case QSysInfo::MV_10_3 :
            operatingSystemString = tr("Mac OS X 10.3 Panther");
            break;
        case QSysInfo::MV_10_4 :
            operatingSystemString = tr("Mac OS X 10.4 Tiger");
            break;
        case QSysInfo::MV_10_5 :
            operatingSystemString = tr("Mac OS X 10.5 Leopard");
            break;
        case QSysInfo::MV_10_6 :
            operatingSystemString = tr("Mac OS X 10.6 Snow Leopard");
            break;
        case QSysInfo::MV_10_7 :
            operatingSystemString = tr("Mac OS X 10.7 Lion");
            break;
        case QSysInfo::MV_10_8 :
            operatingSystemString = tr("Mac OS X 10.8 Mountain Lion");
            break;
        case QSysInfo::MV_10_9 :
            operatingSystemString = tr("Mac OS X 10.9 Mavericks");
            break;
        case QSysInfo::MV_Unknown :
            operatingSystemString = tr("An unknown and currently unsupported Mac platform");
            break;
        default :
            operatingSystemString = tr("Unknown Mac operating system.");
            break;
    }
#else
    //TODO: Detect Unix, Linux etc. distro as described on http://www.novell.com/coolsolutions/feature/11251.html
    operatingSystemString = "Linux";
    QProcess process;

    process.start("uname -s");
    process.waitForFinished(1000);
    QString os = process.readAllStandardOutput().trimmed();

    process.start("uname -r");
    process.waitForFinished(1000);
    QString rev = process.readAllStandardOutput().trimmed();

    process.start("uname -m");
    process.waitForFinished(1000);
    QString mach = process.readAllStandardOutput().trimmed();

    if (os == "SunOS") {
        os = "Solaris";

        process.start("uname -p");
        process.waitForFinished(1000);
        QString arch = process.readAllStandardOutput().trimmed();

        process.start("uname -v");
        process.waitForFinished(1000);
        QString timestamp = process.readAllStandardOutput().trimmed();

        operatingSystemString = os + " " + rev + " (" + arch + " " + timestamp + ")";
    }
    else if (os == "AIX") {
        process.start("oslevel -r");
        process.waitForFinished(1000);
        QString oslevel = process.readAllStandardOutput().trimmed();

        operatingSystemString = os + "oslevel " + oslevel;
    }
    else if (os == "Linux") {
        QString dist;
        QString pseudoname;
        QString kernel = rev;

        if (QFile::exists("/etc/SUSE-release")) {
            process.start("sh -c \"cat /etc/SUSE-release | tr '\\n' ' '| sed s/VERSION.*//\"");
            process.waitForFinished(1000);
            dist = process.readAllStandardOutput().trimmed();

            process.start("sh -c \"cat /etc/SUSE-release | tr '\\n' ' ' | sed s/.*=\\ //\"");
            process.waitForFinished(1000);
            rev = process.readAllStandardOutput().trimmed();
        }
        else if (QFile::exists("/etc/mandrake-release")) {
            dist = "Mandrake";

            process.start("sh -c \"cat /etc/mandrake-release | sed s/.*\\(// | sed s/\\)//\"");
            process.waitForFinished(1000);
            pseudoname = process.readAllStandardOutput().trimmed();

            process.start("sh -c \"cat /etc/mandrake-release | sed s/.*release\\ // | sed s/\\ .*//\"");
            process.waitForFinished(1000);
            rev = process.readAllStandardOutput().trimmed();
        }
        else if (QFile::exists("/etc/lsb-release")) {
            dist = "Ubuntu";

            process.start("sh -c \"cat /etc/lsb-release | grep --max-count=1 DISTRIB_RELEASE=\"");
            process.waitForFinished(1000);
            rev = process.readAllStandardOutput().trimmed();
            qDebug() << "revision:" << rev;
            if (!rev.isEmpty())
            {
                rev.remove("DISTRIB_RELEASE=");
                rev.remove("\"");
            }

            process.start("sh -c \"cat /etc/lsb-release | grep --max-count=1 DISTRIB_CODENAME=\"");
            process.waitForFinished(1000);
            pseudoname = process.readAllStandardOutput().trimmed();
            qDebug() << "pseudoname:" << pseudoname;
            if (!pseudoname.isEmpty())
            {
                pseudoname.remove("DISTRIB_CODENAME=");
                pseudoname.remove("\"");
            }
        }
        else if (QFile::exists("/etc/debian_version")) {
            dist = "Debian";

            process.start("cat /etc/debian_version");
            process.waitForFinished(1000);
            dist += process.readAllStandardOutput().trimmed();

            rev = "";
        }

        if (QFile::exists("/etc/UnitedLinux-release")) {
            process.start("sh -c \"cat /etc/UnitedLinux-release | grep --max-count=1 \"VERSION = \"\"");
            process.waitForFinished(1000);
            dist += process.readAllStandardOutput().trimmed();
            if (!dist.isEmpty())
            {
                dist.remove("VERSION = ");
                dist.remove("\"");
            }
        }

        if (QFile::exists("/etc/os-release")) { //This file makes distribution identification much easier.
            process.start("sh -c \"cat /etc/os-release | grep --max-count=1 PRETTY_NAME=\"");
            process.waitForFinished(1000);
            QString distname = process.readAllStandardOutput().trimmed();
            if (!distname.isEmpty())
            {
                distname.remove("PRETTY_NAME=");
                distname.remove("\"");

                dist = distname;
                pseudoname = "";
            }
        }

        operatingSystemString = os;
        if (!dist.isEmpty())
            operatingSystemString.append(" " + dist);
        if (!rev.isEmpty())
            operatingSystemString.append(" " + rev);
        operatingSystemString.append(" (");
        if (!pseudoname.isEmpty())
            operatingSystemString.append(pseudoname + " ");
        if (!kernel.isEmpty())
            operatingSystemString.append(kernel + " ");
        operatingSystemString.append(mach + ")");
    }
#endif

    return operatingSystemString;
}

#ifdef Q_OS_WIN
void QzSysInfo::DetectWindowsVersion()
{
    if (m_bOsVersionInfoEx)
    {
        switch (m_osvi.dwPlatformId) {
            case VER_PLATFORM_WIN32s:
                m_nWindowsVersion = WindowsVersion::Windows32s;
                break;

            // Test for the Windows 95 product family.
            case VER_PLATFORM_WIN32_WINDOWS:
                if (m_osvi.dwMajorVersion == 4)
                {
                    switch (m_osvi.dwMinorVersion)
                    {
                        case 0:
                            if (m_osvi.szCSDVersion[0] == 'B' || m_osvi.szCSDVersion[0] == 'C')
                                m_nWindowsVersion = WindowsVersion::Windows95OSR2;
                            else
                                m_nWindowsVersion = WindowsVersion::Windows95;
                            break;
                        case 10:
                            if (m_osvi.szCSDVersion[0] == 'A')
                                m_nWindowsVersion = WindowsVersion::Windows98SE;
                            else
                                m_nWindowsVersion = WindowsVersion::Windows98;
                            break;
                        case 90:
                            m_nWindowsVersion = WindowsVersion::WindowsMillennium;
                            break;
                    }
                }
                break;

            // Test for the Windows NT product family.
            case VER_PLATFORM_WIN32_NT:
                if (m_osvi.dwMajorVersion == 3)
                {
                    m_nWindowsVersion = WindowsVersion::WindowsNT351;
                }
                else if (m_osvi.dwMajorVersion == 4)
                {
                    if (m_osvi.wProductType == 1)
                        m_nWindowsVersion = WindowsVersion::WindowsNT40;
                    else if (m_osvi.wProductType == 3)
                        m_nWindowsVersion = WindowsVersion::WindowsNT40Server;
                }
                else if (m_osvi.dwMajorVersion == 5)
                {
                    switch (m_osvi.dwMinorVersion)
                    {
                        case 0:
                            m_nWindowsVersion = WindowsVersion::Windows2000;
                            break;
                        case 1:
                            m_nWindowsVersion = WindowsVersion::WindowsXP;
                            break;
                        case 2:
                            if (m_osvi.wSuiteMask == VER_SUITE_WH_SERVER)
                                m_nWindowsVersion = WindowsVersion::WindowsHomeServer;
                            else if (m_osvi.wProductType == VER_NT_WORKSTATION &&
                                     m_SysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
                                m_nWindowsVersion = WindowsVersion::WindowsXPProfessionalx64;
                            else
                                m_nWindowsVersion = ::GetSystemMetrics(SM_SERVERR2) == 0 ?
                                            WindowsVersion::WindowsServer2003 :
                                            WindowsVersion::WindowsServer2003R2;
                            break;
                    }
                }
                else if (m_osvi.dwMajorVersion == 6)
                {
                    switch (m_osvi.dwMinorVersion)
                    {
                        case 0:
                            m_nWindowsVersion = m_osvi.wProductType == VER_NT_WORKSTATION ?
                                        WindowsVersion::WindowsVista :
                                        WindowsVersion::WindowsServer2008;
                            break;
                        case 1:
                            m_nWindowsVersion = m_osvi.wProductType == VER_NT_WORKSTATION ?
                                        WindowsVersion::Windows7 :
                                        WindowsVersion::WindowsServer2008R2;
                            break;
                        case 2:
                            m_nWindowsVersion = m_osvi.wProductType == VER_NT_WORKSTATION ?
                                        WindowsVersion::Windows8 :
                                        WindowsVersion::WindowsServer2012;
                            break;
                        case 3:
                            m_nWindowsVersion = m_osvi.wProductType == VER_NT_WORKSTATION ?
                                        WindowsVersion::Windows81 :
                                        WindowsVersion::WindowsServer2012R2;
                            break;
                    }
                }
                else if (m_osvi.dwMajorVersion == 10)
                {
                    switch (m_osvi.dwMinorVersion)
                    {
                        case 0:
                            m_nWindowsVersion = m_osvi.wProductType == VER_NT_WORKSTATION ?
                                        WindowsVersion::Windows10 :
                                        WindowsVersion::WindowsServer2016;
                            break;
                    }
                }
                break;
        }
    }
    else // Test for specific product on Windows NT 4.0 SP5 and earlier
    {
        QSettings registry("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\ProductOptions",QSettings::NativeFormat);
        QString sProductType;

        sProductType = registry.value("ProductType").toString();
        sProductType = sProductType.toUpper();

        if (sProductType == "WINNT")
        {
            if (m_osvi.dwMajorVersion <= 4)
            {
                m_nWindowsVersion = WindowsVersion::WindowsNT40;
                m_nWindowsEdition = WindowsEdition::Workstation;
            }
        }
        if (sProductType == "LANMANNT")
        {
            if (m_osvi.dwMajorVersion == 5 && m_osvi.dwMinorVersion == 2)
                m_nWindowsVersion = WindowsVersion::WindowsServer2003;

            if (m_osvi.dwMajorVersion == 5 && m_osvi.dwMinorVersion == 0)
            {
                m_nWindowsVersion = WindowsVersion::Windows2000;
                m_nWindowsEdition = WindowsEdition::Server;
            }

            if (m_osvi.dwMajorVersion <= 4)
            {
                m_nWindowsVersion = WindowsVersion::WindowsNT40;
                m_nWindowsEdition = WindowsEdition::Server;
            }
        }
        if (sProductType == "SERVERNT")
        {
            if (m_osvi.dwMajorVersion == 5 && m_osvi.dwMinorVersion == 2)
            {
                m_nWindowsVersion = WindowsVersion::WindowsServer2003;
                m_nWindowsEdition = WindowsEdition::EnterpriseServer;
            }

            if (m_osvi.dwMajorVersion == 5 && m_osvi.dwMinorVersion == 0)
            {
                m_nWindowsVersion = WindowsVersion::Windows2000;
                m_nWindowsEdition = WindowsEdition::AdvancedServer;
            }

            if (m_osvi.dwMajorVersion <= 4)
            {
                m_nWindowsVersion = WindowsVersion::WindowsNT40;
                m_nWindowsEdition = WindowsEdition::EnterpriseServer;
            }
        }
    }
}

void QzSysInfo::DetectWindowsEdition()
{
    if (m_bOsVersionInfoEx)
    {
        if (m_osvi.dwMajorVersion == 4)
        {
            switch (m_osvi.wProductType)
            {
                case VER_NT_WORKSTATION:
                    m_nWindowsEdition = WindowsEdition::Workstation;
                    break;
                case VER_NT_SERVER:
                    m_nWindowsEdition = (m_osvi.wSuiteMask & VER_SUITE_ENTERPRISE) != 0 ?
                                WindowsEdition::EnterpriseServer :
                                WindowsEdition::StandardServer;
                    break;
            }
        }
        else if (m_osvi.dwMajorVersion == 5)
        {
            switch (m_osvi.wProductType)
            {
                case VER_NT_WORKSTATION:
                    m_nWindowsEdition = (m_osvi.wSuiteMask & VER_SUITE_PERSONAL) != 0 ?
                                WindowsEdition::Home :
                                WindowsEdition::Professional;
                    break;
                case VER_NT_SERVER:
                    switch (m_osvi.dwMinorVersion)
                    {
                        case 0:
                            if ((m_osvi.wSuiteMask & VER_SUITE_DATACENTER) != 0)
                                m_nWindowsEdition = WindowsEdition::DatacenterServer;
                            else if ((m_osvi.wSuiteMask & VER_SUITE_ENTERPRISE) != 0)
                                m_nWindowsEdition = WindowsEdition::AdvancedServer;
                            else
                                m_nWindowsEdition = WindowsEdition::Server;
                            break;
                        default:
                            if ((m_osvi.wSuiteMask & VER_SUITE_DATACENTER) != 0)
                                m_nWindowsEdition = WindowsEdition::DatacenterServer;
                            else if ((m_osvi.wSuiteMask & VER_SUITE_ENTERPRISE) != 0)
                                m_nWindowsEdition = WindowsEdition::EnterpriseServer;
                            else if ((m_osvi.wSuiteMask & VER_SUITE_BLADE) != 0)
                                m_nWindowsEdition = WindowsEdition::WebServer;
                            else
                                m_nWindowsEdition = WindowsEdition::StandardServer;
                            break;
                    }
                    break;
            }
        }
        else if (m_osvi.dwMajorVersion >= 6)
        {
            DWORD dwReturnedProductType = DetectProductInfo();
            switch (dwReturnedProductType)
            {
                case PRODUCT_UNDEFINED:
                    m_nWindowsEdition = WindowsEdition::EditionUnknown;
                    break;
                case PRODUCT_ULTIMATE:
                    m_nWindowsEdition = WindowsEdition::Ultimate;
                    break;
                case PRODUCT_HOME_BASIC:
                    m_nWindowsEdition = WindowsEdition::HomeBasic;
                    break;
                case PRODUCT_HOME_PREMIUM:
                    m_nWindowsEdition = WindowsEdition::HomePremium;
                    break;
                case PRODUCT_ENTERPRISE:
                    m_nWindowsEdition = WindowsEdition::Enterprise;
                    break;
                case PRODUCT_HOME_BASIC_N:
                    m_nWindowsEdition = WindowsEdition::HomeBasicN;
                    break;
                case PRODUCT_BUSINESS:
                    m_nWindowsEdition = WindowsEdition::Business;
                    break;
                case PRODUCT_STANDARD_SERVER:
                    m_nWindowsEdition = WindowsEdition::StandardServer;
                    break;
                case PRODUCT_DATACENTER_SERVER:
                    m_nWindowsEdition = WindowsEdition::DatacenterServer;
                    break;
                case PRODUCT_SMALLBUSINESS_SERVER:
                    m_nWindowsEdition = WindowsEdition::SmallBusinessServer;
                    break;
                case PRODUCT_ENTERPRISE_SERVER:
                    m_nWindowsEdition = WindowsEdition::EnterpriseServer;
                    break;
                case PRODUCT_STARTER:
                    m_nWindowsEdition = WindowsEdition::Starter;
                    break;
                case PRODUCT_DATACENTER_SERVER_CORE:
                    m_nWindowsEdition = WindowsEdition::DatacenterServerCore;
                    break;
                case PRODUCT_STANDARD_SERVER_CORE:
                    m_nWindowsEdition = WindowsEdition::StandardServerCore;
                    break;
                case PRODUCT_ENTERPRISE_SERVER_CORE:
                    m_nWindowsEdition = WindowsEdition::EnterpriseServerCore;
                    break;
                case PRODUCT_ENTERPRISE_SERVER_IA64:
                    m_nWindowsEdition = WindowsEdition::EnterpriseServerIA64;
                    break;
                case PRODUCT_BUSINESS_N:
                    m_nWindowsEdition = WindowsEdition::BusinessN;
                    break;
                case PRODUCT_WEB_SERVER:
                    m_nWindowsEdition = WindowsEdition::WebServer;
                    break;
                case PRODUCT_CLUSTER_SERVER:
                    m_nWindowsEdition = WindowsEdition::ClusterServer;
                    break;
                case PRODUCT_HOME_SERVER:
                    m_nWindowsEdition = WindowsEdition::HomeServer;
                    break;
                case PRODUCT_STORAGE_EXPRESS_SERVER:
                    m_nWindowsEdition = WindowsEdition::StorageExpressServer;
                    break;
                case PRODUCT_STORAGE_STANDARD_SERVER:
                    m_nWindowsEdition = WindowsEdition::StorageStandardServer;
                    break;
                case PRODUCT_STORAGE_WORKGROUP_SERVER:
                    m_nWindowsEdition = WindowsEdition::StorageWorkgroupServer;
                    break;
                case PRODUCT_STORAGE_ENTERPRISE_SERVER:
                    m_nWindowsEdition = WindowsEdition::StorageEnterpriseServer;
                    break;
                case PRODUCT_SERVER_FOR_SMALLBUSINESS:
                    m_nWindowsEdition = WindowsEdition::ServerForSmallBusiness;
                    break;
                case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
                    m_nWindowsEdition = WindowsEdition::SmallBusinessServerPremium;
                    break;
                case PRODUCT_HOME_PREMIUM_N:
                    m_nWindowsEdition = WindowsEdition::HomePremiumN;
                    break;
                case PRODUCT_ENTERPRISE_N:
                    m_nWindowsEdition = WindowsEdition::EnterpriseN;
                    break;
                case PRODUCT_ULTIMATE_N:
                    m_nWindowsEdition = WindowsEdition::UltimateN;
                    break;
                case PRODUCT_WEB_SERVER_CORE:
                    m_nWindowsEdition = WindowsEdition::WebServerCore;
                    break;
                case PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT:
                    m_nWindowsEdition = WindowsEdition::MediumBusinessServerManagement;
                    break;
                case PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY:
                    m_nWindowsEdition = WindowsEdition::MediumBusinessServerSecurity;
                    break;
                case PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING:
                    m_nWindowsEdition = WindowsEdition::MediumBusinessServerMessaging;
                    break;
                case PRODUCT_SERVER_FOUNDATION:
                    m_nWindowsEdition = WindowsEdition::ServerFoundation;
                    break;
                case PRODUCT_HOME_PREMIUM_SERVER:
                    m_nWindowsEdition = WindowsEdition::HomePremiumServer;
                    break;
                case PRODUCT_SERVER_FOR_SMALLBUSINESS_V:
                    m_nWindowsEdition = WindowsEdition::ServerForSmallBusinessV;
                    break;
                case PRODUCT_STANDARD_SERVER_V:
                    m_nWindowsEdition = WindowsEdition::StandardServerV;
                    break;
                case PRODUCT_DATACENTER_SERVER_V:
                    m_nWindowsEdition = WindowsEdition::DatacenterServerV;
                    break;
                case PRODUCT_ENTERPRISE_SERVER_V:
                    m_nWindowsEdition = WindowsEdition::EnterpriseServerV;
                    break;
                case PRODUCT_DATACENTER_SERVER_CORE_V:
                    m_nWindowsEdition = WindowsEdition::DatacenterServerCoreV;
                    break;
                case PRODUCT_STANDARD_SERVER_CORE_V:
                    m_nWindowsEdition = WindowsEdition::StandardServerCoreV;
                    break;
                case PRODUCT_ENTERPRISE_SERVER_CORE_V:
                    m_nWindowsEdition = WindowsEdition::EnterpriseServerCoreV;
                    break;
                case PRODUCT_HYPERV:
                    m_nWindowsEdition = WindowsEdition::HyperV;
                    break;
                case PRODUCT_STORAGE_EXPRESS_SERVER_CORE:
                    m_nWindowsEdition = WindowsEdition::StorageExpressServerCore;
                    break;
                case PRODUCT_STORAGE_STANDARD_SERVER_CORE:
                    m_nWindowsEdition = WindowsEdition::StorageStandardServerCore;
                    break;
                case PRODUCT_STORAGE_WORKGROUP_SERVER_CORE:
                    m_nWindowsEdition = WindowsEdition::StorageWorkgroupServerCore;
                    break;
                case PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE:
                    m_nWindowsEdition = WindowsEdition::StorageEnterpriseServerCore;
                    break;
                case PRODUCT_STARTER_N:
                    m_nWindowsEdition = WindowsEdition::StarterN;
                    break;
                case PRODUCT_PROFESSIONAL:
                    m_nWindowsEdition = WindowsEdition::Professional;
                    break;
                case PRODUCT_PROFESSIONAL_N:
                    m_nWindowsEdition = WindowsEdition::ProfessionalN;
                    break;
                case PRODUCT_SB_SOLUTION_SERVER:
                    m_nWindowsEdition = WindowsEdition::SBSolutionServer;
                    break;
                case PRODUCT_SERVER_FOR_SB_SOLUTIONS:
                    m_nWindowsEdition = WindowsEdition::ServerForSBSolutions;
                    break;
                case PRODUCT_STANDARD_SERVER_SOLUTIONS:
                    m_nWindowsEdition = WindowsEdition::StandardServerSolutions;
                    break;
                case PRODUCT_STANDARD_SERVER_SOLUTIONS_CORE:
                    m_nWindowsEdition = WindowsEdition::StandardServerSolutionsCore;
                    break;
                case PRODUCT_SB_SOLUTION_SERVER_EM:
                    m_nWindowsEdition = WindowsEdition::SBSolutionServerEM;
                    break;
                case PRODUCT_SERVER_FOR_SB_SOLUTIONS_EM:
                    m_nWindowsEdition = WindowsEdition::ServerForSBSolutionsEM;
                    break;
                case PRODUCT_SOLUTION_EMBEDDEDSERVER:
                    m_nWindowsEdition = WindowsEdition::SolutionEmbeddedServer;
                    break;
                case PRODUCT_SOLUTION_EMBEDDEDSERVER_CORE:
                    m_nWindowsEdition = WindowsEdition::SolutionEmbeddedServerCore;
                    break;
                case PRODUCT_ESSENTIALBUSINESS_SERVER_MGMT:
                    m_nWindowsEdition = WindowsEdition::EssentialBusinessServerMGMT;
                    break;
                case PRODUCT_ESSENTIALBUSINESS_SERVER_ADDL:
                    m_nWindowsEdition = WindowsEdition::EssentialBusinessServerADDL;
                    break;
                case PRODUCT_ESSENTIALBUSINESS_SERVER_MGMTSVC:
                    m_nWindowsEdition = WindowsEdition::EssentialBusinessServerMGMTSVC;
                    break;
                case PRODUCT_ESSENTIALBUSINESS_SERVER_ADDLSVC:
                    m_nWindowsEdition = WindowsEdition::EssentialBusinessServerADDLSVC;
                    break;
                case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM_CORE:
                    m_nWindowsEdition = WindowsEdition::SmallBusinessServerPremiumCore;
                    break;
                case PRODUCT_CLUSTER_SERVER_V:
                    m_nWindowsEdition = WindowsEdition::ClusterServerV;
                    break;
                case PRODUCT_EMBEDDED:
                    m_nWindowsEdition = WindowsEdition::Embedded;
                    break;
                case PRODUCT_STARTER_E:
                    m_nWindowsEdition = WindowsEdition::StarterE;
                    break;
                case PRODUCT_HOME_BASIC_E:
                    m_nWindowsEdition = WindowsEdition::HomeBasicE;
                    break;
                case PRODUCT_HOME_PREMIUM_E:
                    m_nWindowsEdition = WindowsEdition::HomePremiumE;
                    break;
                case PRODUCT_PROFESSIONAL_E:
                    m_nWindowsEdition = WindowsEdition::ProfessionalE;
                    break;
                case PRODUCT_ENTERPRISE_E:
                    m_nWindowsEdition = WindowsEdition::EnterpriseE;
                    break;
                case PRODUCT_ULTIMATE_E:
                    m_nWindowsEdition = WindowsEdition::UltimateE;
                    break;
                case PRODUCT_ENTERPRISE_EVALUATION:
                    m_nWindowsEdition = WindowsEdition::EnterpriseEvaluation;
                    break;
                case PRODUCT_MULTIPOINT_STANDARD_SERVER:
                    m_nWindowsEdition = WindowsEdition::MultipointStandardServer;
                    break;
                case PRODUCT_MULTIPOINT_PREMIUM_SERVER:
                    m_nWindowsEdition = WindowsEdition::MultipointPremiumServer;
                    break;
                case PRODUCT_STANDARD_EVALUATION_SERVER:
                    m_nWindowsEdition = WindowsEdition::StandardEvaluationServer;
                    break;
                case PRODUCT_DATACENTER_EVALUATION_SERVER:
                    m_nWindowsEdition = WindowsEdition::DatacenterEvaluationServer;
                    break;
                case PRODUCT_ENTERPRISE_N_EVALUATION:
                    m_nWindowsEdition = WindowsEdition::EnterpriseNEvaluation;
                    break;
                case PRODUCT_STORAGE_WORKGROUP_EVALUATION_SERVER:
                    m_nWindowsEdition = WindowsEdition::StorageWorkgroupEvaluationServer;
                    break;
                case PRODUCT_STORAGE_STANDARD_EVALUATION_SERVER:
                    m_nWindowsEdition = WindowsEdition::StorageStandardEvaluationServer;
                    break;
                case PRODUCT_CORE_N:
                    m_nWindowsEdition = WindowsEdition::CoreN;
                    break;
                case PRODUCT_CORE_COUNTRYSPECIFIC:
                    m_nWindowsEdition = WindowsEdition::CoreCountrySpecific;
                    break;
                case PRODUCT_CORE_SINGLELANGUAGE:
                    m_nWindowsEdition = WindowsEdition::CoreSingleLanguage;
                    break;
                case PRODUCT_CORE:
                    m_nWindowsEdition = WindowsEdition::Core;
                    break;
                case PRODUCT_PROFESSIONAL_WMC:
                    m_nWindowsEdition = WindowsEdition::ProfessionalWindowsMediaCenter;
                    break;
            }
        }
    }
}

void QzSysInfo::DetectWindowsServicePack()
{
    // Display service pack (if any) and build number.
    if (m_osvi.dwMajorVersion == 4 && lstrcmpi(m_osvi.szCSDVersion, L"Service Pack 6") == 0)
    {
        QSettings registry("HKEY_LOCAL_MACHINE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix", QSettings::NativeFormat);

        // Test for SP6 versus SP6a.
        if (registry.childGroups().contains("Q246009"))
        {
            sprintf(m_sServicePack, "Service Pack 6a (Build %lu)", m_osvi.dwBuildNumber & 0xFFFF);
        }
        else // Windows NT 4.0 prior to SP6a
        {
            char* sCSDVersion = new char[128];
            wcstombs(sCSDVersion, m_osvi.szCSDVersion, sizeof(m_osvi.szCSDVersion));
            sprintf(m_sServicePack, "%s (Build %lu)", sCSDVersion, m_osvi.dwBuildNumber & 0xFFFF);
        }
    }
    else // Windows NT 3.51 and earlier or Windows 2000 and later
    {
        char* sCSDVersion = new char[128];
        wcstombs(sCSDVersion, m_osvi.szCSDVersion, sizeof(m_osvi.szCSDVersion));
        sprintf(m_sServicePack, "%s (Build %lu)", sCSDVersion, m_osvi.dwBuildNumber & 0xFFFF);
    }
}

DWORD QzSysInfo::DetectProductInfo()
{
    DWORD dwProductInfo = PRODUCT_UNDEFINED;

    if (QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA)
    {
        if (m_osvi.dwMajorVersion >= 6)
        {
            PGetProductInfo lpProducInfo = (PGetProductInfo)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "GetProductInfo");

            if (lpProducInfo != 0)
                lpProducInfo(m_osvi.dwMajorVersion,
                             m_osvi.dwMinorVersion,
                             m_osvi.wServicePackMajor,
                             m_osvi.wServicePackMinor,
                             &dwProductInfo);
        }
    }

    return dwProductInfo;
}

WindowsVersion::WindowsVersion QzSysInfo::GetWindowsVersion() const
{
    return m_nWindowsVersion;
}

WindowsEdition::WindowsEdition QzSysInfo::GetWindowsEdition() const
{
    return m_nWindowsEdition;
}

bool QzSysInfo::IsNTPlatform() const
{
    return m_osvi.dwPlatformId == VER_PLATFORM_WIN32_NT;
}

bool QzSysInfo::IsWindowsPlatform() const
{
    return m_osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS;
}

bool QzSysInfo::IsWin32sPlatform() const
{
    return m_osvi.dwPlatformId == VER_PLATFORM_WIN32s;
}

DWORD QzSysInfo::GetMajorVersion() const
{
    return m_osvi.dwMajorVersion;
}

DWORD QzSysInfo::GetMinorVersion() const
{
    return m_osvi.dwMinorVersion;
}

DWORD QzSysInfo::GetBuildNumber() const
{
    return m_osvi.dwBuildNumber;
}

DWORD QzSysInfo::GetPlatformID() const
{
    return m_osvi.dwPlatformId;
}

QString QzSysInfo::GetServicePackInfo() const
{
    QString servicePack = m_sServicePack;

    if (servicePack.isEmpty())
        return "";
    else
        return servicePack;
}

bool QzSysInfo::Is32bitPlatform() const
{
    return !Is64bitPlatform();
}

bool QzSysInfo::Is64bitPlatform() const
{
    return (m_SysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 ||
            m_SysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
            m_SysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ALPHA64);
}
#endif //Q_OS_WIN
