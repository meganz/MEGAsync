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
%if 0%{?fedora_version} >= 19
BuildRequires: kf5-kdelibs4support-devel extra-cmake-modules
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

cmake -DCMAKE_INSTALL_PREFIX="`kde4-config --prefix`" $PWD

make
export DESKTOPEXTDIR=$(kde4-config --path services | awk -NF ":" '{print $NF}')

echo mkdir -p %{buildroot}$DESKTOPEXTDIR
mkdir -p %{buildroot}$DESKTOPEXTDIR
make install DESTDIR=%{buildroot}

# Create a temporary file containing the list of files
EXTRA_FILES=%{buildroot}/ExtraFiles.list
touch %{EXTRA_FILES}

echo %(kde4-config --path services | awk -NF ":" '{print $NF}')/megasync-plugin.desktop  >> %{EXTRA_FILES}
echo %(kde4-config --path module | awk -NF ":" '{print $NF}')/megasyncplugin.so >> %{EXTRA_FILES}

if which kf5-config >/dev/null; then
rm -r CMakeFiles
rm CMakeLists.txt
mv CMakeLists_kde5.txt CMakeLists.txt
cmake -DCMAKE_INSTALL_PREFIX="`kf5-config --prefix`" $PWD
make
make install DESTDIR=%{buildroot}

echo %(kf5-config --path services | awk -NF ":" '{print $NF}')/megasync-plugin.desktop >> %{EXTRA_FILES}
echo %(kf5-config --path lib | awk -NF ":" '{print $1}')/qt5/plugins/megasyncplugin.so >> %{EXTRA_FILES}

fi

echo tras install:
find %{buildroot}

echo cat %{EXTRA_FILES}
cat %{EXTRA_FILES}

%clean
echo cleaning
%{?buildroot:%__rm -rf "%{buildroot}"}

%files -f %{EXTRA_FILES}
%defattr(-,root,root)



%changelog
