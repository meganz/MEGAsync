Name:		qt-mega
Version:	1.0
Release:	%(cat MEGA_BUILD_ID || echo "1").1
Summary:	TODO
License:	TODO
Group:		Applications/Others
Url:		https://mega.nz
Source0:	qt-mega_%{version}.tar.gz
Vendor:		TODO
Packager:	TODO

BuildRequires: autoconf, automake, libtool, gcc-c++, unzip, rsync, wayland-devel, libxkbcommon-devel, libxkbcommon-x11-devel
BuildRequires: fontconfig-devel, libxkbcommon-devel, libxkbcommon-x11-devel
BuildRequires: zlib-devel
BuildRequires: cmake

%if 0%{?is_opensuse} || 0%{?suse_version}
BuildRequires: libopenssl-devel, Mesa-devel
%else
BuildRequires: openssl-devel, mesa-libGL-devel
%endif

%if 0%{?fedora} || 0%{?centos_version}>=800
BuildRequires: python2
%else
BuildRequires: python
%endif

%if 0%{?fedora_version}>=21 || 0%{?suse_version} > 1310
BuildRequires: extra-cmake-modules
%endif

%description
TODO

%prep

%setup -q

%define debug_package %{nil}

%build

if [ `gcc -dumpversion | cut -d'.' -f1` -lt 5 ];then
sed -i 's@make -j@sed -i "s#auto tp#const QWindowSystemInterface::TouchPoint \\\&tp#g" qtwayland/src/client/qwaylandinputdevice.cpp\nmake -j@' ./build_minimum.sh
fi
bash -x ./build_minimum.sh
echo "[Paths]" > target/opt/mega/bin/qt.conf
echo "Prefix=/opt/mega" >> target/opt/mega/bin/qt.conf

%install

for i in `find target/ -maxdepth 1 -mindepth 1`; do
cp -rP ${i} $RPM_BUILD_ROOT/$(dirname ${i/target\///})
done

%if 0%{?fedora}>=30
#
# Changing python to python2
#
echo "Changing python to python2 in $RPM_BUILD_ROOT/opt/mega/mkspecs/features/uikit/devices.py"
sed -i '1 s#\#!/usr/bin/python$#\#!/usr/bin/python2#' $RPM_BUILD_ROOT/opt/mega/mkspecs/features/uikit/devices.py
%endif

%post

%clean
%{?buildroot:%__rm -rf "%{buildroot}"}

%files -f %{_topdir}/ExtraFiles.list
/opt
%defattr(-,root,root)

%changelog
