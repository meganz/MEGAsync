Name:       nautilus-megasync
Version:    EXT_VERSION
Release:	%(cat MEGA_BUILD_ID || echo "1").1
Summary:	Easy automated syncing between your computers and your MEGA cloud drive
License:	Freeware
Group:		Applications/Others
Url:		https://mega.nz
Source0:	nautilus-megasync_%{version}.tar.gz
Vendor:		MEGA Limited
Packager:	MEGA Linux Team <linux@mega.co.nz>

BuildRequires:  nautilus-devel

%if 0%{?suse_version} || 0%{?sle_version}
BuildRequires: libqt5-qtbase-devel
%else
BuildRequires: qt5-qtbase-devel
%endif

%if 0%{?rhel_version}
BuildRequires: redhat-logos
%endif
%if 0%{?fedora_version}
BuildRequires: fedora-logos
%endif
%if 0%{?scientificlinux_version}
BuildRequires: sl-logos, gcc-c++
%endif

Requires:       nautilus, megasync >= 3.5

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

NAUTILUS_VERSION=`(rpm -qi nautilus-extensions | grep ^Version) | awk -F':' '{print $2}' | awk -F "." '{FS=".";print $1*10000+$2*100+$3}'`

if [ 0$NAUTILUS_VERSION -gt 31503 ]; then
    for i in data/emblems/64x64/*smaller.png; do mv $i ${i/-smaller/}; done
    echo "NEWER NAUTILUS REQUIRES SMALLER OVERLAY ICONS"
else
    rm data/emblems/64x64/*smaller.png
    echo "OLDER NAUTILUS DOES NOT REQUIRE SMALLER OVERLAY ICONS"
fi

qmake-qt5 || qmake

%if 0%{?fedora_version} >= 27 || 0%{?centos_version} == 800
#tweak to have debug symbols to stripe: for some reason they seem gone by default in Fedora 27,
#   causing "gdb-add-index: No index was created for ..." which lead to error "Empty %files file ....debugsourcefiles.list"
sed "s# gcc# gcc -g#g" -i Makefile
%endif
make

%install
make install

# clean up
rm -fr $RPM_BUILD_ROOT/usr/share/icons/hicolor/icon-theme.cache || true

%post
%if 0%{?suse_version} >= 1140
%icon_theme_cache_post
%else
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :
%endif
# restart Nautilus
UPDATENOTIFIERDIR=/var/lib/update-notifier/user.d
echo "Please restart all running instances of Nautilus."

if [ -d $UPDATENOTIFIERDIR ] ; then
        cat > $UPDATENOTIFIERDIR/megasync-install-notify <<DATA
Name: Nautilus Restart Required
Priority: High
Terminal: False
Command: nautilus -q
DontShowAfterReboot: True
ButtonText: _Restart Nautilus
DisplayIf: pgrep -x nautilus -U \$(id -u) > /dev/null
OnlyAdminUsers: False
Description:
 MEGAsync requires Nautilus to be restarted to function properly.
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
# restart Nautilus
UPDATENOTIFIERDIR=/var/lib/update-notifier/user.d
echo "Please restart all running instances of Nautilus."

if [ -d $UPDATENOTIFIERDIR ] ; then
        cat > $UPDATENOTIFIERDIR/megasync-install-notify <<DATA
Name: Nautilus Restart Required
Priority: High
Terminal: False
Command: nautilus -q
DontShowAfterReboot: True
ButtonText: _Restart Nautilus
DisplayIf: pgrep -x nautilus -U \$(id -u) > /dev/null
OnlyAdminUsers: False
Description:
 MEGAsync requires Nautilus to be restarted to function properly.
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
%{_libdir}/nautilus/extensions-*/libMEGAShellExtNautilus.so*
%{_datadir}/icons/hicolor/*/*/mega-*.icon
%{_datadir}/icons/hicolor/*/*/mega-*.png

%changelog

