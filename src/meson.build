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
  'common/otherTools.cpp'
]
kiwix_sources += lib_resources

if get_option('android')
  subdir('android')
  install_dir = 'kiwix-lib/jniLibs/' + host_machine.cpu_family()
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
                   version: '1.0.0',
                   install: true,
                   install_dir: install_dir)
