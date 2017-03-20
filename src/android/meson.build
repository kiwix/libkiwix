
jni_generator = find_program('gen_kiwix.sh')

kiwix_jni = custom_target('jni',
  input: ['org/kiwix/kiwixlib/JNIKiwix.java',
          'org/kiwix/kiwixlib/JNIKiwixInt.java',
          'org/kiwix/kiwixlib/JNIKiwixString.java',
          'org/kiwix/kiwixlib/JNIKiwixBool.java'],
  output: ['org_kiwix_kiwixlib_JNIKiwix.h'],
  command:[jni_generator, '@INPUT@']
)

kiwix_sources += ['android/kiwix.cpp', kiwix_jni]
