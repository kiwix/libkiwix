project('libkiwix', 'cpp',
  version : '14.0.0',
  license : 'GPLv3+',
  default_options : ['c_std=c11', 'cpp_std=c++17', 'werror=true'])

compiler = meson.get_compiler('cpp')

static_deps = get_option('static-linkage') or get_option('default_library') == 'static'

# See https://github.com/kiwix/libkiwix/issues/371
if ['arm', 'mips', 'm68k', 'ppc', 'sh4'].contains(host_machine.cpu_family())
  extra_libs = ['-latomic']
else
  extra_libs = []
endif

if (compiler.get_id() == 'gcc' and build_machine.system() == 'linux') or host_machine.system() == 'freebsd'
  # C++ std::thread is implemented using pthread on linux by gcc
  thread_dep = dependency('threads')
else
  thread_dep = dependency('', required:false)
endif
libicu_dep = dependency('icu-i18n', static:static_deps)
pugixml_dep = dependency('pugixml', static:static_deps)
libcurl_dep = dependency('libcurl', static:static_deps)
microhttpd_dep = dependency('libmicrohttpd', static:static_deps)
zlib_dep = dependency('zlib', static:static_deps)
xapian_dep = dependency('xapian-core', static:static_deps)

if compiler.has_header('mustache.hpp')
  extra_include = []
elif compiler.has_header('mustache.hpp', args: '-I/usr/include/kainjow')
  extra_include = ['/usr/include/kainjow']
else
  error('Cannot found header mustache.hpp')
endif

libzim_dep = dependency('libzim', version:['>=9.0.0', '<10.0.0'], static:static_deps)

if not compiler.has_header_symbol('zim/zim.h', 'LIBZIM_WITH_XAPIAN', dependencies: libzim_dep)
  error('Libzim seems to be compiled without Xapian. Xapian support is mandatory.')
endif


extra_cflags = ''
if host_machine.system() == 'windows' and static_deps
  add_project_arguments('-DCURL_STATICLIB', language : 'cpp')
  extra_cflags += '-DCURL_STATICLIB'
endif

if host_machine.system() == 'windows'
  add_project_arguments('-DNOMINMAX', language: 'cpp')
  extra_libs += ['-liphlpapi']
endif

if build_machine.system() == 'windows'
  extra_libs += ['-lshlwapi', '-lwinmm']
endif


all_deps = [thread_dep, libicu_dep, libzim_dep, pugixml_dep, libcurl_dep, microhttpd_dep, zlib_dep, xapian_dep]

inc = include_directories('include', extra_include)

conf = configuration_data()
conf.set('LIBKIWIX_VERSION', '"@0@"'.format(meson.project_version()))

subdir('include')
subdir('scripts')
subdir('static')
subdir('src')
subdir('test')
if get_option('doc')
  subdir('docs')
endif

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries : [libkiwix] + extra_libs,
                 version : meson.project_version(),
                 name : 'libkiwix',
                 filebase : 'libkiwix',
                 description : 'A library that contains useful primitives that Kiwix readers have in common',
                 extra_cflags: extra_cflags)
