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

#include <tools/otherTools.h>
#include <map>

static std::map<std::string, std::string> codeisomapping {
//a
  { "ad", "and" },
  { "ae", "are" },
  { "af", "afg" },
  { "ag", "atg" },
  { "ai", "aia" },
  { "al", "alb" },
  { "am", "arm" },
  { "an", "ant" },
  { "ao", "ago" },
  { "aq", "ata" },
  { "ar", "arg" },
  { "as", "asm" },
  { "at", "aut" },
  { "au", "aus" },
  { "aw", "abw" },
  { "ax", "ala" },
  { "az", "aze" },
//b
  { "ba", "bih" },
  { "bb", "brb" },
  { "bd", "bgd" },
  { "be", "bel" },
  { "bf", "bfa" },
  { "bg", "bgr" },
  { "bh", "bhr" },
  { "bi", "bdi" },
  { "bj", "ben" },
  { "bl", "blm" },
  { "bn", "brn" },
  { "bm", "bmu" },
  { "bo", "bol" },
  { "br", "bra" },
  { "bs", "bhs" },
  { "bt", "btn" },
  { "bv", "bvt" },
  { "bw", "bwa" },
  { "by", "blr" },
  { "bz", "blz" },
//c
  { "ca", "can" },
  { "cc", "cck" },
  { "cd", "cod" },
  { "cf", "caf" },
  { "cg", "cog" },
  { "ch", "che" },
  { "ci", "civ" },
  { "ck", "cok" },
  { "cl", "chl" },
  { "cm", "cmr" },
  { "cn", "chn" },
  { "co", "col" },
  { "cr", "cri" },
  { "cu", "cub" },
  { "cv", "cpv" },
  { "cx", "cxr" },
  { "cy", "cyp" },
  { "cz", "cze" },
//d
  { "de", "deu" },
  { "dj", "dji" },
  { "dk", "dnk" },
  { "dm", "dma" },
  { "do", "dom" },
  { "dz", "dza" },
//e
  { "ec", "ecu" },
  { "ee", "est" },
  { "eg", "egy" },
  { "eh", "esh" },
  { "en", "eng" },
  { "er", "eri" },
  { "es", "esp" },
  { "et", "eth" },
//f
  { "fi", "fin" },
  { "fj", "fji" },
  { "fk", "flk" },
  { "fm", "fsm" },
  { "fo", "fro" },
  { "fr", "fra" },
//g
  { "ga", "gab" },
  { "gb", "gbr" },
  { "gd", "grd" },
  { "ge", "geo" },
  { "gf", "guf" },
  { "gg", "ggy" },
  { "gh", "gha" },
  { "gi", "gib" },
  { "gl", "grl" },
  { "gm", "gmb" },
  { "gn", "gin" },
  { "gp", "glp" },
  { "gq", "gnq" },
  { "gr", "grc" },
  { "gs", "sgs" },
  { "gt", "gtm" },
  { "gu", "gum" },
  { "gw", "gnb" },
  { "gy", "guy" },
//h
  { "hk", "hkg" },
  { "hm", "hmd" },
  { "hn", "hnd" },
  { "hr", "hrv" },
  { "ht", "hti" },
  { "hu", "hun" },
//i
  { "id", "idn" },
  { "ie", "irl" },
  { "il", "isr" },
  { "im", "imn" },
  { "in", "ind" },
  { "io", "iot" },
  { "iq", "irq" },
  { "ir", "irn" },
  { "is", "isl" },
  { "it", "ita" },
//j
  { "je", "jey" },
  { "jm", "jam" },
  { "jo", "jor" },
  { "jp", "jpn" },
//k
  { "ke", "ken" },
  { "kg", "kgz" },
  { "kh", "khm" },
  { "ki", "kir" },
  { "km", "com" },
  { "kn", "kna" },
  { "kp", "prk" },
  { "kr", "kor" },
  { "kw", "kwt" },
  { "ky", "cym" },
  { "kz", "kaz" },
//l
  { "la", "lao" },
  { "lb", "lbn" },
  { "lc", "lca" },
  { "li", "lie" },
  { "lk", "lka" },
  { "lr", "lbr" },
  { "ls", "lso" },
  { "lt", "ltu" },
  { "lu", "lux" },
  { "lv", "lva" },
  { "ly", "lby" },
//m
  { "ma", "mar" },
  { "mc", "mco" },
  { "md", "mda" },
  { "me", "mne" },
  { "mf", "maf" },
  { "mg", "mdg" },
  { "mh", "mhl" },
  { "mk", "mkd" },
  { "ml", "mli" },
  { "mm", "mmr" },
  { "mn", "mng" },
  { "mo", "mac" },
  { "mp", "mnp" },
  { "mq", "mtq" },
  { "mr", "mrt" },
  { "ms", "msr" },
  { "mt", "mlt" },
  { "mu", "mus" },
  { "mv", "mdv" },
  { "mw", "mwi" },
  { "mx", "mex" },
  { "my", "mys" },
  { "mz", "moz" },
//n
  { "na", "nam" },
  { "nc", "ncl" },
  { "ne", "ner" },
  { "nf", "nfk" },
  { "ng", "nga" },
  { "ni", "nic" },
  { "nl", "nld" },
  { "no", "nor" },
  { "np", "npl" },
  { "nr", "nru" },
  { "nu", "niu" },
  { "nz", "nzl" },
//o
  { "om", "omn" },
//p
  { "pa", "pan" },
  { "pe", "per" },
  { "pf", "pyf" },
  { "pg", "png" },
  { "ph", "phl" },
  { "pk", "pak" },
  { "pl", "pol" },
  { "pm", "spm" },
  { "pn", "pcn" },
  { "pr", "pri" },
  { "ps", "pse" },
  { "pt", "prt" },
  { "pw", "plw" },
  { "py", "pry" },
//q
  { "qa", "qat" },
//r
  { "re", "reu" },
  { "ro", "rou" },
  { "rs", "srb" },
  { "ru", "rus" },
  { "rw", "rwa" },
//s
  { "sa", "sau" },
  { "sb", "slb" },
  { "sc", "syc" },
  { "sd", "sdn" },
  { "se", "swe" },
  { "sg", "sgp" },
  { "sh", "shn" },
  { "si", "svn" },
  { "sj", "sjm" },
  { "sk", "svk" },
  { "sl", "sle" },
  { "sm", "smr" },
  { "sn", "sen" },
  { "so", "som" },
  { "sr", "sur" },
  { "ss", "ssd" },
  { "st", "stp" },
  { "sv", "slv" },
  { "sy", "syr" },
  { "sz", "swz" },
//t
  { "tc", "tca" },
  { "td", "tcd" },
  { "tf", "atf" },
  { "tg", "tgo" },
  { "th", "tha" },
  { "tj", "tjk" },
  { "tk", "tkl" },
  { "tl", "tls" },
  { "tm", "tkm" },
  { "tn", "tun" },
  { "to", "ton" },
  { "tr", "tur" },
  { "tt", "tto" },
  { "tv", "tuv" },
  { "tw", "twn" },
  { "tz", "tza" },
//u
  { "ua", "ukr" },
  { "ug", "uga" },
  { "um", "umi" },
  { "us", "usa" },
  { "uy", "ury" },
  { "uz", "uzb" },
//v
  { "va", "vat" },
  { "vc", "vct" },
  { "ve", "ven" },
  { "vg", "vgb" },
  { "vi", "vir" },
  { "vn", "vnm" },
  { "vu", "vut" },
//w
  { "wf", "wlf" },
  { "ws", "wsm" },
//y
  { "ye", "yem" },
  { "yt", "myt" },
// z
  { "za", "zaf" },
  { "zm", "zmb" },
  { "zw", "zwe" }
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

std::string kiwix::nodeToString(pugi::xml_node node)
{
  XmlStringWriter writer;
  node.print(writer, "  ");
  return writer.result;
}

std::string kiwix::converta2toa3(const std::string& a2code){
  return codeisomapping.at(a2code);
}
