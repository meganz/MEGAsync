Name:		megasync-debug
Version:	MEGASYNC_VERSION
Release:	%(cat MEGA_BUILD_ID || echo "1").1
Summary:	Get more control over your data
License:	Freeware
Group:		Applications/Others
Url:		https://mega.nz
Source0:	megasync-debug_%{version}.tar.gz
Vendor:		MEGA Limited
Packager:	MEGA Linux Team <linux@mega.co.nz>

BuildRequires: qt-devel, libqt4-devel, openssl-devel, sqlite-devel, zlib-devel, autoconf, automake, libtool, gcc-c++, libraw-devel
BuildRequires: hicolor-icon-theme, unzip, wget

%if 0%{?suse_version}
BuildRequires: libcares-devel, libcryptopp-devel
BuildRequires: update-desktop-files
BuildRequires: libqt4, libqt4-x11
%endif

%if 0%{?fedora}
BuildRequires: c-ares-devel, cryptopp-devel
BuildRequires: desktop-file-utils
BuildRequires: qt, qt-x11
BuildRequires: terminus-fonts, fontpackages-filesystem
%endif

%if 0%{?centos_version}
BuildRequires: c-ares-devel,
BuildRequires: desktop-file-utils
BuildRequires: qt, qt-x11
%endif

%if 0%{?rhel_version}
BuildRequires: desktop-file-utils
BuildRequires: qt, qt-x11
%endif

%description
- Sync your entire MEGA Cloud or selected folders with your computer so your MEGA stays up to date with the changes you make to your data on your computer and vice versa.
- Back up your computer with MEGA to automatically copy data to MEGA in real time and eliminate the risk of accidental data loss.
- Easily add, sort, search for, prioritise, pause, and cancel your uploads and downloads using our transfer manager.

%prep
%setup -q

%build
./configure
export DESKTOP_DESTDIR=$RPM_BUILD_ROOT/usr
%if 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version}
qmake-qt4 "CONFIG += FULLREQUIREMENTS" "CONFIG += debug" DESTDIR=%{buildroot}%{_bindir}
qmake-qt4 "CONFIG += FULLREQUIREMENTS" "CONFIG += debug" DESTDIR=%{buildroot}%{_bindir} MEGASync/MEGASync.pro
lrelease-qt4  MEGASync/MEGASync.pro
%else
qmake "CONFIG += FULLREQUIREMENTS" "CONFIG += debug" DESTDIR=%{buildroot}%{_bindir}
qmake "CONFIG += FULLREQUIREMENTS" "CONFIG += debug" DESTDIR=%{buildroot}%{_bindir} MEGASync/MEGASync.pro
lrelease MEGASync/MEGASync.pro
%endif

make

%install
make install DESTDIR=%{buildroot}%{_bindir}
#mkdir -p %{buildroot}%{_datadir}/applications
#%{__install} MEGAsync/platform/linux/data/megasync.desktop -D %{buildroot}%{_datadir}/applications

%if 0%{?suse_version}
%suse_update_desktop_file -n -i megasync Network System
%else
desktop-file-install \
    --add-category="Network" \
    --dir %{buildroot}%{_datadir}/applications \
%{buildroot}%{_datadir}/applications/megasync.desktop
%endif

%post
%if 0%{?suse_version} >= 1140
%desktop_database_post
%icon_theme_cache_post
%else
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :
%endif

# install rpm key for Fedora
%if 0%{?fedora_version}
# Install new key if it's not present
rpm -q gpg-pubkey-23acd201-53da6e72 > /dev/null 2>&1
if [ "$?" -ne "0" ]; then
KEYFILE=$(mktemp /tmp/megasync.XXXXXX)
if [ -n "$KEYFILE" ]; then
    cat > "$KEYFILE" <<KEY
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
fi

rpm --import "$KEYFILE"

fi

%endif

# Fedora 20
%if 0%{?fedora_version} == 20
YUM_FILE="/etc/yum.repos.d/megasync.repo"
cat > "$YUM_FILE" << DATA
[MEGAsync]
name=MEGAsync
baseurl=http://mega.nz/linux/MEGAsync/Fedora_20/
gpgcheck=1
enabled=1
DATA
%endif

# Fedora 19
%if 0%{?fedora_version} == 19
YUM_FILE="/etc/yum.repos.d/megasync.repo"
cat > "$YUM_FILE" << DATA
[MEGAsync]
name=MEGAsync
baseurl=http://mega.nz/linux/MEGAsync/Fedora_19
gpgcheck=1
enabled=1
DATA
%endif

# openSUSE 13.1
%if 0%{?suse_version} == 1310
if [ -d "/etc/zypp/repos.d/" ]; then
ZYPP_FILE="/etc/zypp/repos.d/megasync.repo"
cat > "$ZYPP_FILE" << DATA
[MEGAsync]
name=MEGAsync
type=rpm-md
baseurl=http://mega.nz/linux/MEGAsync/openSUSE_13.1/
gpgcheck=1
autorefresh=1
gpgkey=http://mega.nz/linux/MEGAsync/openSUSE_13.1/repodata/repomd.xml.key
enabled=1
DATA
fi
%endif

# openSUSE 12.3
%if 0%{?suse_version} == 1230
if [ -d "/etc/zypp/repos.d/" ]; then
ZYPP_FILE="/etc/zypp/repos.d/megasync.repo"
cat > "$ZYPP_FILE" << DATA
[MEGAsync]
name=MEGAsync
type=rpm-md
baseurl=http://mega.nz/linux/MEGAsync/openSUSE_12.3/
gpgcheck=1
autorefresh=1
gpgkey=http://mega.nz/linux/MEGAsync/openSUSE_12.3/repodata/repomd.xml.key
enabled=1
DATA
fi
%endif

# openSUSE 12.2
%if 0%{?suse_version} == 1220
if [ -d "/etc/zypp/repos.d/" ]; then
ZYPP_FILE="/etc/zypp/repos.d/megasync.repo"
cat > "$ZYPP_FILE" << DATA
[MEGAsync]
name=MEGAsync
type=rpm-md
baseurl=http://mega.nz/linux/MEGAsync/openSUSE_12.2/
gpgcheck=1
autorefresh=1
gpgkey=http://mega.nz/linux/MEGAsync/openSUSE_12.2/repodata/repomd.xml.key
enabled=1
DATA
fi
%endif


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
# remove repo files
YUM_FILE="/etc/yum.repos.d/megasync.repo"
rm -f $YUM_FILE 2> /dev/null || true
ZYPP_FILE="/etc/zypp/repos.d/megasync.repo"
rm -f $ZYPP_FILE 2> /dev/null || true
# kill running MEGAsync instance
killall megasync 2> /dev/null || true


%if 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version}
%posttrans
/usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
%endif

%clean
%{?buildroot:%__rm -rf "%{buildroot}"}

%files
%defattr(-,root,root)
%{_bindir}/%{name}
%{_datadir}/applications/megasync.desktop
%{_datadir}/icons/hicolor/*/apps/mega.png
%{_datadir}/icons/hicolor/*/*/mega.png

%changelog

