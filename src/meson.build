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
  'server.cpp',
  'searcher.cpp',
  'search_renderer.cpp',
  'subprocess.cpp',
  'aria2.cpp',
  'tools/base64.cpp',
  'tools/pathTools.cpp',
  'tools/regexTools.cpp',
  'tools/stringTools.cpp',
  'tools/networkTools.cpp',
  'tools/otherTools.cpp',
  'tools/archiveTools.cpp',
  'kiwixserve.cpp',
  'name_mapper.cpp',
  'server/byte_range.cpp',
  'server/etag.cpp',
  'server/request_context.cpp',
  'server/response.cpp',
  'server/internalServer.cpp',
  'server/internalServer_catalog_v2.cpp',
  'opds_catalog.cpp'
]
kiwix_sources += lib_resources

if host_machine.system() == 'windows'
  kiwix_sources += 'subprocess_windows.cpp'
else
  kiwix_sources += 'subprocess_unix.cpp'
endif

if wrapper.contains('android')
  install_dir = 'kiwix-lib/jniLibs/' + meson.get_cross_property('android_abi')
else
  install_dir = get_option('libdir')
endif

if wrapper.contains('android') or wrapper.contains('java')
  subdir('wrapper/java')
endif

config_h = configure_file(output : 'kiwix_config.h',
               configuration : conf,
               input : 'config.h.in')
install_headers(config_h, subdir:'kiwix')

kiwixlib = library('kiwix',
                   kiwix_sources,
                   include_directories : inc,
                   dependencies : all_deps,
                   link_args: extra_libs,
                   version: meson.project_version(),
                   install: true,
                   install_dir: install_dir)
