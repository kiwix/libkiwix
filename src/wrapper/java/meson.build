
java_sources = files([
  'org/kiwix/kiwixlib/JNIICU.java',
  'org/kiwix/kiwixlib/Book.java',
  'org/kiwix/kiwixlib/JNIKiwixReader.java',
  'org/kiwix/kiwixlib/Library.java',
  'org/kiwix/kiwixlib/Manager.java',
  'org/kiwix/kiwixlib/Filter.java',
  'org/kiwix/kiwixlib/JNIKiwixSearcher.java',
  'org/kiwix/kiwixlib/JNIKiwixServer.java',
  'org/kiwix/kiwixlib/JNIKiwixInt.java',
  'org/kiwix/kiwixlib/JNIKiwixString.java',
  'org/kiwix/kiwixlib/JNIKiwixBool.java',
  'org/kiwix/kiwixlib/JNIKiwixException.java',
  'org/kiwix/kiwixlib/DirectAccessInfo.java'
])

kiwix_jni = custom_target('jni',
  input: java_sources,
  output: ['org_kiwix_kiwixlib_JNIKiwix.h',
           'org_kiwix_kiwixlib_Book.h',
           'org_kiwix_kiwixlib_JNIKiwixReader.h',
           'org_kiwix_kiwixlib_Library.h',
           'org_kiwix_kiwixlib_Manager.h',
           'org_kiwix_kiwixlib_Filter.h',
           'org_kiwix_kiwixlib_JNIKiwixServer.h',
           'org_kiwix_kiwixlib_JNIKiwixSearcher.h',
           'org_kiwix_kiwixlib_JNIKiwixSearcher_Result.h'],
  command:['javac', '-d', '@OUTDIR@', '-h', '@OUTDIR@', '@INPUT@']
)

jni_sources = files([
     'kiwixicu.cpp',
     'book.cpp',
     'kiwixreader.cpp',
     'library.cpp',
     'manager.cpp',
     'filter.cpp',
     'kiwixsearcher.cpp',
     'kiwixserver.cpp',
])

kiwix_sources += jni_sources + [kiwix_jni]

if 'java' in wrapper
  kiwix_jar = jar('kiwixlib', java_sources)
  #junit_jar = files('org/kiwix/testing/junit-4.13.jar')
  #test_jar = jar('testing', 'org/kiwix/testing/test.java',
  #               link_with: [kiwix_jar, junit_jar])
  #test('javatest', test_jar)
endif

install_subdir('org', install_dir: 'kiwix-lib/java', exclude_directories: ['kiwix/testing'])
install_subdir('res', install_dir: 'kiwix-lib')
install_data('AndroidManifest.xml', install_dir: 'kiwix-lib')
