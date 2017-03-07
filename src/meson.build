kiwix_sources = [
  'library.cpp',
  'manager.cpp',
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
  if not get_option('android')
    kiwix_sources += ['xapianIndexer.cpp']
  endif
endif

if not get_option('android')
  kiwix_sources += ['indexer.cpp']
else
  subdir('android')
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
                   install : true)
