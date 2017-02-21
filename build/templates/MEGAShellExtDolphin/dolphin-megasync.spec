Name:       dolphin-megasync
Version:    EXT_VERSION
Release:	1%{?dist}
Summary:	Extension for Dolphin to interact with Megasync
License:	Freeware
Group:		Applications/Others
Url:		https://mega.nz
Source0:	dolphin-megasync_%{version}.tar.gz
Vendor:		MEGA Limited
Packager:	MEGA Linux Team <linux@mega.co.nz>

#BuildRequires:  libqt4-dev, kdelibs5-dev, cmake
BuildRequires:  qt-devel
%if 0%{?suse_version}
BuildRequires:  libkde4-devel
%endif
%if 0%{?fedora}
BuildRequires:  kdelibs, kdelibs-devel
%if 0%{?fedora_version} <= 23
BuildRequires: qca2
%endif
%endif

Requires:       megasync

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
qmake-qt4 INCLUDEPATH+=$(kde4-config --path include)
%else
qmake INCLUDEPATH+=$(kde4-config --path include)
%endif

make

%install
export DESKTOPEXTDIR=$(kde4-config --path services | awk -NF ":" '{print $NF}')
#export LIBEXTENSIONDIR=$(kde4-config --path module | awk -NF ":" '{print $NF}')
make install INSTALL_ROOT=%{buildroot}
# INSTALL_ROOT=%{buildroot}$LIBEXTENSIONDIR

mkdir -p %{buildroot}$DESKTOPEXTDIR
%{__install} megasync-plugin.desktop -D %{buildroot}$DESKTOPEXTDIR

%clean
%{?buildroot:%__rm -rf "%{buildroot}"}

%files
%defattr(-,root,root)

%(kde4-config --path services | awk -NF ":" '{print $NF}')/megasync-plugin.desktop
%(kde4-config --path module | awk -NF ":" '{print $NF}')/lib*.so*

%changelog
