Name:      gles
Version:   0.0.1
Release:   1
License:   To be filled
Summary:   Hello GL Application
Group:     Application
Source0:   %{name}-%{version}.tar.gz
BuildRequires: cmake

BuildRequires:  pkgconfig(gles20)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(freetype2)


%if %{?static_check:1}%{!?static_check:0}
BuildRequires:  clang
%endif

%description
OpenGLES Sample

%prep
%setup -q

cp %{name}.manifest ./../../SOURCES/

%build
cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} \
%if %{?static_check:1}%{!?static_check:0}
-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
%endif
%if %{!?nodebug:1}%{?nodebug:0}
-DDEBUG=1
%endif

%if %{?static_check:1}%{!?static_check:0}
find src/ -name *.cpp | xargs clang-check -analyze
%endif

make %{?jobs:-j%jobs}

%clean

%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}%{_includedir}

%files
%manifest %{name}.manifest
%defattr(-, root, root, -)
%{_libdir}/libgles.so
