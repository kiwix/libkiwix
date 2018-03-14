project('kiwix-lib', 'cpp',
  version : '1.1.1',
  license : 'GPL',
  default_options : ['c_std=c11', 'cpp_std=c++11', 'werror=true'])

compiler = meson.get_compiler('cpp')
find_library_in_compiler = meson.version().version_compare('>=0.31.0')

static_deps = get_option('android') or get_option('default_library') == 'static'

thread_dep = dependency('threads')
libicu_dep = dependency('icu-i18n', static:static_deps)
libzim_dep = dependency('libzim', version : '>=3.2.0', static:static_deps)
pugixml_dep = dependency('pugixml', static:static_deps)

ctpp2_include_path = ''
has_ctpp2_dep = false
ctpp2_prefix_install = get_option('ctpp2-install-prefix')
ctpp2_link_args = []
if ctpp2_prefix_install == ''
  if compiler.has_header('ctpp2/CTPP2Logger.hpp')
    if find_library_in_compiler
      ctpp2_lib = compiler.find_library('ctpp2')
    else
      ctpp2_lib = find_library('ctpp2')
    endif
    ctpp2_link_args = ['-lctpp2']
    if meson.is_cross_build() and host_machine.system() == 'windows'
      if find_library_in_compiler
        iconv_lib = compiler.find_library('iconv', required:false)
      else
        iconv_lib = find_library('iconv', required:false)
      endif
      if iconv_lib.found()
        ctpp2_link_args += ['-liconv']
      endif
    endif
    has_ctpp2_dep = true
    ctpp2_dep = declare_dependency(link_args:ctpp2_link_args)
  else
    message('ctpp2/CTPP2Logger.hpp not found. Compiling without CTPP2 support')
  endif
else
  if not find_library_in_compiler
    error('For custom ctpp2_prefix_install you need a meson version >=0.31.0')
  endif
  ctpp2_include_path = ctpp2_prefix_install + '/include'
  ctpp2_include_args = ['-I'+ctpp2_include_path]
  if compiler.has_header('ctpp2/CTPP2Logger.hpp', args:ctpp2_include_args)
    ctpp2_include_dir = include_directories(ctpp2_include_path, is_system:true)
    ctpp2_lib_path = join_paths(ctpp2_prefix_install, get_option('libdir'))
    message(ctpp2_lib_path)
    ctpp2_lib = compiler.find_library('ctpp2', dirs:ctpp2_lib_path, required:false)
    if not ctpp2_lib.found()
      ctpp2_lib_path = join_paths(ctpp2_prefix_install, 'lib')
      message(ctpp2_lib_path)
      ctpp2_lib = compiler.find_library('ctpp2', dirs:ctpp2_lib_path)
    endif
    ctpp2_link_args = ['-L'+ctpp2_lib_path, '-lctpp2']
    if meson.is_cross_build() and host_machine.system() == 'windows'
      iconv_lib = compiler.find_library('iconv', required:false)
      if iconv_lib.found()
        ctpp2_link_args += ['-liconv']
      endif
    endif
    has_ctpp2_dep = true
    ctpp2_dep = declare_dependency(include_directories:ctpp2_include_dir, link_args:ctpp2_link_args)
  else
    message('ctpp2/CTPP2Logger.hpp not found. Compiling without CTPP2 support')
  endif
endif

xapian_dep = dependency('xapian-core', required:false, static:static_deps)

all_deps = [thread_dep, libicu_dep, libzim_dep, xapian_dep, pugixml_dep]
if has_ctpp2_dep
  all_deps += [ctpp2_dep]
endif

inc = include_directories('include')

conf = configuration_data()
conf.set('VERSION', '"@0@"'.format(meson.project_version()))
conf.set('ENABLE_CTPP2', has_ctpp2_dep)

subdir('include')
subdir('scripts')
subdir('static')
subdir('src')

pkg_requires = ['libzim', 'icu-i18n', 'pugixml']
if xapian_dep.found()
    pkg_requires += ['xapian-core']
endif

extra_libs = []
extra_cflags = ''
if has_ctpp2_dep
  extra_libs += ctpp2_link_args
  if ctpp2_include_path != ''
    extra_cflags = '-I'+ctpp2_include_path
  endif
endif

pkg_conf = configuration_data()
pkg_conf.set('prefix', get_option('prefix'))
pkg_conf.set('requires', ' '.join(pkg_requires))
pkg_conf.set('extra_libs', ' '.join(extra_libs))
pkg_conf.set('extra_cflags', extra_cflags)
pkg_conf.set('version', meson.project_version())
configure_file(output : 'kiwix.pc',
               configuration : pkg_conf,
               input : 'kiwix.pc.in',
               install_dir: get_option('libdir')+'/pkgconfig'
              )

