#include "unaccent.h"

UErrorCode status = U_ZERO_ERROR;
Transliterator *trans = Transliterator::createInstance("Lower; NFD; [:M:] remove; NFC", UTRANS_FORWARD, status);

std::string &removeAccents(std::string &text) {
  ucnv_setDefaultName("UTF-8");
  UnicodeString ustring = UnicodeString(text.c_str());
  trans->transliterate(ustring);
  text.clear();
  ustring.toUTF8String(text);
  return text;
}

void printStringInHexadecimal(UnicodeString s) {
  std::cout << std::showbase << std::hex;
  for (int i=0; i<s.length(); i++) {
    char c = (char)((s.getTerminatedBuffer())[i]);
    if (c & 0x80)
      std::cout << (c & 0xffff) << " ";
    else
      std::cout << c << " ";
  }
  std::cout << std::endl;
}

void printStringInHexadecimal(const char *s) {
  std::cout << std::showbase << std::hex;
  for (char const* pc = s; *pc; ++pc) {
    if (*pc & 0x80)
      std::cout << (*pc & 0xffff);
    else
      std::cout << *pc;
    std::cout << ' ';
  }
  std::cout << std::endl;
}
