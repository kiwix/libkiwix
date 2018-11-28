headers = [
  'book.h',
  'common.h',
  'library.h',
  'manager.h',
  'libxml_dumper.h',
  'opds_dumper.h',
  'downloader.h',
  'reader.h',
  'entry.h',
  'searcher.h'
]

if xapian_dep.found()
  headers += ['xapianSearcher.h']
endif

install_headers(headers, subdir:'kiwix')

install_headers(
  'common/base64.h',
  'common/networkTools.h',
  'common/otherTools.h',
  'common/pathTools.h',
  'common/regexTools.h',
  'common/stringTools.h',
  subdir:'kiwix/common'
)

if has_ctpp2_dep
  install_headers(
    'ctpp2/CTPP2VMStringLoader.hpp',
    subdir:'kiwix/ctpp2'
  )
endif

