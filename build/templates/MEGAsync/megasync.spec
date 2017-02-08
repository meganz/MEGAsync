Name:		megasync
Version:	MEGASYNC_VERSION
Release:	1%{?dist}
Summary:	Easy automated syncing between your computers and your MEGA cloud drive
License:	Freeware
Group:		Applications/Others
Url:		https://mega.nz
Source0:	megasync_%{version}.tar.gz
Vendor:		MEGA Limited
Packager:	MEGA Linux Team <linux@mega.co.nz>

BuildRequires: openssl-devel, sqlite-devel, zlib-devel, autoconf, automake, libtool, gcc-c++
BuildRequires: hicolor-icon-theme, unzip, wget

%if 0%{?suse_version}
BuildRequires: libcares-devel
BuildRequires: update-desktop-files
 
%if 0%{?sle_version} == 120200 || 0%{?suse_version} > 1320
BuildRequires: libqt5-qtbase-devel >= 5.6, libqt5-linguist
Requires: libQt5Core5 >= 5.6
%else
BuildRequires: libqt4-devel, qt-devel
%endif

# disabling post-build-checks that ocassionally prevent opensuse rpms from being generated
# plus it speeds up building process
BuildRequires: -post-build-checks

%if 0%{?suse_version} <= 1320
BuildRequires: libcryptopp-devel
%endif

%endif



%if 0%{?fedora}
%if 0%{?fedora_version} >= 23
BuildRequires: c-ares-devel, cryptopp-devel
BuildRequires: desktop-file-utils
BuildRequires: qt5-qtbase-devel qt5-qttools-devel
Requires: qt5-qtbase >= 5.6
BuildRequires: terminus-fonts, fontpackages-filesystem
%else
BuildRequires: c-ares-devel, cryptopp-devel
BuildRequires: desktop-file-utils
BuildRequires: qt, qt-x11, qt-devel
BuildRequires: terminus-fonts, fontpackages-filesystem
%endif
%endif

%if 0%{?centos_version} || 0%{?scientificlinux_version}
BuildRequires: c-ares-devel,
BuildRequires: desktop-file-utils
BuildRequires: qt, qt-x11, qt-devel
%endif

%if 0%{?rhel_version}
BuildRequires: desktop-file-utils
BuildRequires: qt, qt-x11, qt-devel
%endif


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

%define flag_cryptopp %{nil}

%if 0%{?centos_version} || 0%{?scientificlinux_version}
%define flag_cryptopp -q
%endif

%if 0%{?rhel_version}
%define flag_cryptopp -q
%endif

%define flag_disablezlib %{nil}
%if 0%{?fedora_version} == 23
%define flag_disablezlib -z
%endif

%if 0%{?suse_version} > 1320
%define flag_cryptopp -q
%endif

export DESKTOP_DESTDIR=$RPM_BUILD_ROOT/usr

./configure %{flag_cryptopp} -g %{flag_disablezlib}

# Fedora uses system Crypto++ header files
%if 0%{?fedora}
rm -fr MEGASync/mega/bindings/qt/3rdparty/include/cryptopp
%endif

%if 0%{?fedora} || 0%{?sle_version} == 120200 || 0%{?suse_version} > 1320

%if 0%{?fedora_version} >= 23 || 0%{?sle_version} == 120200 || 0%{?suse_version} > 1320
qmake-qt5 DESTDIR=%{buildroot}%{_bindir} THE_RPM_BUILD_ROOT=%{buildroot}
lrelease-qt5  MEGASync/MEGASync.pro
%else
qmake-qt4 DESTDIR=%{buildroot}%{_bindir} THE_RPM_BUILD_ROOT=%{buildroot}
lrelease-qt4  MEGASync/MEGASync.pro
%endif
%else

%if 0%{?rhel_version} || 0%{?centos_version} || 0%{?scientificlinux_version}
qmake-qt4 DESTDIR=%{buildroot}%{_bindir} THE_RPM_BUILD_ROOT=%{buildroot}
lrelease-qt4  MEGASync/MEGASync.pro
%else
qmake DESTDIR=%{buildroot}%{_bindir} THE_RPM_BUILD_ROOT=%{buildroot}
lrelease MEGASync/MEGASync.pro
%endif

%endif

make

%install
make install DESTDIR=%{buildroot}%{_bindir}
#mkdir -p %{buildroot}%{_datadir}/applications
#%{__install} MEGAsync/platform/linux/data/megasync.desktop -D %{buildroot}%{_datadir}/applications

%if 0%{?suse_version}
%suse_update_desktop_file -n -i %{name} Network System
%else
desktop-file-install \
    --add-category="Network" \
    --dir %{buildroot}%{_datadir}/applications \
