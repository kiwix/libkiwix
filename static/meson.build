
lib_resources = custom_target('resources',
    input: 'resources_list.txt',
    output: ['kiwixlib-resources.cpp', 'kiwixlib-resources.h'],
    command:[res_compiler,
             '--cxxfile', '@OUTPUT0@',
             '--hfile', '@OUTPUT1@',
             '--source_dir', '@OUTDIR@',
             '@INPUT@'],
    build_always_stale: true
)
