#include "html_dumper.h"
#include "libkiwix-resources.h"
#include "tools/otherTools.h"

namespace kiwix
{

/* Constructor */
HTMLDumper::HTMLDumper(const Library* library, const NameMapper* nameMapper)
  : LibraryDumper(library, nameMapper)
{
}
/* Destructor */
HTMLDumper::~HTMLDumper()
{
}

std::string HTMLDumper::dumpPlainHTML() const
{
  kainjow::mustache::list booksData;
  for ( const auto& bookId : library->getBooksIds() ) {
    const auto bookObj = library->getBookById(bookId);
    const auto bookTitle = bookObj.getTitle();
    const auto bookDescription = bookObj.getDescription();
    const auto langCode = bookObj.getCommaSeparatedLanguages();
    const auto bookIconUrl = rootLocation + "/catalog/v2/illustration/" + bookId +  "/?size=48";
    std::string faviconAttr = "style=background-image:url(" + bookIconUrl + ")";
    booksData.push_back(kainjow::mustache::object{
      {"title", bookTitle},
      {"description", bookDescription},
      {"langCode", langCode},
      {"faviconAttr", faviconAttr}
    });
  }

  return render_template(
             RESOURCE::templates::no_js_library_page_html,
             kainjow::mustache::object{
               {"root", rootLocation},
               {"books", booksData }
             }
  );
}

} // namespace kiwix
