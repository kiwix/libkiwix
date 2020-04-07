project('kiwix-lib', 'cpp',
  version : '9.0.1', # Also change this in android-kiwix-lib-publisher/kiwixLibAndroid/build.gradle
  license : 'GPL',
  default_options : ['c_std=c11', 'cpp_std=c++11', 'werror=true'])

compiler = meson.get_compiler('cpp')

wrapper = get_option('wrapper')

static_deps = 'android' in wrapper or 'java' in wrapper or get_option('default_library') == 'static'
if 'android' in wrapper
  extra_libs = ['-llog']
else
  extra_libs = []
endif

if 'java' in wrapper
  add_languages('java')
endif

thread_dep = dependency('threads')
libicu_dep = dependency('icu-i18n', static:static_deps)
libzim_dep = dependency('libzim', version : '>=5.0.0', static:static_deps)
pugixml_dep = dependency('pugixml', static:static_deps)
libcurl_dep = dependency('libcurl', static:static_deps)
microhttpd_dep = dependency('libmicrohttpd', static:static_deps)

if not compiler.has_header('mustache.hpp')
  error('Cannot found header mustache.hpp')
endif

extra_cflags = ''
if target_machine.system() == 'windows' and static_deps
  add_project_arguments('-DCURL_STATICLIB', language : 'cpp')
  extra_cflags += '-DCURL_STATICLIB'
endif

all_deps = [thread_dep, libicu_dep, libzim_dep, pugixml_dep, libcurl_dep, microhttpd_dep]

inc = include_directories('include')

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

pkgconfig = import('pkgconfig')
pkgconfig.generate(
  kiwixlib,
  name: 'kiwix',
  description: 'A library that contains a lot of things used by used by other kiwix programs',
  url: 'https://github.com/kiwix/kiwix-lib',
  requires: pkg_requires + extra_libs,
  extra_cflags: extra_cflags
)

