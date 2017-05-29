#!/bin/sh 
###############################################################################
# Configure and compile QupZilla source.
###############################################################################
set -e

SCRIPT_PATH="$(dirname "$(readlink -f "$0")")"
NCPUS=$(getconf _NPROCESSORS_ONLN)  || :

which mksquashfs >/dev/null 2>&1 || TEST=no 
which chrpath >/dev/null 2>&1 || TEST=no
TEST=${TEST:-yes}

if [ $(which qmake 2>/dev/null) ]; then
SYSTEM_QMAKE=$(which qmake)
elif [ $(which qmake-qt5 2>/dev/null) ]; then
SYSTEM_QMAKE=$(which qmake-qt5)
fi

BLD1="\033[1m"
BLD0="\033[21m"
ITL1="\033[3m"
ITL0="\033[23m"
UDR1="\033[4m"
UDR0="\033[24m"
CRS1="\033[9m"
CRS0="\033[29m"
RDFG="\033[31m"
RDBG="\033[41m"
DFFG="\033[39m"
DFBG="\033[49m"
ALL0="\033[00m"

NO_SYSTEM_DATAPATH=${NO_SYSTEM_DATAPATH:-false} ; export NO_SYSTEM_DATAPATH
PORTABLE_BUILD=${PORTABLE_BUILD:-false} ; export PORTABLE_BUILD
NONBLOCK_JS_DIALOGS=${NONBLOCK_JS_DIALOGS:-false} ; export NONBLOCK_JS_DIALOGS
NO_X11=${NO_X11:-false} ; export NO_X11
USE_WEBGL=${USE_WEBGL:-true} ; export USE_WEBGL
KDE_INTEGRATION=${KDE_INTEGRATION:-false} ; export KDE_INTEGRATION
GNOME_INTEGRATION=${GNOME_INTEGRATION:-false} ; export GNOME_INTEGRATION
DISABLE_DBUS=${DISABLE_DBUS:-false} ; export DISABLE_DBUS
QMAKE=${QMAKE:-$SYSTEM_QMAKE} ; export QMAKE
SOURCE_DIR=${SOURCE_DIR:-${SCRIPT_PATH}/..} ; export SOURCE_DIR

CFLAGS="${CFLAGS:--O2 -g -pipe -Wall }" ; export CFLAGS ; 
CXXFLAGS="${CXXFLAGS:--O2 -g -pipe -Wall }" ; export CXXFLAGS ;
LDFLAGS="${LDFLAGS:--Wl,-z,relro }"; export LDFLAGS ; 

optPrint(){
printf "\n\t\t${ITL1}VALID OPTIONS ARE${ITL0}:\n
         --prefix=[path]
         --sourcedir=[path]
         --shared=[path]
         --libdir=[path]
         --qmake=[path to executable]
         --debug | -D
         --appimage | -A
         --runtime=[path]
         --portable
         --nojs
         --no-x11
         --with-kde | -K
         --with-gnome | -G
         --nodatapath
         --nodbus
         --disable-webgl
         --with-system-qtsingleapp
         --update-git-source | -U
         --no-clean | --noclean
         --help | -h | help |-H\n\n"
}

