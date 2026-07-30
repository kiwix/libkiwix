#include "library_dumper.h"
#include "book.h"

#include "libkiwix-resources.h"
#include <mustache.hpp>

#include "tools/stringTools.h"
#include "tools/otherTools.h"
#include "tools.h"

namespace kiwix
{

kainjow::mustache::list getBookIllustrationInfo(const Book& book)
{
    kainjow::mustache::list illustrations;
    for ( const auto& illustration : book.getIllustrations() ) {
      // For now, we are handling only sizexsize@1 illustration.
      // So we can simply pass one size to mustache.
      illustrations.push_back(kainjow::mustache::object{
        {"icon_size", to_string(illustration->width)},
        {"icon_mimetype", illustration->mimeType}
      });
    }
    return illustrations;
}

std::string fullEntryOpds(const Book& book,
                         const std::string& rootLocation,
                         const std::string& contentAccessUrl,
                         const std::string& contentId)
{
    const auto bookDate = book.getDate() + "T00:00:00Z";
    const kainjow::mustache::object data{
      {"root",  rootLocation},
      {"contentAccessUrl",  onlyAsNonEmptyMustacheValue(contentAccessUrl)},
      {"id", book.getId()},
      {"name", book.getName()},
      {"title", book.getTitle()},
      {"description", book.getDescription()},
      {"language", book.getCommaSeparatedLanguages()},
      {"content_id",  urlEncode(contentId)},
      {"updated", bookDate}, // XXX: this should be the entry update datetime
      {"book_date", bookDate},
      {"category", book.getCategory()},
      {"flavour", book.getFlavour()},
      {"tags", book.getTags()},
      {"article_count", to_string(book.getArticleCount())},
      {"media_count", to_string(book.getMediaCount())},
      {"author_name", book.getCreator()},
      {"publisher_name", book.getPublisher()},
      {"url", onlyAsNonEmptyMustacheValue(book.getUrl())},
      {"size", to_string(book.getSize())},
      {"icons", getBookIllustrationInfo(book)},
    };
    return render_template(RESOURCE::templates::catalog_v2_entry_xml, data);
}

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
