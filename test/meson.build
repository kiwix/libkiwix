tests = [
    'library',
    'regex',
    'tagParsing',
    'counterParsing',
    'stringTools',
    'pathTools',
    'kiwixserve',
    'book',
    'manager',
    'name_mapper',
    'opds_catalog'
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
      'zimfile&other.zim',
      'corner_cases.zim',
      'poor.zim',
      'library.xml'
    ]
    foreach file : data_files
        # configure_file(input : 'data/' + file,
        #                output : file,
        #                copy: true )
        #
        # Above (commented) command doesn't work with Meson versions below 0.47
        # (in which the 'copy' keyword was first introduced). We want to keep
        # compatibility with Ubuntu 18.04 Bionic (which has Meson version 0.45)
        # until its EOL.
        #
        # Below is a python based workaround.
        configure_file(input : 'data/' + file,
                       output : file,
                       command: [
                         find_program('python3'),
                          '-c',
                          'import sys; import shutil; shutil.copy(sys.argv[1], sys.argv[2])',
                          '@INPUT@',
                          '@OUTPUT@'
                       ])
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
