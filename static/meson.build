if meson.version().version_compare('>=0.47.0')
    resource_files = run_command(
        res_manager,
        '--list-all',
        files('resources_list.txt'),
        check: true
    ).stdout().strip().split('\n')
else
    resource_files = run_command(
        res_manager,
        '--list-all',
        files('resources_list.txt')
    ).stdout().strip().split('\n')
endif


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
    input: [preprocessed_resources, 'i18n_resources_list.txt'],
    output: ['libkiwix-resources.cpp', 'libkiwix-resources.h'],
    command:[res_compiler,
             '--cxxfile', '@OUTPUT0@',
             '--hfile', '@OUTPUT1@',
             '--source_dir', '@OUTDIR@',
             '@INPUT@'],
    depends: preprocessed_resources
)

# This could be replaced with
# ```
# fs = import('fs')
# i18n_resource_files = fs.read('i18n_resources_list.txt').strip().split('\n')
# ```
# once we move to meson >= 0.57.0

if meson.version().version_compare('>=0.47.0')
    i18n_resource_files = run_command(
        find_program('python3'),
        '-c',
        'import sys; f=open(sys.argv[1]); print(f.read())',
        files('i18n_resources_list.txt'),
        check: true
    ).stdout().strip().split('\n')
else
    i18n_resource_files = run_command(
        find_program('python3'),
        '-c',
        'import sys; f=open(sys.argv[1]); print(f.read())',
        files('i18n_resources_list.txt'),
    ).stdout().strip().split('\n')
endif


i18n_resources = custom_target('i18n_resources',
    input: i18n_resource_files,
    output: ['libkiwix-i18n-resources.cpp'],
    command:[i18n_compiler,
             '--cxxfile', '@OUTPUT0@',
             '@INPUT@'],
)