helpPrint(){
printf "\n\t\t\t${ITL1}PARAMETERS${ITL0}:

${BLD1}--portable${BLD0}

          QupZilla won't write any data outside of path of execution.
          It will also disable plugins by default.
          (disabled by default)


${BLD1}--nojs${BLD0}

          Enable non-blocking JavaScript dialogs from alert() prompt()
          and confirm() functions. They are shown inside page and are not
          blocking application window.
          However, due to synchronous API, there is a possible crash when
          closing browser windows with opened dialogs.
          If you can take this risk and/or make sure you aren't closing browser
          with opened dialogs, you may enable this option.
          These dialogs are much more beautiful than normal QDialogs.
          (disabled by default)


${BLD1}--no-x11${BLD0}

          Disable all X11 calls.
          Enable this when building for Wayland-only.
          All X11 calls are guarded by runtime X11 platform check
          even without this option.


${BLD1}--with-kde | -K ${BLD0}

          Enable KDE integration.
          Currently it enables building of KWallet Password plugin,
          which provides support for storing passwords in KWallet.


${BLD1}--with-gnome | -G${BLD0}

          Enable Gnome integration.
          Currently it enables building of Gnome-Keyring Password plugin,
          which provides support for storing passwords in Gnome-Keyring.


${BLD1}--libdir=${BLD0}

          By default, /usr/lib/ is used for libQupZilla and /usr/lib/qupzilla
          for plugins.
          You can change it by setting this define.

               ${UDR1}example:--libdir="/usr/lib64"${UDR0}

${BLD1}--nodatapath=${BLD0}

          By default, QupZilla is using /usr/share/qupzilla/ path
          for storing themes and translations.
          By setting this define, QupZilla will use path of execution.
          (disabled by default)


${BLD1}--prefix=${BLD0}

          You can define different prefix.
          QupZilla binary will then be moved to PREFIX/bin/, use
          PREFIX/share/qupzilla/ as datadir, PREFIX/share/applications for
          desktop launcher and PREFIX/share/pixmaps for icon.
          (default prefix is "/usr/local")

               ${UDR1}example:--prefix="/usr"${UDR0}


${BLD1}--shared=${BLD0}

          You can define the path of the share folder, i.e. /usr/share
          QupZilla will then use shared/qupzilla as datadir,
          shared/applications for desktop launcher and
          shared/pixmaps for the icon. By default it is not defined
          and files will be installed as described above.
          (default share folder is "/usr/local/share")

               ${UDR1}example:--shared="/usr/share"${UDR0}


${BLD1}--nodbus${BLD0}

          Build without QtDBus module. Native desktop notifications
          will be disabled.


${BLD1}--sourcedir=${BLD0}

          Assuming this script is located in ${ITL1}qupzilla/scripts${ITL0},
          otherwise you must specify the path to 
          QupZilla source directory.

               ${UDR1}example:--sourcedir="/home/build/qupzilla"${UDR0}


${BLD1}--runtime=[path]${BLD0}

          Path to precompiled „${BLD1}runtime.c${BLD0}“ ${ITL1}(part of AppImageKit)${ITL0}.
          More info at: ${UDR1}https://github.com/probonopd/AppImageKit${UDR0}
          Requires ${BLD1}--appimage${BLD0}

${BLD1}--appimage | -A${BLD0}

          Also create an AppImage.
          In order to build AppImage, 
          you must have installed the following tools:
          ${ITL1}mmksquashfs, patchelf${ITL0}!
          Also, you should use precompiled Qt package
          downloaded from ${UDR1}${ITL1}http://download.qt.io/official_releases/qt/${ALL0},
          and precompiled „${BLD1}runtime${BLD0}“ binary - part of ${ITL1}AppImageKit${ITL0} 
          ${UDR1}https://github.com/probonopd/AppImageKit${UDR0},
          otherwise this option will be ignored.
          Requires ${BLD1}--runtime${BLD0}
          Implies ${BLD1}--nodatapath${BLD0}


${BLD1}--disable-webgl${BLD0}
          
          Disable WebGL. You need to build QupZilla with WebKit built
          with WebGL support, otherwise you won't be able to compile
          without errors.
          Only for QtWebKit lower than 2.3
          (disabled by default)


${BLD1}--with-system-qtsingleapp${BLD0}

          Use system QtSingleApplication library.
          This option can only be used with system qmake!


${BLD1}--debug | -D${BLD0}

          You may want to build QupZilla with debugging symbols (for generating
          backtrace of crash).


${BLD1}--update-git-source | -U${BLD0}

          Fetches the information from QupZilla online git repository
          and merges it with your local copy


${BLD1}--no-clean | --noclean${BLD0}

          Skip cleaning previously compilled files.

${BLD1}--qmake=${BLD0}

          Full path to qmake executable.
          This option is mandatory in case you want
          to create an AppImage.
\n"
}

