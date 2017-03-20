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

AutoReq: 0

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
%if 0%{?fedora_version} >= 22
# Fedora 21 cmake is too old for KF5
BuildRequires: kf5-kdelibs4support-devel extra-cmake-modules
%endif
%endif

%if 0%{?centos_version}
BuildRequires: qt-devel kdelibs-devel
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

# Create a temporary file containing the list of files
EXTRA_FILES=%{buildroot}/ExtraFiles.list
touch %{EXTRA_FILES}

cmake -DCMAKE_INSTALL_PREFIX="`kde4-config --prefix`" $PWD
make
make install DESTDIR=%{buildroot}

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

%if 0%{?centos_version}
#fix conflict with existing /usr/lib64 (pointing to /usr/lib)
if [ -d %{buildroot}/usr/lib ]; then
    rsync -av %{buildroot}/usr/lib/ %{buildroot}/usr/lib64/
    rm -rf %{buildroot}/usr/lib
fi
%endif

%clean
echo cleaning
%{?buildroot:%__rm -rf "%{buildroot}"}

%files -f %{EXTRA_FILES}
%defattr(-,root,root)



%changelog
