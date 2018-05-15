

ctpp2c = find_program('ctpp2c', required:false)

if ctpp2c.found()
  search_result_template = custom_target('result_template',
    input: 'results.tmpl',
    output: 'results.ct2',
    command: [intermediate_ctpp2c, ctpp2c, '@INPUT@', '@OUTPUT@']
  )
  resources_list = 'resources_list_ctpp2.txt'
  lib_resources = custom_target('resources',
    input: 'resources_list.txt',
    output: ['kiwixlib-resources.cpp', 'kiwixlib-resources.h'],
    command:[res_compiler,
             '--cxxfile', '@OUTPUT0@',
             '--hfile', '@OUTPUT1@',
             '--source_dir', '@OUTDIR@',
             '@INPUT@'],
    depends: [search_result_template]
  )
else
 lib_resources = []
endif
