

ctpp2c = find_program('ctpp2c')
search_result_template = custom_target('result_template',
  input: 'results.tmpl',
  output: 'results.ct2',
  command: [intermediate_ctpp2c, ctpp2c, '@INPUT@', '@OUTPUT@']
)

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