printConf(){
printf "\n\tBuild configuration:\n
    QUPZILLA_PREFIX=${QUPZILLA_PREFIX}
    SOURCE_DIR=${SOURCE_DIR}
    SHARE_FOLDER=${SHARE_FOLDER}
    USE_LIBPATH=${USE_LIBPATH}
    DEBUG_BUILD=${DEBUG_BUILD}
    BUILD_AI=${BUILD_AI}
    PORTABLE_BUILD=${PORTABLE_BUILD}
    NONBLOCK_JS_DIALOGS=${NONBLOCK_JS_DIALOGS}
    NO_X11=${NO_X11}
    KDE_INTEGRATION=${KDE_INTEGRATION}
    GNOME_INTEGRATION=${GNOME_INTEGRATION}
    NO_SYSTEM_DATAPATH=${NO_SYSTEM_DATAPATH}
    DISABLE_DBUS=${DISABLE_DBUS}
    USE_WEBGL=${USE_WEBGL}
    SKIP_CLEANNING=${SKIP_CLEANNING}
    USE_SYS_QTSA=${USE_SYS_QTSA}
    RUNTIME_BINARY=${RUNTIME_BINARY}
    QMAKE=${QMAKE}\n" | sed -r 's/=$/ » Not set/g'
}

getVal(){
echo $* | sed -r 's/([[:graph:]]*=)//'
}

varAssign(){
while [ $# != 0 ] ;do
        CFG_OPT="$1"
        case "${CFG_OPT}" in
         --prefix=*)
               QUPZILLA_PREFIX=$(getVal "${CFG_OPT}")
                export QUPZILLA_PREFIX
             ;;
         --sourcedir=*)
               SOURCE_DIR=$(getVal "${CFG_OPT}")
                export SOURCE_DIR
             ;;
         --shared=*)
                SHARE_FOLDER=$(getVal "${CFG_OPT}")
                export SHARE_FOLDER
             ;;
         --libdir=*)
                USE_LIBPATH=$(getVal "${CFG_OPT}")
                export USE_LIBPATH
             ;;
         --debug|-D)
                DEBUG_BUILD="CONFIG+=debug"
                export DEBUG_BUILD
             ;;
         --appimage|-A)
                BUILD_AI="true"
                export BUILD_AI
             ;;
         --runtime=*)
                RUNTIME_BINARY=$(getVal "${CFG_OPT}")
                export RUNTIME_BINARY
             ;;
         --portable)
                PORTABLE_BUILD="true"
                export PORTABLE_BUILD
             ;;
         --nojs)
                NONBLOCK_JS_DIALOGS="true"
                export NONBLOCK_JS_DIALOGS
             ;;
         --no-clean|--noclean)
                SKIP_CLEANNING="true"
                export SKIP_CLEANNING
             ;;
         --no-x11)
                NO_X11="true"
                export NO_X11
             ;;
         --with-kde|-K)
                KDE_INTEGRATION="true"
                export KDE_INTEGRATION
             ;;
         --with-gnome|-G)
                GNOME_INTEGRATION="true"
                export GNOME_INTEGRATION
             ;;
         --nodatapath)
                NO_SYSTEM_DATAPATH="true"
                export NO_SYSTEM_DATAPATH
             ;;
         --nodbus)
                DISABLE_DBUS="true"
                export DISABLE_DBUS
             ;;
         --disable-webgl)
                USE_WEBGL="false"
                export USE_WEBGL
             ;;
         --with-system-qtsingleapp)
               USE_SYS_QTSA="CONFIG+=QtSingleApplication"
                export USE_SYS_QTSA
             ;;
         --qmake=*)
                QMAKE=$(getVal "${CFG_OPT}")
                export QMAKE
             ;;
         --update-git-source|-U)
                UPDATE_SOURCE="true"
                export UPDATE_SOURCE
             ;;
         --help|help|-h|-H)
                helpPrint
                exit 1
             ;;
        *)
                printf "\n${RDBG}unknown parameter: ${CFG_OPT}${DFBG}\n"
                optPrint
                exit 1
                ;;
     esac
     shift
done
}

