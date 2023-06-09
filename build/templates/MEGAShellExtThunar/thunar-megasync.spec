Name:       thunar-megasync
Version:    EXT_VERSION
Release:	%(cat MEGA_BUILD_ID || echo "1").1
Summary:	MEGA Desktop App plugin for Thunar
License:	Freeware
Group:		Applications/Others
Url:		https://mega.nz
Source0:	thunar-megasync_%{version}.tar.gz
Vendor:		MEGA Limited
Packager:	MEGA Linux Team <linux@mega.co.nz>

%if 0%{?suse_version}
BuildRequires:  glib2-devel, thunar-devel, libqt5-qtbase-devel
%endif

%if 0%{?fedora}
BuildRequires: Thunar-devel
BuildRequires: qt5-qtbase-devel
%global debug_package %{nil}
%endif

%if 0%{?rhel_version} || 0%{?centos_version}
BuildRequires: Thunar-devel
%if 0%{?rhel_version} >= 800 || 0%{?centos_version} >=800
BuildRequires: qt5-qtbase-devel
%else
BuildRequires: qt-devel
%endif
%endif

Requires:       thunar, megasync >= 3.5

%description
- Easily see and track your sync statuses.

- Send files and folders to MEGA.

- Share your synced files and folders with anyone by creating links.

- View files in MEGA's browser (webclient).

%prep
%setup -q

%build
export DESKTOP_DESTDIR=$RPM_BUILD_ROOT/usr

qmake-qt5 || qmake-qt4 || qmake

%if 0%{?fedora_version} >= 27 || 0%{?centos_version} >=800
#tweak to have debug symbols to stripe: for some reason they seem gone by default in Fedora 27,
#   causing "gdb-add-index: No index was created for ..." which lead to error "Empty %files file ....debugsourcefiles.list"
sed "s# gcc# gcc -g#g" -i Makefile
%endif
make

%install
make install

%clean
%{?buildroot:%__rm -rf "%{buildroot}"}

%files
%defattr(-,root,root)
%(pkg-config --variable=extensionsdir thunarx-3 || pkg-config --variable=extensionsdir thunarx-2)/libMEGAShellExtThunar.so*

%changelog
