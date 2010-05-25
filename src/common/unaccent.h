#ifndef KIWIX_UNACCENT_H
#define KIWIX_UNACCENT_H

#include <unicode/translit.h>
#include <unicode/normlzr.h>
#include <unicode/unistr.h>
#include <unicode/rep.h>
#include <unicode/translit.h>
#include <unicode/uniset.h>
#include <unicode/ustring.h>
#include <unicode/ucnv.h>

#include <iostream>
#include <string>

std::string &removeAccents(std::string &text);
void printStringInHexadecimal(const char *s);
void printStringInHexadecimal(UnicodeString s);

#endif