nowBuild()
{
QUPZILLA_PREFIX=${QUPZILLA_PREFIX:-/usr/local} ; export QUPZILLA_PREFIX
SHARE_FOLDER=${SHARE_FOLDER:-$QUPZILLA_PREFIX/share} ; export SHARE_FOLDER
USE_LIBPATH=${USE_LIBPATH:-$QUPZILLA_PREFIX/lib} ; export USE_LIBPATH

printConf

cd "${SOURCE_DIR}"

if [[ -z "${SKIP_CLEANNING}" ]]; then
make distclean >/dev/null 2>&1 || :
rm -fr bundle_build_dir qupzilla.squashfs bin/QupZilla.AppImage || :
fi

if [[ "${UPDATE_SOURCE}" == "true" ]]; then
git pull || :
fi 

if [[ ! -z "${USE_SYS_QTSA}" ]]; then
HEADERSARETHERE=$(${QMAKE} -query | grep INSTALL_HEADERS | sed 's/QT_INSTALL_HEADERS://')
rm -fr src/lib/3rdparty/qtsingleapplication
ln -s ${HEADERSARETHERE}/QtSolutions src/lib/3rdparty/qtsingleapplication
sed -i 's,include.*qtsingleapplication.*,,' src/plugins.pri
sed -i 's,include.*qtsingleapplication.*,,' src/lib/lib.pro
fi

${QMAKE} ${DEBUG_BUILD} ${USE_SYS_QTSA}
printf "Compiling QupZilla\n"
make -j$NCPUS
}

nowBldImg(){
NO_SYSTEM_DATAPATH="true"; export NO_SYSTEM_DATAPATH

QTFILESARETHERE=$(${QMAKE} -query | grep INSTALL_PREFIX | sed 's/QT_INSTALL_PREFIX://')
LIBSARETHERE=$(${QMAKE} -query | grep INSTALL_LIBS | sed 's/QT_INSTALL_LIBS://')
PLUGINSARETHERE=$(${QMAKE} -query | grep INSTALL_PLUGINS | sed 's/QT_INSTALL_PLUGINS://')
QMLSARETHERE=$(${QMAKE} -query | grep INSTALL_QML | sed 's/QT_INSTALL_QML://')
TRANSLATIONSARETHERE=$(${QMAKE} -query | grep INSTALL_TRANSLATIONS | sed 's/QT_INSTALL_TRANSLATIONS://')
LIBEXECSARETHERE=$(${QMAKE} -query | grep INSTALL_LIBEXECS | sed 's/QT_INSTALL_LIBEXECS://')

NEEDEDLIBSLIST="libicudata.so
libicui18n.so
libicuuc.so
libQt5Core.so
libQt5DBus.so
libQt5Gui.so
libQt5Multimedia.so
libQt5MultimediaWidgets.so
libQt5Network.so
libQt5OpenGL.so
libQt5Positioning.so
libQt5PrintSupport.so
libQt5Qml.so
libQt5Quick.so
libQt5QuickWidgets.so
libQt5Sql.so
libQt5Svg.so
libQt5WebChannel.so
libQt5WebEngineCore.so
libQt5WebEngineCore.so
libQt5WebEngine.so
libQt5WebEngineWidgets.so
libQt5Widgets.so
libQt5X11Extras.so
libQt5XcbQpa.so"

NEEDEDPLUGINSLIST="generic
iconengines
imageformats
platforminputcontexts
platformthemes
printsupport
xcbglintegrations"

nowBuild

mkdir bundle_build_dir || :
cp -r bin/* bundle_build_dir 
pushd bundle_build_dir/plugins 
patchelf --set-rpath '$ORIGIN/../lib' *.so 
popd 

mkdir -p bundle_build_dir/lib \
bundle_build_dir/plugins/{platforms,sqldrivers} \
bundle_build_dir/qtwebengine_dictionaries \
bundle_build_dir/qml \
bundle_build_dir/translations || :
for L in ${NEEDEDLIBSLIST} ; do
cp -d ${LIBSARETHERE}/${L}* bundle_build_dir/lib ;
done

for P in ${NEEDEDPLUGINSLIST} ; do
cp -r ${PLUGINSARETHERE}/${P} bundle_build_dir/plugins ;
done
install ${PLUGINSARETHERE}/platforms/libqxcb.so bundle_build_dir/plugins/platforms
install ${PLUGINSARETHERE}/sqldrivers/libqsqlite.so bundle_build_dir/plugins/sqldrivers
cp -r ${QMLSARETHERE}/{QtQuick.2,QtWebEngine} bundle_build_dir/qml
cp -r ${QTFILESARETHERE}/resources bundle_build_dir
cp -r ${TRANSLATIONSARETHERE}/qtwebengine_locales bundle_build_dir/translations
cp ${LIBEXECSARETHERE}/QtWebEngineProcess bundle_build_dir

CRYPTONEEDED=$(ldd 'bin/qupzilla'| grep libcrypto | sed 's/.*=>//;s/(.*//')
cp ${CRYPTONEEDED} bundle_build_dir/lib

cp linux/applications/qupzilla.desktop bundle_build_dir
cp linux/pixmaps/qupzilla.png bundle_build_dir
ln -sf qupzilla.png bundle_build_dir/.DirIcon

pushd bundle_build_dir 
patchelf --set-rpath '$ORIGIN:$ORIGIN/lib' libQupZilla.so
patchelf --set-rpath '$ORIGIN:$ORIGIN/lib' qupzilla 
patchelf --set-rpath '$ORIGIN:$ORIGIN/lib' QtWebEngineProcess

cat <<EOQTCFG >qt.conf
[Paths]
Plugins=plugins
Imports=qml
Qml2Imports=qml
LibraryExecutables=.
EOQTCFG

cat <<EOF >AppRun
#!/bin/sh
#
set -e

QUPZILLA_MOUNT_PATH="\$(dirname "\$(readlink -f "\$0")")"

QT_DIR=\${QUPZILLA_MOUNT_PATH}
export QT_DIR

QT_QPA_PLATFORM_PLUGIN_PATH="\${QT_DIR}/plugins/platforms"
QT_PLUGIN_PATH="\${QT_DIR}/plugins"
QML2_IMPORT_PATH="\${QT_DIR}/qml"
QTWEBENGINEPROCESS_PATH="\${QT_DIR}/QtWebEngineProcess"

export QT_QPA_PLATFORM_PLUGIN_PATH QT_PLUGIN_PATH QML2_IMPORT_PATH QTWEBENGINEPROCESS_PATH 

cd "\${QUPZILLA_MOUNT_PATH}/"
exec ./qupzilla "\$@" 
EOF
chmod +x AppRun 
popd 

printf "Generating app image\n"
mksquashfs bundle_build_dir qupzilla.squashfs -root-owned -noappend 

cat "${RUNTIME_BINARY}" >bin/QupZilla.AppImage
cat qupzilla.squashfs >>bin/QupZilla.AppImage 
chmod a+x bin/QupZilla.AppImage
}

