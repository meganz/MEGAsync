Name:		megasync-debug
Version:	MEGASYNC_VERSION
Release:	1%{?dist}
Summary:	Easy automated syncing between your computers and your MEGA cloud drive
License:	Freeware
Group:		Applications/Others
Url:		https://mega.co.nz
Source0:	megasync-debug_%{version}.tar.gz
Vendor:		MEGA Limited
Packager:	MEGA Linux Team <linux@mega.co.nz>

BuildRequires: qt-devel, libqt4-devel, openssl-devel, sqlite-devel, zlib-devel, autoconf, automake, libtool, gcc-c++, gdb
BuildRequires: hicolor-icon-theme

%if 0%{?suse_version}
BuildRequires: libcares-devel, libcryptopp-devel
BuildRequires: update-desktop-files
BuildRequires: libqt4, libqt4-x11
%endif

%if 0%{?rhel_version} || 0%{?centos_version} || 0%{?fedora}
BuildRequires: c-ares-devel, cryptopp-devel
BuildRequires: desktop-file-utils
BuildRequires: qt, qt-x11
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
%configure
export DESKTOP_DESTDIR=$RPM_BUILD_ROOT/usr

%if 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version}
qmake-qt4 "CONFIG += debug" DESTDIR=%{buildroot}%{_bindir}
lrelease-qt4  MEGASync/MEGASync.pro
%else
qmake "CONFIG += debug" DESTDIR=%{buildroot}%{_bindir}
lrelease MEGASync/MEGASync.pro
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

# install rpm key for Fedora
%if 0%{?fedora_version}
# Install new key if it's not present
rpm -q gpg-pubkey-23acd201-53da6e72 > /dev/null 2>&1
if [ "$?" -ne "0" ]; then
KEYFILE=$(mktemp /tmp/megasync.XXXXXX)
if [ -n "$KEYFILE" ]; then
    cat > "$KEYFILE" <<KEY
-----BEGIN PGP PUBLIC KEY BLOCK-----
Version: GnuPG v2.0.22 (GNU/Linux)

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
URYSLAw5e33Za+vKMGF/hifuJ1V0GYkUig==
=FzpN
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
baseurl=http://mega.co.nz/linux/MEGAsync/Fedora_20/
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
baseurl=http://mega.co.nz/linux/MEGAsync/Fedora_19
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
baseurl=http://mega.co.nz/linux/MEGAsync/openSUSE_13.1/
gpgcheck=1
autorefresh=1
gpgkey=http://mega.co.nz/linux/MEGAsync/openSUSE_13.1/repodata/repomd.xml.key
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
baseurl=http://mega.co.nz/linux/MEGAsync/openSUSE_12.3/
gpgcheck=1
autorefresh=1
gpgkey=http://mega.co.nz/linux/MEGAsync/openSUSE_12.3/repodata/repomd.xml.key
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
baseurl=http://mega.co.nz/linux/MEGAsync/openSUSE_12.2/
gpgcheck=1
autorefresh=1
gpgkey=http://mega.co.nz/linux/MEGAsync/openSUSE_12.2/repodata/repomd.xml.key
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

