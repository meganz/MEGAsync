Name:       dolphin-megasync
Version:    EXT_VERSION
Release:	%(cat MEGA_BUILD_ID || echo "1").1
Summary:	MEGA Desktop App plugin for Dolphin
License:	Freeware
Group:		Applications/Others
Url:		https://mega.io/desktop
Source0:	dolphin-megasync_%{version}.tar.gz
Vendor:		MEGA Limited
Packager:	MEGA Linux Team <linux@mega.io>

AutoReq: 0

BuildRequires:  cmake


#OpenSUSE
%if 0%{?sle_version} && 0%{?suse_version} < 1600
BuildRequires:  kf5-filesystem
BuildRequires:  extra-cmake-modules
BuildRequires:  kio-devel
BuildRequires:  libqt5-qtbase-devel
#%global debug_package %{nil}
%endif
%if (0%{?sle_version} || 0%{?is_opensuse}) && 0%{?suse_version} >= 1600
BuildRequires:  kf6-extra-cmake-modules
BuildRequires:  kf6-kio-devel
BuildRequires:  qt6-base-devel
%endif

#Fedora specific
%if 0%{?fedora}
BuildRequires:  gcc-c++
BuildRequires:  extra-cmake-modules
%if 0%{?fedora} < 40
BuildRequires:  kf5-rpm-macros
BuildRequires:  kf5-kio-devel
%endif
%if 0%{?fedora} >= 40
BuildRequires:  kf6-rpm-macros
BuildRequires:  kf6-kio-devel
%endif
%endif

#RHEL
%if 0%{?rhel}
# Disabling -debugsource for rhel to avoid "Empty %files debugsourcefiles.list"
%global _enable_debugsource 0
BuildRequires:  gcc-c++
BuildRequires:  extra-cmake-modules
BuildRequires:  redhat-rpm-config
%if 0%{?rhel} >= 10
BuildRequires:  kf6-rpm-macros
BuildRequires:  kf6-kio-devel
%else
BuildRequires:  kf5-rpm-macros
BuildRequires:  kf5-kio-devel
%endif
%endif

Requires:       megasync >= 5.3.0

%description
- Easily see and track your sync statuses.

- Send files and folders to MEGA.

- Share your synced files and folders with anyone by creating links.

- View files in MEGA's browser (webclient).

%prep
%setup -q

%build
%if 0%{?fedora}
KIO_PACKAGE=$(rpm -qa | grep 'kio-devel-')
KF_VERSION=$(rpm -ql ${KIO_PACKAGE} | grep -m1 -oP '/usr/include/KF\K\w+(?=/)')
if [ "${KF_VERSION}" == "5" ]
then
    %cmake_kf5 -DKF_VER=${KF_VERSION}
fi
if [ "${KF_VERSION}" == "6" ]
then
    %cmake_kf6 -DKF_VER=${KF_VERSION}
fi
%cmake_build
%endif

%if 0%{?sle_version} && 0%{?suse_version} < 1600
%cmake_kf5 -d build -- -DDKF_VER=5
%make_jobs
%endif

%if (0%{?sle_version} || 0%{?is_opensuse}) && 0%{?suse_version} >= 1600
%cmake_kf6 -DKF_VER=6
%kf6_build
%endif

#RHEL
%if 0%{?rhel}
%if 0%{?rhel} >= 10
    %cmake_kf6 -DKF_VER=6
%else
    %cmake_kf5 -DKF_VER=5
%endif
%cmake_build
%endif

%install
%if 0%{?fedora}
%cmake_install
%endif
%if 0%{?sle_version} && 0%{?suse_version} < 1600
%kf5_makeinstall -C build
%endif
%if (0%{?sle_version} || 0%{?is_opensuse}) && 0%{?suse_version} >= 1600
%kf6_install
%endif
#RHEL
%if 0%{?rhel}
%cmake_install
%endif

%clean
echo cleaning
%{?buildroot:%__rm -rf "%{buildroot}"}

