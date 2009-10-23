%{!?ruby_sitearchdir: %define ruby_sitearchdir %(/usr/bin/ruby -rrbconfig -e "puts Config::CONFIG['sitearchdir']")}

Summary: A powerful pattern matching system for parsing and processing text
Name: grok
Version: 20091023
Release: 1
Group: System Environment/Utilities
License: BSD
Source0: http://semicomplete.googlecode.com/files/%{name}-%{version}.tar.gz
URL: http://www.semicomplete.com/projects/grok/
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Requires: libevent
Requires: tokyocabinet >= 1.4.9
BuildRequires: ruby-devel libevent-devel gperf tokyocabinet-devel

%description
A powerful pattern matching system for parsing and processing text data such
as log files.

%package ruby
Group: System Environment/Utilities
Summary: Ruby library to access grok functions
Requires: ruby >= 1.8.5
Requires: grok = %{version}-%{release}

%description ruby
A ruby interface to the grok pattern matching library.

%package devel
Group: Development Tools
Summary: Grok development headers

%description devel
Headers required for grok development.

%prep
%setup -q

%build
make

# for the ruby extention
export LD_LIBRARY_PATH=$PWD
cd ruby
ruby extconf.rb
make
cd ..

%install
%{__rm} -rf %{buildroot}
%{__mkdir_p} %{buildroot}%{_bindir}
%{__mkdir_p} %{buildroot}%{_libdir}
%{__mkdir_p} %{buildroot}%{_includedir}
%{__mkdir_p} %{buildroot}%{ruby_sitearchdir}
install -c grok %{buildroot}/%{_bindir}
install -c libgrok.so %{buildroot}/%{_libdir}
for header in grok.h grokre.h grok_pattern.h grok_capture.h grok_capture_xdr.h grok_match.h grok_logging.h; do
 install -c $header %{buildroot}/%{_includedir}
done
cd ruby
make install DESTDIR=%{buildroot}
cd ..

%clean
%{__rm} -rf %{buildroot}

%files
%defattr(-,root,root)
%{_bindir}/grok
%{_libdir}/libgrok.so

%files ruby
%{ruby_sitearchdir}/Grok.so

%files devel
%{_includedir}

%post
/sbin/ldconfig

%changelog
* Mon Oct 19 2009 Pete Fritchman <petef@databits.net> 20090928-1
- Initial packaging.
