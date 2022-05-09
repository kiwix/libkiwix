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

i18n_resource_listing_script = '''import glob
import os
import sys
qqq = sys.argv[1]
d = os.path.dirname(qqq)
r = glob.glob(d + "/*.json")
r.remove(qqq)
for f in r: print(f)
'''

i18n_resource_listing_cmd = [
                                find_program('python3'),
                                '-c',
                                i18n_resource_listing_script,
                                files('i18n/qqq.json')
                            ]

i18n_resource_files = run_command(i18n_resource_listing_cmd).stdout().strip().split('\n')

i18n_resources = custom_target('i18n_resources',
    input: i18n_resource_files,
    output: ['libkiwix-i18n-resources.cpp'],
    command:[i18n_compiler,
             '--cxxfile', '@OUTPUT0@',
             '@INPUT@'],
)
