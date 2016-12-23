project('kiwixlib', 'cpp',
  version : '1.0.0',
  license : 'GPL')

compiler = meson.get_compiler('cpp')

thread_dep = dependency('threads')
libicu_dep = dependency('icu-i18n')
libzim_dep = dependency('zimlib')
pugixml_dep = dependency('pugixml')
aria2_dep = find_program('aria2c')


ctpp2_prefix_install = get_option('ctpp2-install-prefix')
if get_option('default_library') == 'static'
    libname = 'ctpp2-st'
else
    libname = 'ctpp2'
endif
if ctpp2_prefix_install == ''
  if not compiler.has_header('ctpp2/CTPP2Logger.hpp')
    error('ctpp2/CTPP2Logger.hppnot found')
  endif
  ctpp2_lib = compiler.find_library(libname)
  ctpp2_dep = declare_dependency(dependencies:[ctpp2_lib])
else
  ctpp2_include_path = ctpp2_prefix_install + '/include'
  ctpp2_include_args = ['-I'+ctpp2_include_path]
  if not compiler.has_header('ctpp2/CTPP2Logger.hpp', args:ctpp2_include_args)
    error('ctpp2/CTPP2Logger.hppnot found')
  endif
  ctpp2_include_path = include_directories(ctpp2_include_path, is_system:true)
  ctpp2_lib_path = ctpp2_prefix_install+'/lib'
  ctpp2_lib = compiler.find_library(libname, dirs:ctpp2_lib_path)
  ctpp2_dep = declare_dependency(include_directories:ctpp2_include_path, dependencies:[ctpp2_lib])
endif

pkg_requires = ['zimlib', 'icu-i18n', 'pugixml']
xapian_dep = dependency('xapian-core', required:false)
if xapian_dep.found()
    pkg_requires += ['xapian-core']
    message('xapian_dep found')
else
    message('xapian_dep not found')
endif

all_deps = [thread_dep, libicu_dep, libzim_dep, ctpp2_dep, xapian_dep, pugixml_dep]

inc = include_directories('include')

subdir('include')
subdir('scripts')
subdir('static')
subdir('src')

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries : kiwixlib,
                 version : '1.0',
                 name : 'libkiwix',
                 filebase : 'kiwix',
                 description : 'A library that contains a lot of things used by used by other kiwix programs',
                 requires: pkg_requires)

