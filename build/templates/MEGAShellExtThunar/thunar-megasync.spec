Name:       thunar-megasync
Version:    EXT_VERSION
Release:	1%{?dist}
Summary:	Easy automated syncing between your computers and your MEGA cloud drive
License:	Freeware
Group:		Applications/Others
Url:		https://mega.nz
Source0:	thunar-megasync_%{version}.tar.gz
Vendor:		MEGA Limited
Packager:	MEGA Linux Team <linux@mega.co.nz>

BuildRequires:  qt-devel, glib2-devel, libthunarx-2-0, gnome-common
BuildRequires:  thunar-devel
BuildRequires:	hicolor-icon-theme, gnome-shell
Requires:       thunar, megasync

%description
Secure:
Your data is encrypted end to end. Nobody can intercept it while in storage or in transit.

Flexible:
Sync any folder from your PC to any folder in the cloud. Sync any number of folders in parallel.

Fast:
Take advantage of MEGA's high-powered infrastructure and multi-connection transfers.

Generous:
Store up to 50 GB for free!

%prep
%setup -q

%build
export DESKTOP_DESTDIR=$RPM_BUILD_ROOT/usr

%if 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version}
qmake-qt4
%else
qmake
%endif

make

%install
make install

%ifarch x86_64
mkdir -p $RPM_BUILD_ROOT%{_libdir}/x86_64-linux-gnu/thunarx-2
%{__install} libMEGAShellExtThunar.so -D $RPM_BUILD_ROOT%{_libdir}/x86_64-linux-gnu/thunarx-2
%else
mkdir -p $RPM_BUILD_ROOT%{_libdir}/i386-linux-gnu/thunarx-2
%{__install} libMEGAShellExtThunar.so -D $RPM_BUILD_ROOT%{_libdir}/i386-linux-gnu/thunarx-2
%endif

%clean
%{?buildroot:%__rm -rf "%{buildroot}"}

%files
%defattr(-,root,root)

%ifarch x86_64
%{_libdir}/x86_64-linux-gnu/thunarx-2/libMEGAShellExtThunar.so
%else
%{_libdir}/i386-linux-gnu/thunarx-2/libMEGAShellExtThunar.so
%endif

%changelog
