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

BuildRequires: qt-devel, openssl-devel, sqlite-devel, zlib-devel, autoconf, automake, libtool, gcc-c++
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

%if 0%{?centos_version}
%define flag_cryptopp -q
%endif

%if 0%{?rhel_version}
%define flag_cryptopp -q
%endif




export DESKTOP_DESTDIR=$RPM_BUILD_ROOT/usr
./configure %{flag_cryptopp}
# Fedora uses system Crypto++ header files
%if 0%{?fedora}
rm -fr MEGASync/mega/bindings/qt/3rdparty/include/cryptopp
%endif

%if 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version}
qmake-qt4 DESTDIR=%{buildroot}%{_bindir} THE_RPM_BUILD_ROOT=%{buildroot}
lrelease-qt4  MEGASync/MEGASync.pro
%else
qmake DESTDIR=%{buildroot}%{_bindir} THE_RPM_BUILD_ROOT=%{buildroot}
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

# RHEL 7
%if 0%{?rhel_version} == 700
YUM_FILE="/etc/yum.repos.d/megasync.repo"
cat > "$YUM_FILE" << DATA
[MEGAsync]
name=MEGAsync
baseurl=http://mega.nz/linux/MEGAsync/RHEL_7/
gpgkey=https://mega.nz/linux/MEGAsync/RHEL_7/repodata/repomd.xml.key
gpgcheck=1
enabled=1
DATA
%endif

# CentOS 7
%if 0%{?centos_version} == 700
YUM_FILE="/etc/yum.repos.d/megasync.repo"
cat > "$YUM_FILE" << DATA
[MEGAsync]
name=MEGAsync
baseurl=http://mega.nz/linux/MEGAsync/CentOS_7/
gpgkey=https://mega.nz/linux/MEGAsync/CentOS_7/repodata/repomd.xml.key
gpgcheck=1
enabled=1
DATA
%endif

# Fedora 23
%if 0%{?fedora_version} == 23
YUM_FILE="/etc/yum.repos.d/megasync.repo"
cat > "$YUM_FILE" << DATA
[MEGAsync]
name=MEGAsync
baseurl=http://mega.nz/linux/MEGAsync/Fedora_23/
gpgkey=https://mega.nz/linux/MEGAsync/Fedora_23/repodata/repomd.xml.key
gpgcheck=1
enabled=1
DATA
%endif

# Fedora 22
%if 0%{?fedora_version} == 22
YUM_FILE="/etc/yum.repos.d/megasync.repo"
cat > "$YUM_FILE" << DATA
[MEGAsync]
name=MEGAsync
baseurl=http://mega.nz/linux/MEGAsync/Fedora_22/
gpgkey=https://mega.nz/linux/MEGAsync/Fedora_22/repodata/repomd.xml.key
gpgcheck=1
enabled=1
DATA
%endif

# Fedora 21
%if 0%{?fedora_version} == 21
YUM_FILE="/etc/yum.repos.d/megasync.repo"
cat > "$YUM_FILE" << DATA
[MEGAsync]
name=MEGAsync
baseurl=http://mega.nz/linux/MEGAsync/Fedora_21/
gpgkey=https://mega.nz/linux/MEGAsync/Fedora_21/repodata/repomd.xml.key
gpgcheck=1
enabled=1
DATA
%endif

# Fedora 20
%if 0%{?fedora_version} == 20
YUM_FILE="/etc/yum.repos.d/megasync.repo"
cat > "$YUM_FILE" << DATA
[MEGAsync]
name=MEGAsync
baseurl=http://mega.nz/linux/MEGAsync/Fedora_20/
gpgkey=https://mega.nz/linux/MEGAsync/Fedora_20/repodata/repomd.xml.key
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
gpgkey=https://mega.nz/linux/MEGAsync/Fedora_19/repodata/repomd.xml.key
gpgcheck=1
enabled=1
DATA
%endif

# openSUSE Leap 42.1
%if 0%{?suse_version} == 1315
if [ -d "/etc/zypp/repos.d/" ]; then
ZYPP_FILE="/etc/zypp/repos.d/megasync.repo"
cat > "$ZYPP_FILE" << DATA
[MEGAsync]
name=MEGAsync
type=rpm-md
baseurl=http://mega.nz/linux/MEGAsync/openSUSE_Leap_42.1/
gpgcheck=1
autorefresh=1
gpgkey=http://mega.nz/linux/MEGAsync/openSUSE_Leap_42.1/repodata/repomd.xml.key
enabled=1
DATA
fi
%endif

# openSUSE Tumbleweed (rolling release)
%if 0%{?suse_version} > 1320
if [ -d "/etc/zypp/repos.d/" ]; then
ZYPP_FILE="/etc/zypp/repos.d/megasync.repo"
cat > "$ZYPP_FILE" << DATA
[MEGAsync]
name=MEGAsync
type=rpm-md
baseurl=http://mega.nz/linux/MEGAsync/openSUSE_Tumbleweed/
gpgcheck=1
autorefresh=1
gpgkey=http://mega.nz/linux/MEGAsync/openSUSE_Tumbleweed/repodata/repomd.xml.key
enabled=1
DATA
fi
%endif

# openSUSE 13.2
%if 0%{?suse_version} == 1320
if [ -d "/etc/zypp/repos.d/" ]; then
ZYPP_FILE="/etc/zypp/repos.d/megasync.repo"
cat > "$ZYPP_FILE" << DATA
[MEGAsync]
name=MEGAsync
type=rpm-md
baseurl=http://mega.nz/linux/MEGAsync/openSUSE_13.2/
gpgcheck=1
autorefresh=1
gpgkey=http://mega.nz/linux/MEGAsync/openSUSE_13.2/repodata/repomd.xml.key
enabled=1
DATA
fi
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
#%{_datadir}/icons/hicolor/*/apps/mega.png
%{_datadir}/icons/hicolor/*/*/mega.png
%{_datadir}/doc/megasync
%{_datadir}/doc/megasync/*

%changelog

