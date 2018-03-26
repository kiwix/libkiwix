kiwix_sources = [
  'library.cpp',
  'manager.cpp',
  'opds_dumper.cpp',
  'reader.cpp',
  'searcher.cpp',
  'common/base64.cpp',
  'common/pathTools.cpp',
  'common/regexTools.cpp',
  'common/stringTools.cpp',
  'common/networkTools.cpp',
  'common/otherTools.cpp',
  'xapian/htmlparse.cc',
  'xapian/myhtmlparse.cc'
]
kiwix_sources += lib_resources

if xapian_dep.found()
  kiwix_sources += ['xapianSearcher.cpp']
endif

if get_option('android')
  subdir('android')
  install_dir = 'kiwix-lib/jniLibs/' + meson.get_cross_property('android_abi')
else
  install_dir = get_option('libdir')
endif


if has_ctpp2_dep
  kiwix_sources += ['ctpp2/CTPP2VMStringLoader.cpp']
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
