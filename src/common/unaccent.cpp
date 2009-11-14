#include "unaccent.h"

using namespace std;

/* Remove accent */
std::string removeAccents(const char *text) { 
  char* out = 0;
  size_t out_length = 0;
  std::string textWithoutAccent = text;

  if (!unac_string("UTF8", text, strlen(text), &out, &out_length)) {
    textWithoutAccent = string(out, out_length);
    free(out);
  }

  return textWithoutAccent;
}
