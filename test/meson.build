tests = [
    'parseUrl',
    'library',
    'regex',
    'tagParsing',
    'stringTools',
    'pathTools',
    'kiwixserve',
    'book',
    'manager',
]

if build_machine.system() != 'windows'
  tests += ['server']
endif



gtest_dep = dependency('gtest',
                       main:true,
                       fallback: ['gtest', 'gtest_main_dep'],
                       required:false)

if gtest_dep.found() and not meson.is_cross_build()
    data_files = [
      'example.zim',
      'zimfile.zim',
      'corner_cases.zim'
    ]
    foreach file : data_files
        configure_file(input : 'data/' + file,
                       output : file,
                       copy: true )
    endforeach

    foreach test_name : tests
        # XXX: implicit_include_directories must be set to false, otherwise
        # XXX: '#include <regex>' includes the regex unit test binary
        test_exe = executable(test_name, [test_name+'.cpp'],
                              implicit_include_directories: false,
                              link_with : kiwixlib,
                              link_args: extra_link_args,
                              dependencies : all_deps + [gtest_dep],
                             build_rpath : '$ORIGIN')
        test(test_name, test_exe, timeout : 160)
    endforeach
endif