%files
%defattr(-,root,root)
%if 0%{?fedora}
%if 0%{?fedora} >= 40
%{_kf6_datadir}/icons/hicolor/32x32/emblems/mega-dolphin-pending.png
%{_kf6_datadir}/icons/hicolor/32x32/emblems/mega-dolphin-synced.png
%{_kf6_datadir}/icons/hicolor/32x32/emblems/mega-dolphin-syncing.png
%{_kf6_datadir}/icons/hicolor/64x64/emblems/mega-dolphin-pending.png
%{_kf6_datadir}/icons/hicolor/64x64/emblems/mega-dolphin-synced.png
%{_kf6_datadir}/icons/hicolor/64x64/emblems/mega-dolphin-syncing.png
%{_kf6_plugindir}/overlayicon/megasync-overlay-plugin.so
%{_kf6_plugindir}/kfileitemaction/megasync-plugin.so
%endif
%if 0%{?fedora} < 40
%{_kf5_datadir}/icons/hicolor/32x32/emblems/mega-dolphin-pending.png
%{_kf5_datadir}/icons/hicolor/32x32/emblems/mega-dolphin-synced.png
%{_kf5_datadir}/icons/hicolor/32x32/emblems/mega-dolphin-syncing.png
%{_kf5_datadir}/icons/hicolor/64x64/emblems/mega-dolphin-pending.png
%{_kf5_datadir}/icons/hicolor/64x64/emblems/mega-dolphin-synced.png
%{_kf5_datadir}/icons/hicolor/64x64/emblems/mega-dolphin-syncing.png
%{_kf5_plugindir}/overlayicon/megasync-overlay-plugin.so
%{_kf5_plugindir}/kfileitemaction/megasync-plugin.so
%endif
%endif
%if 0%{?sle_version} && 0%{?suse_version} < 1600
%{_kf5_iconsdir}/hicolor/32x32/emblems/mega-dolphin-pending.png
%{_kf5_iconsdir}/hicolor/32x32/emblems/mega-dolphin-synced.png
%{_kf5_iconsdir}/hicolor/32x32/emblems/mega-dolphin-syncing.png
%{_kf5_iconsdir}/hicolor/64x64/emblems/mega-dolphin-pending.png
%{_kf5_iconsdir}/hicolor/64x64/emblems/mega-dolphin-synced.png
%{_kf5_iconsdir}/hicolor/64x64/emblems/mega-dolphin-syncing.png
%{_kf5_plugindir}/kf5/overlayicon/megasync-overlay-plugin.so
%{_kf5_plugindir}/kf5/kfileitemaction/megasync-plugin.so
%endif
%if (0%{?sle_version} || 0%{?is_opensuse}) && 0%{?suse_version} >= 1600
%{_kf6_iconsdir}/hicolor/32x32/emblems/mega-dolphin-pending.png
%{_kf6_iconsdir}/hicolor/32x32/emblems/mega-dolphin-synced.png
%{_kf6_iconsdir}/hicolor/32x32/emblems/mega-dolphin-syncing.png
%{_kf6_iconsdir}/hicolor/64x64/emblems/mega-dolphin-pending.png
%{_kf6_iconsdir}/hicolor/64x64/emblems/mega-dolphin-synced.png
%{_kf6_iconsdir}/hicolor/64x64/emblems/mega-dolphin-syncing.png
%{_kf6_plugindir}/kf6/overlayicon/megasync-overlay-plugin.so
%{_kf6_plugindir}/kf6/kfileitemaction/megasync-plugin.so
%endif
#RHEL
%if 0%{?rhel}
%if 0%{?rhel} >= 10
%{_kf6_datadir}/icons/hicolor/32x32/emblems/mega-dolphin-pending.png
%{_kf6_datadir}/icons/hicolor/32x32/emblems/mega-dolphin-synced.png
%{_kf6_datadir}/icons/hicolor/32x32/emblems/mega-dolphin-syncing.png
%{_kf6_datadir}/icons/hicolor/64x64/emblems/mega-dolphin-pending.png
%{_kf6_datadir}/icons/hicolor/64x64/emblems/mega-dolphin-synced.png
%{_kf6_datadir}/icons/hicolor/64x64/emblems/mega-dolphin-syncing.png
%{_kf6_plugindir}/overlayicon/megasync-overlay-plugin.so
%{_kf6_plugindir}/kfileitemaction/megasync-plugin.so
%else
%{_kf5_datadir}/icons/hicolor/32x32/emblems/mega-dolphin-pending.png
%{_kf5_datadir}/icons/hicolor/32x32/emblems/mega-dolphin-synced.png
%{_kf5_datadir}/icons/hicolor/32x32/emblems/mega-dolphin-syncing.png
%{_kf5_datadir}/icons/hicolor/64x64/emblems/mega-dolphin-pending.png
%{_kf5_datadir}/icons/hicolor/64x64/emblems/mega-dolphin-synced.png
%{_kf5_datadir}/icons/hicolor/64x64/emblems/mega-dolphin-syncing.png
%{_kf5_plugindir}/overlayicon/megasync-overlay-plugin.so
%{_kf5_plugindir}/kfileitemaction/megasync-plugin.so
%endif
%endif

%changelog
