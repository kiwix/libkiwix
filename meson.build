project('libkiwix', 'cpp',
  version : '14.0.0',
  license : 'GPLv3+',
  default_options : ['c_std=c11', 'cpp_std=c++17', 'werror=true'])

compiler = meson.get_compiler('cpp')
static_deps = get_option('static-linkage') or get_option('default_library') == 'static'
extra_libs = []

# Atomics as compiled by GCC or clang can lead to external references to
# functions depending on the type size and the platform. LLVM provides them in
# 'libcompiler_rt', which clang normally automatically links in, while GNU
# provides them in 'libatomic', which GCC *does not* link in automatically (but
# this is probably going to change, see
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=81358). Regardless of the setup
# of the compiler driver itself (GCC or clang), we can thus assume that if some
# atomic references can't be resolved, then 'libatomic' is missing.
atomics_program = '''
#include <atomic>
#include <cstdint>

using namespace std;

int main() {
  volatile atomic_bool a_b(true);
  volatile atomic_ullong a_ull(-1);
  // Next two lines are to cover atomic<socket_t> from 'httplib.h'.
  volatile atomic<uint32_t> a_u32(-1);
  volatile atomic<uint64_t> a_u64(-1);

  return atomic_load(&a_b) == false && atomic_load(&a_ull) == 0 &&
    atomic_load(&a_u32) == 0 && atomic_load(&a_u64) == 0;
}
'''
if not compiler.links(atomics_program,
                      name: 'compiler driver readily supports atomics')
  libatomic = compiler.find_library('atomic')
  compiler.links(atomics_program, name: 'atomics work with libatomic',
                 dependencies: libatomic, required: true)
  extra_libs += ['-latomic']
endif

# C++ std::thread is implemented using pthread on Linux by GCC, and on FreeBSD
# for both GCC and LLVM.
if (host_machine.system() == 'linux' and compiler.get_id() == 'gcc') or \
   host_machine.system() == 'freebsd'
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