varAssign $*

if [[ ! -x ${QMAKE} ]] ;then
printf "${RDFG}ERROR${DFFG}: ${BLD1}qmake${BLD0} was not found! Please install it or use ${BLD1}--qmake=${BLD0} option to specify the path where it is located!\n"
exit 1
fi

if [[ ! -z "${USE_SYS_QTSA}" ]] && [[ ${QMAKE} != ${SYSTEM_QMAKE} ]] ;then
printf "${RDFG}ERROR${DFFG}: You must use system qmake in order to build Qupzilla with system QtsingleApplication libraries!\n"
exit 1
fi

if [[ ! -d "${SOURCE_DIR}/src" ]]; then
printf "Please install ${UDR1}$0${UDR0} in „${BLD1}scripts${BLD0}“ ${ITL1}(a sub folder in QupZilla source directory)${ITL0},
or specify the source path with ${BLD1}--sourcedir=${BLD0} parameter!\n"
exit 1
fi 


if [[ ${BUILD_AI} == "true" ]] && [[ ${TEST} == "yes" ]] && [[ ${QMAKE} != ${SYSTEM_QMAKE} ]] && [[ ! -z ${RUNTIME_BINARY} ]] ; then
nowBldImg
else
nowBuild
fi

if [[ $? == 0 ]] && [[ -x ${SOURCE_DIR}/bin/qupzilla ]]; then
 printf "\\033c"
 printf "Done!\nThe compiled files are in "${PWD}"/bin\n"
 if [[ -z "${BUILD_AI}" ]]; then
 printf "Now you can type „${ITL1}make install${ITL}“ to install them\n"
 fi
fi

exit 0