%{buildroot}%{_datadir}/applications/%{name}.desktop
%endif

%post
%if 0%{?suse_version} >= 1140
%desktop_database_post
%icon_theme_cache_post
%else
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :
%endif

%if 0%{?rhel_version} == 700
# RHEL 7
YUM_FILE="/etc/yum.repos.d/megasync.repo"
cat > "$YUM_FILE" << DATA
[MEGAsync]
name=MEGAsync
baseurl=https://mega.nz/linux/MEGAsync/RHEL_7/
gpgkey=https://mega.nz/linux/MEGAsync/RHEL_7/repodata/repomd.xml.key
gpgcheck=1
enabled=1
DATA
%endif

%if 0%{?scientificlinux_version} == 700
# Scientific Linux 7
YUM_FILE="/etc/yum.repos.d/megasync.repo"
cat > "$YUM_FILE" << DATA
[MEGAsync]
name=MEGAsync
baseurl=https://mega.nz/linux/MEGAsync/ScientificLinux_7/
gpgkey=https://mega.nz/linux/MEGAsync/ScientificLinux_7/repodata/repomd.xml.key
gpgcheck=1
enabled=1
DATA
%endif

%if 0%{?sc} == 700
# CentOS 7
YUM_FILE="/etc/yum.repos.d/megasync.repo"
cat > "$YUM_FILE" << DATA
[MEGAsync]
name=MEGAsync
baseurl=https://mega.nz/linux/MEGAsync/CentOS_7/
gpgkey=https://mega.nz/linux/MEGAsync/CentOS_7/repodata/repomd.xml.key
gpgcheck=1
enabled=1
DATA
%endif

%if 0%{?fedora_version} == 25
# Fedora 25
YUM_FILE="/etc/yum.repos.d/megasync.repo"
cat > "$YUM_FILE" << DATA
[MEGAsync]
name=MEGAsync
baseurl=https://mega.nz/linux/MEGAsync/Fedora_25/
gpgkey=https://mega.nz/linux/MEGAsync/Fedora_25/repodata/repomd.xml.key
gpgcheck=1
enabled=1
DATA
%endif

%if 0%{?fedora_version} == 24
# Fedora 24
YUM_FILE="/etc/yum.repos.d/megasync.repo"
cat > "$YUM_FILE" << DATA
[MEGAsync]
name=MEGAsync
baseurl=https://mega.nz/linux/MEGAsync/Fedora_24/
gpgkey=https://mega.nz/linux/MEGAsync/Fedora_24/repodata/repomd.xml.key
gpgcheck=1
enabled=1
DATA
%endif

%if 0%{?fedora_version} == 23
# Fedora 23
YUM_FILE="/etc/yum.repos.d/megasync.repo"
cat > "$YUM_FILE" << DATA
[MEGAsync]
name=MEGAsync
baseurl=https://mega.nz/linux/MEGAsync/Fedora_23/
gpgkey=https://mega.nz/linux/MEGAsync/Fedora_23/repodata/repomd.xml.key
gpgcheck=1
enabled=1
DATA
%endif

%if 0%{?fedora_version} == 22
# Fedora 22
YUM_FILE="/etc/yum.repos.d/megasync.repo"
cat > "$YUM_FILE" << DATA
[MEGAsync]
name=MEGAsync
baseurl=https://mega.nz/linux/MEGAsync/Fedora_22/
gpgkey=https://mega.nz/linux/MEGAsync/Fedora_22/repodata/repomd.xml.key
gpgcheck=1
enabled=1
DATA
%endif

%if 0%{?fedora_version} == 21
# Fedora 21
YUM_FILE="/etc/yum.repos.d/megasync.repo"
cat > "$YUM_FILE" << DATA
[MEGAsync]
name=MEGAsync
baseurl=https://mega.nz/linux/MEGAsync/Fedora_21/
gpgkey=https://mega.nz/linux/MEGAsync/Fedora_21/repodata/repomd.xml.key
gpgcheck=1
enabled=1
DATA
%endif

%if 0%{?fedora_version} == 20
# Fedora 20
YUM_FILE="/etc/yum.repos.d/megasync.repo"
cat > "$YUM_FILE" << DATA
[MEGAsync]
name=MEGAsync
baseurl=https://mega.nz/linux/MEGAsync/Fedora_20/
gpgkey=https://mega.nz/linux/MEGAsync/Fedora_20/repodata/repomd.xml.key
gpgcheck=1
enabled=1
DATA
%endif

