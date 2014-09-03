Name:		megasync
Version:	MEGASYNC_VERSION
Release:	1%{?dist}
Summary:	Easy automated syncing between your computers and your MEGA cloud drive
License:	Freeware
Group:		Applications/Others
Url:		https://mega.co.nz
Source0:	megasync_%{version}.tar.gz

BuildRequires: qt-devel, libqt4-devel, openssl-devel, sqlite-devel, zlib-devel, autoconf, automake, libtool, gcc-c++
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
qmake-qt4 DESTDIR=%{buildroot}%{_bindir}
lrelease-qt4  MEGASync/MEGASync.pro
%else
qmake DESTDIR=%{buildroot}%{_bindir}
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

# Fedora 20
%if 0%{?fedora_version} == 20
YUM_FILE="/etc/yum.repos.d/megasync.repo"
cat > "$YUM_FILE" << DATA
[MEGAsync]
name=MEGAsync
type=rpm-md
baseurl=http://mega.co.nz/linux/MEGAsync/Fedora_20/
gpgcheck=1
gpgkey=http://mega.co.nz/linux/MEGAsync/Fedora_20/repodata/repomd.xml.key
enabled=1
DATA
%endif

# Fedora 19
%if 0%{?fedora_version} == 19
YUM_FILE="/etc/yum.repos.d/megasync.repo"
cat > "$YUM_FILE" << DATA
[MEGAsync]
name=MEGAsync
type=rpm-md
baseurl=http://mega.co.nz/linux/MEGAsync/Fedora_19
gpgcheck=1
gpgkey=http://mega.co.nz/linux/MEGAsync/Fedora_19/repodata/repomd.xml.key
enabled=1
DATA
%endif

# openSUSE 13.1
%if 0%{?suse_version} == 1310
ZYPP_FILE="/etc/zypp/repos.d/megasync.repo"
cat > "$ZYPP_FILE" << DATA
[MEGAsync]
name=MEGAsync
type=rpm-md
baseurl=http://mega.co.nz/linux/MEGAsync/openSUSE_13.1/
gpgcheck=1
gpgkey=http://mega.co.nz/linux/MEGAsync/openSUSE_13.1/repodata/repomd.xml.key
enabled=1
DATA
%endif

# openSUSE 12.3
%if 0%{?suse_version} == 1230
ZYPP_FILE="/etc/zypp/repos.d/megasync.repo"
cat > "$ZYPP_FILE" << DATA
[MEGAsync]
name=MEGAsync
type=rpm-md
baseurl=http://mega.co.nz/linux/MEGAsync/openSUSE_12.3/
gpgcheck=1
gpgkey=http://mega.co.nz/linux/MEGAsync/openSUSE_12.3/repodata/repomd.xml.key
enabled=1
DATA
%endif

# openSUSE 12.2
%if 0%{?suse_version} == 1220
ZYPP_FILE="/etc/zypp/repos.d/megasync.repo"
cat > "$ZYPP_FILE" << DATA
[MEGAsync]
name=MEGAsync
type=rpm-md
baseurl=http://mega.co.nz/linux/MEGAsync/openSUSE_12.2/
gpgcheck=1
gpgkey=http://mega.co.nz/linux/MEGAsync/openSUSE_12.2/repodata/repomd.xml.key
enabled=1
DATA
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

