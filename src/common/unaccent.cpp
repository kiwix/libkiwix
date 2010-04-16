/********************************************************************
 * COPYRIGHT:
 * Copyright (c) 1999-2003, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/

#include "unaccent.h"

const char UnaccentTransliterator::fgClassID = 0;

/**
 * Constructor
 */
UnaccentTransliterator::UnaccentTransliterator() :
    normalizer("", UNORM_NFD),
    Transliterator("Unaccent", 0) {
}

/**
 * Destructor
 */
UnaccentTransliterator::~UnaccentTransliterator() {
}

/**
 * Remove accents from a character using Normalizer.
 */
UChar UnaccentTransliterator::unaccent(UChar c) const {
    UnicodeString str(c);
    UErrorCode status = U_ZERO_ERROR;
    UnaccentTransliterator* t = (UnaccentTransliterator*)this;

    t->normalizer.setText(str, status);
    if (U_FAILURE(status)) {
        return c;
    }
    return (UChar) t->normalizer.next();
}

/**
 * Implement Transliterator API
 */
void UnaccentTransliterator::handleTransliterate(Replaceable& text,
                                                 UTransPosition& index,
                                                 UBool incremental) const {
    UnicodeString str("a");
    while (index.start < index.limit) {
        UChar c = text.charAt(index.start);
        UChar d = unaccent(c);
        if (c != d) {
            str.setCharAt(0, d);
            text.handleReplaceBetween(index.start, index.start+1, str);
        }
        index.start++;
    }
}

/* Remove accents from a String */
UnaccentTransliterator unaccent;
char *unaccentedString = NULL;
unsigned unaccentedStringSize=0;
UnicodeString unicodeAccentedString;

const char* removeAccents(const char *accentedString, const unsigned size) {

  /* Realloc memory if necessary */
  if (size > unaccentedStringSize) {
    unaccentedString = (char*)realloc(unaccentedString, size+1);
    unaccentedStringSize = size+1;
  }

  /* Transcode the String */
  unicodeAccentedString = UnicodeString(accentedString);
  unaccent.transliterate(unicodeAccentedString);
  
  /* Extract and return the result */
  unicodeAccentedString.extract(0, size, unaccentedString, size, "UTF-8");
  return unaccentedString;
}
