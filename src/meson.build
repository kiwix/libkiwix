kiwix_sources = [
  'library.cpp',
  'manager.cpp',
  'reader.cpp',
  'searcher.cpp',
  'indexer.cpp',
  'common/base64.cpp',
  'common/pathTools.cpp',
  'common/regexTools.cpp',
  'common/stringTools.cpp',
  'common/networkTools.cpp',
  'common/otherTools.cpp',
  'ctpp2/CTPP2VMStringLoader.cpp',
  'xapian/htmlparse.cc',
  'xapian/myhtmlparse.cc'
]
kiwix_sources += lib_resources

if xapian_dep.found()
  kiwix_sources += ['xapianIndexer.cpp', 'xapianSearcher.cpp']
endif

kiwixlib = library('kiwix',
                   kiwix_sources,
                   include_directories : inc,
                   dependencies : all_deps,
                   install : true)
