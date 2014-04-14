Name:		megasync
Version:	1.0
Release:	1%{?dist}
Summary:	Easy automated syncing between your computers and your MEGA cloud drive
License:	Freeware
Group:		Applications/Others
Url:		https://mega.co.nz
Source0:	megasync-%{version}.tar.gz

BuildRequires: qt-devel, curl-devel, openssl-devel, sqlite-devel, zlib-devel autoconf automake libtool gcc-c++ wget ca-certificates
Requires: ca-certificates

%if 0%{?fedora}
BuildRequires:	c-ares-devel, cryptopp-devel
%endif

%if 0%{?suse_version}
BuildRequires: libcares-devel, libcryptopp-devel
%endif

%if 0%{?rhel_version} || 0%{?centos_version}
BuildRequires: c-ares-devel, cryptopp-devel
%endif

BuildRoot:      %{_tmppath}/%{name}-%{version}-build    

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

%configure
%build
%if 0%{?fedora} || 0%{?rhel_version} > 0
qmake-qt4
lrelease-qt4 MEGASync/MEGASync.pro
%else
qmake
lrelease MEGASync/MEGASync.pro
%endif

%{__make}
%install
%{__install} -Dm 755 -s MEGASync/%{name} %{buildroot}%{_bindir}/%{name}

%clean

%files
%{_bindir}/%{name}

%defattr(-,root,root)
%changelog

