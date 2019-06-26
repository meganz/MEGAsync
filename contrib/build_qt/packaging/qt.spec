Name:		qt-mega
Version:	1.0
Release:	1%{?dist}
Summary:	TODO
License:	TODO
Group:		Applications/Others
Url:		https://mega.nz
Source0:	qt.tar.gz
Source1:	qt-rpmlintrc
Vendor:		TODO
Packager:	TODO

BuildRequires: autoconf, automake, libtool, gcc-c++, unzip, rsync
#if 0%{?suse_version}
BuildRequires: zlib-devel
#%endif

BuildRequires: cmake

%if 0%{?fedora_version}>=21 || 0%{?suse_version} > 1310
BuildRequires: extra-cmake-modules
%endif

%description
TODO

%prep

%setup -q

%define debug_package %{nil}

%build
bash -x ./build_minimum.sh


%install
for i in `find target/lib -name "lib*a" -type f`; do %{__install} -D $i $RPM_BUILD_ROOT%{_libdir}/${i/target\/lib\//}; done
for i in `find target/include -type f | grep -v openssl `; do %{__install} -D -m 444 $i $RPM_BUILD_ROOT%{_includedir}/qt-mega/${i/target\/include\//}; done

(cd $RPM_BUILD_ROOT; for i in `find ./%{_libdir} -type f`; do echo $i | sed "s#^./#/#g"; done) >> %{_topdir}/ExtraFiles.list
echo /%{_includedir}/qt-mega  >> %{_topdir}/ExtraFiles.list

%post

%clean
%{?buildroot:%__rm -rf "%{buildroot}"}

%files -f %{_topdir}/ExtraFiles.list
%defattr(-,root,root)

%changelog
