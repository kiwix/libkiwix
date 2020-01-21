

tests = [
    'parseUrl',
    'library',
    'regex',
    'tagParsing',
    'stringTools',
    'pathTools',
    'kiwixserve'
]


gtest_dep = dependency('gtest',
                       main:true,
                       fallback: ['gtest', 'gtest_main_dep'],
                       required:false)

if gtest_dep.found() and not meson.is_cross_build()
    foreach test_name : tests
        test_exe = executable(test_name, [test_name+'.cpp'],
                              link_with : kiwixlib,
                              link_args: extra_link_args,
                              dependencies : all_deps + [gtest_dep],
                             build_rpath : '$ORIGIN')
        test(test_name, test_exe)
    endforeach
endif
