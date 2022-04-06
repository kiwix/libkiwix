resource_files = run_command(res_manager,
                    '--list-all',
                    files('resources_list.txt')
                 ).stdout().strip().split('\n')

preprocessed_resources = custom_target('preprocessed_resource_files',
    input: 'resources_list.txt',
    output: ['resources_list.txt'],
    command:[res_manager,
             '--preprocess',
             '--outdir', '@OUTDIR@',
             '@INPUT@'],
    depend_files: resource_files
)

lib_resources = custom_target('resources',
    input: preprocessed_resources,
    output: ['kiwixlib-resources.cpp', 'kiwixlib-resources.h'],
    command:[res_compiler,
             '--cxxfile', '@OUTPUT0@',
             '--hfile', '@OUTPUT1@',
             '--source_dir', '@OUTDIR@',
             '@INPUT@'],
    depends: preprocessed_resources
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
