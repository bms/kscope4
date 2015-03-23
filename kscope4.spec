%bcond_with			debug

%define name			kscope4
%define release			1
%define version			1.7.1

Summary:	KDE frontend to Cscope
Name:           %name
Version:        %version
Release:        %release
License: 	BSD
Group: 		Development/Other
URL: 		http://sourceforge.net/projects/kscope4

Requires:	cscope ctags

BuildRequires:	kdebase4-devel
BuildRequires:	kdelibs4-devel
BuildRequires:  libgraphviz-devel
BuildRequires:  flex bison
BuildRoot: 	%{_tmppath}/%{name}-%{version}-%{release}-buildroot

Source: 	http://ovh.dl.sourceforge.net/sourceforge/kscope/%name-%version.tar.xz

%description
KScope is a source-editing environment for KDE based on Cscope.

%prep
%setup -q

%build
%cmake_kde4 %{!?with_debug:-DCMAKE_BUILD_TYPE=Release}

%make clean all

%install
rm -rf %buildroot

make -C build DESTDIR=%buildroot install

# Icons
install -D -m 644 %{buildroot}%{_iconsdir}/hicolor/32x32/apps/kscope.png %{buildroot}%{_liconsdir}/kscope.png
install -D -m 644 %{buildroot}%{_iconsdir}/hicolor/32x32/apps/kscope.png %{buildroot}%{_iconsdir}/kscope.png
install -D -m 644 %{buildroot}%{_iconsdir}/hicolor/16x16/apps/kscope.png %{buildroot}%{_miconsdir}/kscope.png

mkdir -p %{buildroot}%{_datadir}/applications/kde4
desktop-file-install	--vendor=''						\
			--dir %{buildroot}%{_kde_applicationsdir}		\
			--add-category='TextEditor'				\
			%{buildroot}%{_kde_applicationsdir}/kscope.desktop

%find_lang kscope

%clean
rm -rf %buildroot

#---------------------------------------------------------------------------

%files -f kscope.lang
%defattr(-,root,root)
%{_kde_bindir}/kscope

%dir %{_kde_datadir}/apps/kscope/
%{_kde_appsdir}/kscope/kscopeui.rc
%{_kde_appsdir}/kscope/pics/*.png
%{_kde_appsdir}/kscope/kscope_config
%{_kde_applicationsdir}/kscope.desktop

%{_kde_iconsdir}/*/*/apps/*.png
%{_kde_iconsdir}/kscope.png
%{_miconsdir}/kscope.png
%{_liconsdir}/kscope.png

%doc %{_kde_docdir}/HTML/*/kscope/*

%changelog
* Mon Mar 23 2015 fe6fbq <fe6fbq@users.sourceforge.net> 1.7.1-1
- Fix MakeDlg pb if the command line is too long due to KHistoryComboBox
  Lineedit behaviour
- Fix bug when creating or opening a project
- Allow project creation in an existing directory
- Fix pb in cscope env. var. setting with a permanent project
- Cleanup source files (shrink blank lines; remove #if 0 ... #endif blocks)
- Fix icon name
- Add dialog to setup cscope env. var. when rebuilding database