%if 0%{?fedora_version} == 19
# Fedora 19
YUM_FILE="/etc/yum.repos.d/megasync.repo"
cat > "$YUM_FILE" << DATA
[MEGAsync]
name=MEGAsync
baseurl=https://mega.nz/linux/MEGAsync/Fedora_19
gpgkey=https://mega.nz/linux/MEGAsync/Fedora_19/repodata/repomd.xml.key
gpgcheck=1
enabled=1
DATA
%endif

%if 0%{?sle_version} == 120200
# openSUSE Leap 42.2
if [ -d "/etc/zypp/repos.d/" ]; then
ZYPP_FILE="/etc/zypp/repos.d/megasync.repo"
cat > "$ZYPP_FILE" << DATA
[MEGAsync]
name=MEGAsync
type=rpm-md
baseurl=https://mega.nz/linux/MEGAsync/openSUSE_Leap_42.2/
gpgcheck=1
autorefresh=1
gpgkey=https://mega.nz/linux/MEGAsync/openSUSE_Leap_42.2/repodata/repomd.xml.key
enabled=1
DATA
fi
%endif


%if 0%{?sle_version} == 120100
# openSUSE Leap 42.1
if [ -d "/etc/zypp/repos.d/" ]; then
ZYPP_FILE="/etc/zypp/repos.d/megasync.repo"
cat > "$ZYPP_FILE" << DATA
[MEGAsync]
name=MEGAsync
type=rpm-md
baseurl=https://mega.nz/linux/MEGAsync/openSUSE_Leap_42.1/
gpgcheck=1
autorefresh=1
gpgkey=https://mega.nz/linux/MEGAsync/openSUSE_Leap_42.1/repodata/repomd.xml.key
enabled=1
DATA
fi
%endif
 

%if 0%{?suse_version} > 1320
# openSUSE Tumbleweed (rolling release)
if [ -d "/etc/zypp/repos.d/" ]; then
ZYPP_FILE="/etc/zypp/repos.d/megasync.repo"
cat > "$ZYPP_FILE" << DATA
[MEGAsync]
name=MEGAsync
type=rpm-md
baseurl=https://mega.nz/linux/MEGAsync/openSUSE_Tumbleweed/
gpgcheck=1
autorefresh=1
gpgkey=https://mega.nz/linux/MEGAsync/openSUSE_Tumbleweed/repodata/repomd.xml.key
enabled=1
DATA
fi
%endif

%if 0%{?suse_version} == 1320
# openSUSE 13.2
if [ -d "/etc/zypp/repos.d/" ]; then
ZYPP_FILE="/etc/zypp/repos.d/megasync.repo"
cat > "$ZYPP_FILE" << DATA
[MEGAsync]
name=MEGAsync
type=rpm-md
baseurl=https://mega.nz/linux/MEGAsync/openSUSE_13.2/
gpgcheck=1
autorefresh=1
gpgkey=https://mega.nz/linux/MEGAsync/openSUSE_13.2/repodata/repomd.xml.key
enabled=1
DATA
fi
%endif

%if 0%{?suse_version} == 1310
# openSUSE 13.1
if [ -d "/etc/zypp/repos.d/" ]; then
ZYPP_FILE="/etc/zypp/repos.d/megasync.repo"
cat > "$ZYPP_FILE" << DATA
[MEGAsync]
name=MEGAsync
type=rpm-md
baseurl=https://mega.nz/linux/MEGAsync/openSUSE_13.1/
gpgcheck=1
autorefresh=1
gpgkey=https://mega.nz/linux/MEGAsync/openSUSE_13.1/repodata/repomd.xml.key
enabled=1
DATA
fi
%endif

%if 0%{?suse_version} == 1230
# openSUSE 12.3
if [ -d "/etc/zypp/repos.d/" ]; then
ZYPP_FILE="/etc/zypp/repos.d/megasync.repo"
cat > "$ZYPP_FILE" << DATA
[MEGAsync]
name=MEGAsync
type=rpm-md
baseurl=https://mega.nz/linux/MEGAsync/openSUSE_12.3/
gpgcheck=1
autorefresh=1
gpgkey=https://mega.nz/linux/MEGAsync/openSUSE_12.3/repodata/repomd.xml.key
enabled=1
DATA
fi
%endif

%if 0%{?suse_version} == 1220
# openSUSE 12.2
if [ -d "/etc/zypp/repos.d/" ]; then
ZYPP_FILE="/etc/zypp/repos.d/megasync.repo"
cat > "$ZYPP_FILE" << DATA
[MEGAsync]
name=MEGAsync
type=rpm-md
baseurl=https://mega.nz/linux/MEGAsync/openSUSE_12.2/
gpgcheck=1
autorefresh=1
gpgkey=https://mega.nz/linux/MEGAsync/openSUSE_12.2/repodata/repomd.xml.key
enabled=1
DATA
fi
%endif


