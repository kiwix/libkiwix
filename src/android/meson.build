
kiwix_jni = custom_target('jni',
  input: ['org/kiwix/kiwixlib/JNIICU.java',
          'org/kiwix/kiwixlib/JNIKiwixReader.java',
          'org/kiwix/kiwixlib/JNIKiwixSearcher.java',
          'org/kiwix/kiwixlib/JNIKiwixInt.java',
          'org/kiwix/kiwixlib/JNIKiwixString.java',
          'org/kiwix/kiwixlib/JNIKiwixBool.java',
          'org/kiwix/kiwixlib/JNIKiwixException.java',
          'org/kiwix/kiwixlib/Pair.java'],
  output: ['org_kiwix_kiwixlib_JNIKiwix.h',
           'org_kiwix_kiwixlib_JNIKiwixReader.h',
           'org_kiwix_kiwixlib_JNIKiwixSearcher.h',
           'org_kiwix_kiwixlib_JNIKiwixSearcher_Result.h'],
  command:['javac', '-d', '@OUTDIR@', '-h', '@OUTDIR@', '@INPUT@']
)

kiwix_sources += [
    'android/kiwixicu.cpp',
    'android/kiwixreader.cpp',
    'android/kiwixsearcher.cpp',
    kiwix_jni]

install_subdir('org', install_dir: 'kiwix-lib/java')
install_subdir('res', install_dir: 'kiwix-lib')
install_data('AndroidManifest.xml', install_dir: 'kiwix-lib')
