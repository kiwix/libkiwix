#include "unaccent.h"

using namespace std;

/* Remove accent */
std::string removeAccents(const char *text = NULL) { 
  char* out = NULL;
  size_t out_length = 0;

  if (!unac_string("UTF8", text, strlen(text), &out, &out_length)) {
    std::string textWithoutAccent = text;
    textWithoutAccent = string(out, out_length);
    free(out);
    return textWithoutAccent;
  } 

  if (text != NULL) {
    return string(text);
  }

  return "";
}
