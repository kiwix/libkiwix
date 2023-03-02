#include "library_dumper.h"
#include "tools/stringTools.h"
#include "tools/otherTools.h"
#include "tools.h"
#include "tools/regexTools.h"

namespace kiwix
{
/* Constructor */
LibraryDumper::LibraryDumper(const Library* library, const NameMapper* nameMapper)
  : library(library),
    nameMapper(nameMapper)
{
}
/* Destructor */
LibraryDumper::~LibraryDumper()
{
}

void LibraryDumper::setOpenSearchInfo(int totalResults, int startIndex, int count)
{
  m_totalResults = totalResults;
  m_startIndex = startIndex,
  m_count = count;
}

namespace {

std::map<std::string, std::string> iso639_3 = {
  {"atj", "atikamekw"},
  {"azb", "آذربایجان دیلی"},
  {"bcl", "central bikol"},
  {"bgs", "tagabawa"},
  {"bxr", "буряад хэлэн"},
  {"cbk", "chavacano"},
  {"cdo", "閩東語"},
  {"dag", "Dagbani"},
  {"diq", "dimli"},
  {"dty", "डोटेली"},
  {"eml", "emiliân-rumagnōl"},
  {"fbs", "српскохрватски"},
  {"hbs", "srpskohrvatski"},
  {"ido", "ido"},
  {"kbp", "kabɩyɛ"},
  {"kld", "Gamilaraay"},
  {"lbe", "лакку маз"},
  {"lbj", "ལ་དྭགས་སྐད་"},
  {"map", "Austronesian"},
  {"mhr", "марий йылме"},
  {"mnw", "ဘာသာမန်"},
  {"myn", "mayan"},
  {"nah", "nahuatl"},
  {"nai", "north American Indian"},
  {"nds", "plattdütsch"},
  {"nrm", "bhasa narom"},
  {"olo", "livvi"},
  {"pih", "Pitcairn-Norfolk"},
  {"pnb", "Western Panjabi"},
  {"rmr", "Caló"},
  {"rmy", "romani shib"},
  {"roa", "romance languages"},
  {"twi", "twi"},
};

std::once_flag fillLanguagesFlag;

void fillLanguagesMap()
{
  for (auto icuLangPtr = icu::Locale::getISOLanguages(); *icuLangPtr != NULL; ++icuLangPtr) {
    const ICULanguageInfo lang(*icuLangPtr);
    iso639_3.insert({lang.iso3Code(), lang.selfName()});
  }
}

std::string getLanguageSelfName(const std::string& lang) {
  const auto itr = iso639_3.find(lang);
  if (itr != iso639_3.end()) {
    return itr->second;
  }
  return lang;
};

} // unnamed namespace

kainjow::mustache::list LibraryDumper::getCategoryData() const
{
  const auto now = gen_date_str();
  kainjow::mustache::list categoryData;
  for ( const auto& category : library->getBooksCategories() ) {
    const auto urlencodedCategoryName = urlEncode(category);
    categoryData.push_back(kainjow::mustache::object{
      {"name", category},
      {"urlencoded_name",  urlencodedCategoryName},
      {"updated", now},
      {"id", gen_uuid(libraryId + "/categories/" + urlencodedCategoryName)}
    });
  }
  return categoryData;
}

kainjow::mustache::list LibraryDumper::getLanguageData() const
{
  const auto now = gen_date_str();
  kainjow::mustache::list languageData;
  std::call_once(fillLanguagesFlag, fillLanguagesMap);
  for ( const auto& langAndBookCount : library->getBooksLanguagesWithCounts() ) {
    const std::string languageCode = langAndBookCount.first;
    const int bookCount = langAndBookCount.second;
    const auto languageSelfName = getLanguageSelfName(languageCode);
    languageData.push_back(kainjow::mustache::object{
      {"lang_code",  languageCode},
      {"lang_self_name", languageSelfName},
      {"book_count", to_string(bookCount)},
      {"updated", now},
      {"id", gen_uuid(libraryId + "/languages/" + languageCode)}
    });
  }
  return languageData;
}

} // namespace kiwix
