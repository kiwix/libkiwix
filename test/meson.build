
tests = [
    'parseUrl',
    'library',
    'regex',
    'tagParsing',
    'stringTools',
    'pathTools',
    'kiwixserve',
    'server',
    'book'
]


gtest_dep = dependency('gtest',
                       main:true,
                       fallback: ['gtest', 'gtest_main_dep'],
                       required:false)

if gtest_dep.found() and not meson.is_cross_build()
    configure_file(input : 'data/wikipedia_en_ray_charles_mini_2020-03.zim',
                   output : 'zimfile.zim',
                   copy: true )

    foreach test_name : tests
        test_sources = [test_name+'.cpp']
        if test_name == 'server'
            test_sources += 'httplib.cpp'
        endif
        # XXX: implicit_include_directories must be set to false, otherwise
        # XXX: '#include <regex>' includes the regex unit test binary
        test_exe = executable(test_name, test_sources,
                              implicit_include_directories: false,
                              link_with : kiwixlib,
                              link_args: extra_link_args,
                              dependencies : all_deps + [gtest_dep],
                             build_rpath : '$ORIGIN')
        test(test_name, test_exe, workdir: meson.current_build_dir())
    endforeach
endif
