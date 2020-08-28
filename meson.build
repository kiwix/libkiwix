project('kiwix-lib', 'cpp',
  version : '9.4.0', # Also change this in android-kiwix-lib-publisher/kiwixLibAndroid/build.gradle
  license : 'GPLv3+',
  default_options : ['c_std=c11', 'cpp_std=c++11', 'werror=true'])

compiler = meson.get_compiler('cpp')

wrapper = get_option('wrapper')

static_deps = wrapper.contains('android') or wrapper.contains('java') or get_option('default_library') == 'static'
if wrapper.contains('android')
  extra_libs = ['-llog']
else
  extra_libs = []
endif

if wrapper.contains('java')
  add_languages('java')
endif

# See https://github.com/kiwix/kiwix-lib/issues/371
if ['arm', 'mips', 'm68k', 'ppc', 'sh4'].contains(target_machine.cpu_family())
  extra_libs += '-latomic'
endif

thread_dep = dependency('threads')
libicu_dep = dependency('icu-i18n', static:static_deps)
libzim_dep = dependency('libzim', version : '>=6.1.8', static:static_deps)
pugixml_dep = dependency('pugixml', static:static_deps)
libcurl_dep = dependency('libcurl', static:static_deps)
microhttpd_dep = dependency('libmicrohttpd', static:static_deps)

if compiler.has_header('mustache.hpp')
  extra_include = []
elif compiler.has_header('mustache.hpp', args: '-I/usr/include/kainjow')
  extra_include = ['/usr/include/kainjow']
else
  error('Cannot found header mustache.hpp')
endif

extra_cflags = ''
if target_machine.system() == 'windows' and static_deps
  add_project_arguments('-DCURL_STATICLIB', language : 'cpp')
  extra_cflags += '-DCURL_STATICLIB'
endif

all_deps = [thread_dep, libicu_dep, libzim_dep, pugixml_dep, libcurl_dep, microhttpd_dep]

inc = include_directories('include', extra_include)

conf = configuration_data()
conf.set('VERSION', '"@0@"'.format(meson.project_version()))

if build_machine.system() == 'windows'
     extra_link_args = ['-lshlwapi', '-lwinmm']
else
     extra_link_args = []
endif

subdir('include')
subdir('scripts')
subdir('static')
subdir('src')
subdir('test')

pkg_requires = ['libzim', 'icu-i18n', 'pugixml', 'libcurl']

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

