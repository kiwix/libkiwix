/*
 * Copyright 2014 Emmanuel Engelhart <kelson@kiwix.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "tools/otherTools.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <map>
#include <pugixml.hpp>


static std::map<std::string, std::string> codeisomapping {
{ "aa", "aar" },
{ "af", "afr" },
{ "ak", "aka" },
{ "am", "amh" },
{ "ar", "ara" },
{ "as", "asm" },
{ "az", "aze" },
{ "ba", "bak" },
{ "be", "bel" },
{ "bg", "bul" },
{ "bm", "bam" },
{ "bn", "ben" },
{ "bo", "bod" },
{ "br", "bre" },
{ "bs", "bos" },
{ "ca", "cat" },
{ "ce", "che" },
{ "co", "cos" },
{ "cs", "ces" },
{ "cu", "chu" },
{ "cv", "chv" },
{ "cy", "cym" },
{ "da", "dan" },
{ "de", "deu" },
{ "dv", "div" },
{ "dz", "dzo" },
{ "ee", "ewe" },
{ "el", "ell" },
{ "en", "eng" },
{ "es", "spa" },
{ "et", "est" },
{ "eu", "eus" },
{ "fa", "fas" },
{ "ff", "ful" },
{ "fi", "fin" },
{ "fo", "fao" },
{ "fr", "fra" },
{ "fy", "fry" },
{ "ga", "gle" },
{ "gd", "gla" },
{ "gl", "glg" },
{ "gn", "grn" },
{ "gu", "guj" },
{ "gv", "glv" },
{ "ha", "hau" },
{ "he", "heb" },
{ "hi", "hin" },
{ "hr", "hrv" },
{ "hu", "hun" },
{ "hy", "hye" },
{ "ia", "ina" },
{ "id", "ind" },
{ "ig", "ibo" },
{ "is", "isl" },
{ "it", "ita" },
{ "iu", "iku" },
{ "ja", "jpn" },
{ "jv", "jav" },
{ "ka", "kat" },
{ "ki", "kik" },
{ "kk", "kaz" },
{ "kl", "kal" },
{ "km", "khm" },
{ "kn", "kan" },
{ "ko", "kor" },
{ "ks", "kas" },
{ "ku", "kur" },
{ "kw", "cor" },
{ "ky", "kir" },
{ "lb", "ltz" },
{ "lg", "lug" },
{ "ln", "lin" },
{ "lo", "lao" },
{ "lt", "lit" },
{ "lv", "lav" },
{ "mg", "mlg" },
{ "mi", "mri" },
{ "mi", "mri" },
{ "mk", "mkd" },
{ "ml", "mal" },
{ "mn", "mon" },
{ "mr", "mar" },
{ "ms", "msa" },
{ "mt", "mlt" },
{ "my", "mya" },
{ "nb", "nob" },
{ "ne", "nep" },
{ "nl", "nld" },
{ "nn", "nno" },
{ "no", "nor" },
{ "ny", "nya" },
{ "oc", "oci" },
{ "om", "orm" },
{ "or", "ori" },
{ "os", "oss" },
{ "pa", "pan" },
{ "pl", "pol" },
{ "ps", "pus" },
{ "pt", "por" },
{ "qu", "que" },
{ "rm", "roh" },
{ "rn", "run" },
{ "ro", "ron" },
{ "ru", "rus" },
{ "rw", "kin" },
{ "sa", "san" },
{ "sd", "snd" },
{ "se", "sme" },
{ "sg", "sag" },
{ "si", "sin" },
{ "sk", "slk" },
{ "sl", "slv" },
{ "sn", "sna" },
{ "so", "som" },
{ "sq", "sqi" },
{ "sr", "srp" },
{ "ss", "ssw" },
{ "st", "sot" },
{ "sv", "swe" },
{ "sw", "swa" },
{ "ta", "tam" },
{ "te", "tel" },
{ "tg", "tgk" },
{ "th", "tha" },
{ "ti", "tir" },
{ "tk", "tuk" },
{ "tl", "tgl" },
{ "tn", "tsn" },
{ "to", "ton" },
{ "tr", "tur" },
{ "ts", "tso" },
{ "tt", "tat" },
{ "ug", "uig" },
{ "uk", "ukr" },
{ "ur", "urd" },
{ "uz", "uzb" },
{ "ve", "ven" },
{ "vi", "vie" },
{ "wa", "wln" },
{ "wo", "wol" },
{ "xh", "xho" },
{ "yo", "yor" },
{ "zh", "zho" },
{ "zu", "zul" }
};

void kiwix::sleep(unsigned int milliseconds)
{
#ifdef _WIN32
  Sleep(milliseconds);
#else
  usleep(1000 * milliseconds);
#endif
}


struct XmlStringWriter: pugi::xml_writer
{
  std::string result;
  virtual void write(const void* data, size_t size){
    result.append(static_cast<const char*>(data), size);
  }
};

std::string kiwix::nodeToString(const pugi::xml_node& node)
{
  XmlStringWriter writer;
  node.print(writer, "  ");
  return writer.result;
}

std::string kiwix::converta2toa3(const std::string& a2code){
  return codeisomapping.at(a2code);
}