### include public signing key #####
# Install new key if it's not present
# Notice, for openSuse, postinst is checked (and therefore executed) when creating the rpm
# we need to ensure no command results in fail (returns !=0)
rpm -q gpg-pubkey-7f068e5d-563dc081 > /dev/null 2>&1 || KEY_NOT_FOUND=1

if [ ! -z "$KEY_NOT_FOUND" ]; then
KEYFILE=$(mktemp /tmp/megasync.XXXXXX || :)
if [ -n "$KEYFILE" ]; then
    cat > "$KEYFILE" <<KEY || :
-----BEGIN PGP PUBLIC KEY BLOCK-----
Version: GnuPG v2

mI0EVj3AgQEEAO2XyJgpvE5HDRVsggcrMhf5+KpQepl7m7OyrPSgxLi72Wuy5GWp
hO64BX1UzmdUirIEOc13YxdeuhwJ3YP0wnKUyUrdWA0r2HjOz555vN6ldrPlSCBI
RxKBWRMQaR4cwNKQ8V4xV9tVdPGgrQ9L/4H3fM9fYqCwEMKBxxLZsF3PABEBAAG0
IE1lZ2FMaW1pdGVkIDxzdXBwb3J0QG1lZ2EuY28ubno+iL8EEwECACkFAlY9wIEC
GwMFCRLMAwAHCwkIBwMCAQYVCAIJCgsEFgIDAQIeAQIXgAAKCRADw606fwaOXfOS
A/998rh6f0wsrHmX2LTw2qmrWzwPj4m+vp0m3w5swPZw1x4qSNsmNsIXX8J0ZcSE
qymOwHZ0B9imBS3iz+U496NSfPNWABbIBnUAu8zq0IR28Q9pUcLe5MWFsw9NO+w2
5dByoN9JKeUftZt1x76NHn5wmxB9fv7WVlCnZJ+T16+nh7iNBFY9wIEBBADHpopM
oXNkrGZLI6Ok1F5N7+bSgiyZwkvBMAqCkPawUgwJztFKGf8F/sSbydsKRC2aQcuJ
eOj0ZPUtJ80+o3w8MsHRtZDSxDIxqqiHeupoDRI3Be9S544vI5/UmiiygTuhmNTT
NWgStoZz7hEK4IsELAG1EFodIMtBSkptDL92HwARAQABiKUEGAECAA8FAlY9wIEC
GwwFCRLMAwAACgkQA8OtOn8Gjl3HlAQAoOckF5JBJWekmlX+k2RYwtgfszk31Gq+
Jjiho4rUEW8c1EUPvK8v1jRGwjYED3ihJ6510eblYFPl+6k91OWlScnxuVVAmSn4
35RW3vR+nYUvf3s8rctbw97gJJZAA7p5oAowTux3oHotKSYhhxKcz15goMXzSb5G
/h7fJRhMnw4=
=fp/e
-----END PGP PUBLIC KEY BLOCK-----
KEY

mv /var/lib/rpm/.rpm.lock /var/lib/rpm/.rpm.lock_moved || : #to allow rpm import within postinst
%if 0%{?suse_version}
#Key import would hang and fail due to lock in /var/lib/rpm/Packages. We create a copy
mv /var/lib/rpm/Packages{,_moved}
cp /var/lib/rpm/Packages{_moved,}
%endif
rpm --import "$KEYFILE" 2>&1 || FAILED_IMPORT=1
%if 0%{?suse_version}
rm /var/lib/rpm/Packages_moved  #remove the old one
%endif
mv /var/lib/rpm/.rpm.lock_moved /var/lib/rpm/.rpm.lock || : #take it back

rm $KEYFILE || :
fi
fi

### END of POSTINST


%postun
%if 0%{?suse_version} >= 1140
%desktop_database_postun
%icon_theme_cache_postun
%else
if [ $1 -eq 0 ] ; then
    /bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null
    /usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
fi
%endif
# kill running MEGAsync instance
killall megasync 2> /dev/null || true


%if 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version} || 0%{?scientificlinux_version}
%posttrans
/usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
%endif

%clean
%{?buildroot:%__rm -rf "%{buildroot}"}

%files
%defattr(-,root,root)
%{_bindir}/%{name}
%{_datadir}/applications/megasync.desktop
#%{_datadir}/icons/hicolor/*/apps/mega.png
%{_datadir}/icons/hicolor/*/*/mega.png
%{_datadir}/doc/megasync
%{_datadir}/doc/megasync/*

%changelog
