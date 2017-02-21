Name:       thunar-megasync
Version:    EXT_VERSION
Release:	1%{?dist}
Summary:	Extension for Thunar to interact with Megasync
License:	Freeware
Group:		Applications/Others
Url:		https://mega.nz
Source0:	thunar-megasync_%{version}.tar.gz
Vendor:		MEGA Limited
Packager:	MEGA Linux Team <linux@mega.co.nz>


BuildRequires:  qt-devel
%if 0%{?suse_version}
BuildRequires:  glib2-devel, libthunarx-2-0, thunar-devel
%endif
%if 0%{?fedora}
BuildRequires:  Thunar-devel
%endif

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
%if 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version}
qmake-qt4
%else
qmake
%endif

make

%install
export EXTENSIONSDIR=$(pkg-config --variable=extensionsdir thunarx-2)

mkdir -p %{buildroot}$EXTENSIONSDIR

%{__install} libMEGAShellExtThunar.so -D %{buildroot}$EXTENSIONSDIR

%clean
%{?buildroot:%__rm -rf "%{buildroot}"}

%files
%defattr(-,root,root)

%(pkg-config --variable=extensionsdir thunarx-2)/libMEGAShellExtThunar.so
#%(echo $EXTENSIONSDIR/libMEGAShellExtThunar.so)
#%{getenv:EXTENSIONSDIR}/libMEGAShellExtThunar.so
#$EXTENSIONSDIR/libMEGAShellExtThunar.so

%changelog
