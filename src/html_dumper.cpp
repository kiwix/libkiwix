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
    const auto bookEx = library->getBookById(bookId);
    const auto bookName = bookEx.getName();
    booksData.push_back(kainjow::mustache::object{
      {"name", bookName}
    });
  }

  return render_template(
             RESOURCE::templates::no_js_library_page_html,
             kainjow::mustache::object{
               {"books", booksData }
             }
  );
}

} // namespace kiwix
