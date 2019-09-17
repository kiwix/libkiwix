

tests = [
    'parseUrl',
    'library',
    'regex',
    'tagParsing'
]


gtest_dep = dependency('gtest',
                       main:true,
                       fallback: ['gtest', 'gtest_dep'],
                       required:false)

if gtest_dep.found()
    foreach test_name : tests
        test_exe = executable(test_name, [test_name+'.cpp'],
                              link_with : kiwixlib,
                              link_args: extra_link_args,
                              dependencies : all_deps + [gtest_dep],
                             build_rpath : '$ORIGIN')
        test(test_name, test_exe)
    endforeach
endif
