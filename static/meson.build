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
