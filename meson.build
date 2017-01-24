project('kiwixlib', 'cpp',
  version : '1.0.0',
  license : 'GPL')

compiler = meson.get_compiler('cpp')

thread_dep = dependency('threads')
libicu_dep = dependency('icu-i18n')
libzim_dep = dependency('libzim')
pugixml_dep = dependency('pugixml')


ctpp2_prefix_install = get_option('ctpp2-install-prefix')
if get_option('default_library') == 'static'
    libname = 'ctpp2-st'
else
    libname = 'ctpp2'
endif

find_library_in_compiler = meson.version().version_compare('>=0.31.0')
if ctpp2_prefix_install == ''
  if not compiler.has_header('ctpp2/CTPP2Logger.hpp')
    error('ctpp2/CTPP2Logger.hppnot found')
  endif
  if find_library_in_compiler
    ctpp2_lib = compiler.find_library(libname)
  else
    ctpp2_lib = find_library(libname)
  endif
  link_args = ['-l'+libname]
  if meson.is_cross_build()
    if host_machine.system() == 'windows'
      link_args += ['-liconv']
    endif
  endif
  ctpp2_dep = declare_dependency(link_args:link_args)
else
  if not find_library_in_compiler
    error('For custom ctpp2_prefix_install you need a meson version >=0.31.0')
  endif
  ctpp2_include_path = ctpp2_prefix_install + '/include'
  ctpp2_include_args = ['-I'+ctpp2_include_path]
  if not compiler.has_header('ctpp2/CTPP2Logger.hpp', args:ctpp2_include_args)
    error('ctpp2/CTPP2Logger.hpp not found')
  endif
  ctpp2_include_path = include_directories(ctpp2_include_path, is_system:true)
  ctpp2_lib_path = ctpp2_prefix_install+'/lib'
  ctpp2_lib = compiler.find_library(libname, dirs:ctpp2_lib_path)
  link_args = ['-L'+ctpp2_lib_path, '-l'+libname]
  if meson.is_cross_build()
    if host_machine.system() == 'windows'
      link_args += ['-liconv']
    endif
  endif
  ctpp2_dep = declare_dependency(include_directories:ctpp2_include_path, link_args:link_args)
endif

pkg_requires = ['libzim', 'icu-i18n', 'pugixml']
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

