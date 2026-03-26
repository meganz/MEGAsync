Name:       thunar-megasync
Version:    EXT_VERSION
Release:	%(cat MEGA_BUILD_ID || echo "1").1
Summary:	MEGA Desktop App plugin for Thunar
License:	Freeware
Group:		Applications/Others
Url:		https://mega.io/desktop
Source0:	thunar-megasync_%{version}.tar.gz
Vendor:		MEGA Limited
Packager:	MEGA Linux Team <linux@mega.io>

BuildRequires: cmake, gcc-c++

%if 0%{?rhel_version} || 0%{?centos_version} || 0%{?almalinux} || 0%{?rhel}
%global debug_package %{nil}
BuildRequires: redhat-rpm-config
BuildRequires: Thunar-devel
%endif

%if 0%{?suse_version}
BuildRequires:  glib2-devel, thunar-devel
%endif

%if 0%{?fedora}
BuildRequires: Thunar-devel
%global debug_package %{nil}
%endif


Requires:       thunar, megasync >= 5.3.0

%description
- Easily see and track your sync statuses.

- Send files and folders to MEGA.

- Share your synced files and folders with anyone by creating links.

- View files in MEGA's browser (webclient).

%prep
%setup -q

%build
cmake -S . -B %{_builddir} -DCMAKE_INSTALL_PREFIX=%{_prefix}
cmake --build %{_builddir}

%install
DESTDIR=%{buildroot} cmake --install %{_builddir}
echo "== INSTALLED TREE =="
find %{buildroot} -type f | sort

%clean
rm -fr %{buildroot}/%{_datadir}/icons/hicolor/icon-theme.cache || true

%files
%defattr(-,root,root)
%(pkg-config --variable=extensionsdir thunarx-3 || pkg-config --variable=extensionsdir thunarx-2)/libMEGAShellExtThunar.so*

%changelog
