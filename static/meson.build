
cat = find_program('cat', required: false)

if cat.found()
    resource_files = run_command('cat',
                        meson.current_source_dir(),
                        'resources_list.txt'
                     ).stdout().strip().split('\n')
    always_build_lib_resources = false
else
    resource_files = 'resources_list.txt'
    always_build_lib_resources = true
endif

lib_resources = custom_target('resources',
    input: 'resources_list.txt',
    output: ['kiwixlib-resources.cpp', 'kiwixlib-resources.h'],
    command:[res_compiler,
             '--cxxfile', '@OUTPUT0@',
             '--hfile', '@OUTPUT1@',
             '--source_dir', '@OUTDIR@',
             '@INPUT@'],
    depend_files: resource_files,
    build_always_stale: always_build_lib_resources
)
