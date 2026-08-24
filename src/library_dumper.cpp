#include "library_dumper.h"

#include <mustache.hpp>

#include "book.h"
#include "libkiwix-resources.h"
#include "tools.h"
#include "tools/base64.h"
#include "tools/otherTools.h"
#include "tools/stringTools.h"

namespace kiwix
{

namespace
{

std::string getIllustrationMimeTypeStr(const Book::Illustration& illustration)
{
  std::ostringstream result;

  result << illustration.mimeType;
  if ( !contains(illustration.mimeType, ";width=") ) {
      result << ";width=" << illustration.width;
  }
  if ( !contains(illustration.mimeType, ";height=") ) {
      result << ";height=" << illustration.height;
  }
  if ( !contains(illustration.mimeType, ";scale=") ) {
      result << ";scale=1";
  }
  return result.str();
}

/**
 * Get thumbnail <link> info for a book's illustrations that already have
 * (or, for the live catalog, are always given) an external url to fetch
 * their data from.
 *
 * Live catalog: keep every icon (its own endpoint can serve embedded or
 * remote data). File dump: keep only icons with an external url, since one
 * with no url has no server to serve its data from (see
 * getEmbeddedThumbnailLinks() for those instead).
 */
kainjow::mustache::list
getExternalThumbnailLinks(const Book& book, const std::string& rootLocation,
  bool isLiveCatalog = true)
{
  kainjow::mustache::list thumbnailLinks;
  for ( const auto& illustration : book.getIllustrations() ) {
    if (!(isLiveCatalog || !illustration->url.empty())) {
      continue;
    }
    const std::string iconSizeWidth = to_string(illustration->width);
    // Book IDs do not contain special characters, so they are HTML-safe.
    // Therefore, no HTML encoding is required.
    const std::string thumbnailUrl = isLiveCatalog
    ? rootLocation + "/catalog/v2/illustration/" + kiwix::urlEncode(book.getId()) + "/?size=" + iconSizeWidth
    : illustration->url;

    thumbnailLinks.push_back(kainjow::mustache::object{
      {"icon_mimetype", getIllustrationMimeTypeStr(*illustration)},
      {"icon_url", thumbnailUrl}
    });
  }

  return thumbnailLinks;
}

/**
 * Get thumbnail <link> info for a book's illustrations that have no
 * external url, rendering their data as a "data:" URI href
 * (OPDS 1.2 5.2.2), same as getExternalThumbnailLinks()'s external-url
 * links.
 *
 * Only applies to file dumps (never the live catalog, which serves embedded
 * data from its own endpoint instead) and only for icons without an external
 * url, i.e. those that would otherwise have no way to reach the reader.
 */
kainjow::mustache::list
getEmbeddedThumbnailLinks(const Book& book, bool isLiveCatalog = true)
{
  kainjow::mustache::list thumbnailLinks;
  if (isLiveCatalog) {
    return thumbnailLinks;
  }
  for ( const auto& illustration : book.getIllustrations() ) {
    if (!illustration->url.empty()) {
      continue;
    }
    const std::string thumbnailData = illustration->getData();
    const std::string dataUri = thumbnailData.empty() ? ""
      : "data:" + illustration->mimeType + ";base64," + base64_encode(thumbnailData);
    thumbnailLinks.push_back(kainjow::mustache::object{
      {"icon_mimetype", getIllustrationMimeTypeStr(*illustration)},
      {"icon_url", dataUri}
    });
  }

  return thumbnailLinks;
}

} // namespace


std::string fullEntryOpds(const Book& book,
                         const std::string& rootLocation,
                         const std::string& contentAccessUrl,
                         const std::string& contentId,
                         const std::string& selfPath,
                         bool isLiveCatalog)
{
    const auto bookDate = book.getDate() + "T00:00:00Z";
    auto thumbnailLinks
      = getExternalThumbnailLinks(book, rootLocation, isLiveCatalog);
    auto embeddedThumbnailLinks
      = getEmbeddedThumbnailLinks(book, isLiveCatalog);
    thumbnailLinks.insert(thumbnailLinks.end(),
      std::make_move_iterator(embeddedThumbnailLinks.begin()),
      std::make_move_iterator(embeddedThumbnailLinks.end()));
    const kainjow::mustache::object data{
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
      {"thumbnailLinks", thumbnailLinks},
      {"self_path", onlyAsNonEmptyMustacheValue(selfPath)},
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
