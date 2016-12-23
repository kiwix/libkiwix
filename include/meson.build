headers = [
  'indexer.h',
  'library.h',
  'manager.h',
  'reader.h',
  'searcher.h'
]

if xapian_dep.found()
  headers += ['xapianIndexer.h', 'xapianSearcher.h']
endif

install_headers(headers, subdir:'kiwix')

install_headers(
  'common/base64.h',
  'common/networkTools.h',
  'common/otherTools.h',
  'common/pathTools.h',
  'common/regexTools.h',
  'common/stringTools.h',
  'common/tree.h',
  subdir:'kiwix/common'
)


install_headers(
  'ctpp2/CTPP2VMStringLoader.hpp',
  subdir:'kiwix/ctpp2'
)
