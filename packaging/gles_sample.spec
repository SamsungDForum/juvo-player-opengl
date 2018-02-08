Name:      gles_sample
Version:   0.0.1
Release:   1
License:   To be filled
Summary:   Hello GL Application
Group:     Application
Source0:   %{name}-%{version}.tar.gz
BuildRequires: cmake

BuildRequires:  pkgconfig(gles20)
BuildRequires:  pkgconfig(egl)

%description
OpenGLES Sample

%prep
%setup -q

cp %{name}.manifest ./../../SOURCES/


%build
cmake -DCMAKE_INSTALL_PREFIX=%{_prefix}

make %{?jobs:-j%jobs}

%clean

%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}%{_includedir}

%files
%manifest %{name}.manifest
%defattr(-, root, root, -)
%{_libdir}/libgles_sample.so