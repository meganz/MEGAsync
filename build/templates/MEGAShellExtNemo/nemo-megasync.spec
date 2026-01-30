Name:       nemo-megasync
Version:    EXT_VERSION
Release:	%(cat MEGA_BUILD_ID || echo "1").1
Summary:	MEGA Desktop App plugin for Nemo
License:	Freeware
Group:		Applications/Others
Url:		https://mega.io/desktop
Source0:	nemo-megasync_%{version}.tar.gz
Vendor:		MEGA Limited
Packager:	MEGA Linux Team <linux@mega.io>

BuildRequires:   nemo-devel, cmake, gcc-c++

%if 0%{?rhel} || 0%{?rhel_version} || 0%{?centos} || 0%{?centos_version} || 0%{?almalinux}
%global debug_package %{nil}
BuildRequires: redhat-rpm-config
BuildRequires: gcc-c++
%endif
%if 0%{?suse_version} || 0%{?sle_version}
BuildRequires: libnemo-extension1
%else
BuildRequires: nemo-extensions
%endif
%if 0%{?fedora_version} 
BuildRequires: fedora-logos
%global debug_package %{nil}
%endif
%if 0%{?scientificlinux_version} 
BuildRequires: sl-logos
%endif

Requires:       nemo, megasync >= 5.3.0

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
# clean up
rm -fr %{buildroot}/%{_datadir}/icons/hicolor/icon-theme.cache || true

%post
%if 0%{?suse_version} >= 1140
%icon_theme_cache_post
%else
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :
%endif
# restart Nemo
UPDATENOTIFIERDIR=/var/lib/update-notifier/user.d
echo "Please restart all running instances of Nemo."

if [ -d $UPDATENOTIFIERDIR ] ; then
        cat > $UPDATENOTIFIERDIR/megasync-install-notify <<DATA
Name: Nemo Restart Required
Priority: High
Terminal: False
Command: nemo -q
DontShowAfterReboot: True
ButtonText: _Restart Nemo
DisplayIf: pgrep -x nemo -U \$(id -u) > /dev/null
OnlyAdminUsers: False
Description:
 MEGAsync requires Nemo to be restarted to function properly.
DATA
fi

%postun
%if 0%{?suse_version} >= 1140
%icon_theme_cache_postun
%else
if [ $1 -eq 0 ] ; then
    /bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null
    /usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
fi
%endif
# restart Nemo
UPDATENOTIFIERDIR=/var/lib/update-notifier/user.d
echo "Please restart all running instances of Nemo."

if [ -d $UPDATENOTIFIERDIR ] ; then
        cat > $UPDATENOTIFIERDIR/megasync-install-notify <<DATA
Name: Nemo Restart Required
Priority: High
Terminal: False
Command: nemo -q
DontShowAfterReboot: True
ButtonText: _Restart Nemo
DisplayIf: pgrep -x nemo -U \$(id -u) > /dev/null
OnlyAdminUsers: False
Description:
 MEGAsync requires Nemo to be restarted to function properly.
DATA
fi


%if 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version} || 0%{?scientificlinux_version}
%posttrans
/usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
%endif

%clean
%{?buildroot:%__rm -rf "%{buildroot}"}

%files
%defattr(-,root,root)
%{_libdir}/nemo/extensions-3.0/libMEGAShellExtNemo.so*
%{_datadir}/icons/hicolor/*/*/mega-*.icon
%{_datadir}/icons/hicolor/*/*/mega-*.png

%changelog

