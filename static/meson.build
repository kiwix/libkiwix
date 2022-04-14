resource_files = run_command(find_program('python3'),
                    '-c',
                    'import sys; f=open(sys.argv[1]); print(f.read())',
                    files('resources_list.txt')
                 ).stdout().strip().split('\n')

lib_resources = custom_target('resources',
    input: 'resources_list.txt',
    output: ['kiwixlib-resources.cpp', 'kiwixlib-resources.h'],
    command:[res_compiler,
             '--cxxfile', '@OUTPUT0@',
             '--hfile', '@OUTPUT1@',
             '--source_dir', '@OUTDIR@',
             '@INPUT@'],
    depend_files: resource_files
)

i18n_resource_files = run_command(find_program('python3'),
                    '-c',
                    'import sys; f=open(sys.argv[1]); print(f.read())',
                    files('i18n_resources_list.txt')
                 ).stdout().strip().split('\n')

i18n_resources = custom_target('i18n_resources',
    input: 'i18n_resources_list.txt',
    output: ['libkiwix-i18n-resources.cpp'],
    command:[i18n_compiler,
             '--cxxfile', '@OUTPUT0@',
             '@INPUT@'],
    depend_files: i18n_resource_files
)
