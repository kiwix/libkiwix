kiwix_sources = [
  'book.cpp',
  'bookmark.cpp',
  'library.cpp',
  'manager.cpp',
  'libxml_dumper.cpp',
  'opds_dumper.cpp',
  'downloader.cpp',
  'reader.cpp',
  'entry.cpp',
  'searcher.cpp',
  'subprocess.cpp',
  'aria2.cpp',
  'tools/base64.cpp',
  'tools/pathTools.cpp',
  'tools/regexTools.cpp',
  'tools/stringTools.cpp',
  'tools/networkTools.cpp',
  'tools/otherTools.cpp',
]
kiwix_sources += lib_resources

if host_machine.system() == 'windows'
  kiwix_sources += 'subprocess_windows.cpp'
else
  kiwix_sources += 'subprocess_unix.cpp'
endif

if get_option('android')
  subdir('android')
  install_dir = 'kiwix-lib/jniLibs/' + meson.get_cross_property('android_abi')
else
  install_dir = get_option('libdir')
endif

config_h = configure_file(output : 'kiwix_config.h',
               configuration : conf,
               input : 'config.h.in')
install_headers(config_h, subdir:'kiwix')

kiwixlib = library('kiwix',
                   kiwix_sources,
                   include_directories : inc,
                   dependencies : all_deps,
                   version: meson.project_version(),
                   install: true,
                   install_dir: install_dir,
                   install_rpath: '$ORIGIN')
