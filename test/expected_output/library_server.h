////////////////////////////////////////////////////////////////////////////////
// Building blocks for OPDS output
////////////////////////////////////////////////////////////////////////////////

#include "./building_blocks/opds/catalog/v2/charles_ray.h"
#include "./building_blocks/opds/catalog/v2/charles_ray_no_namemapper.h"
#include "./building_blocks/opds/catalog/v2/ray_charles.h"
#include "./building_blocks/opds/catalog/v2/ray_charles_no_namemapper.h"
#include "./building_blocks/opds/catalog/v2/uncategorized_ray_charles.h"
#include "./building_blocks/opds/catalog/v2/inaccessiblezim.h"

#define CATALOG_V2_ENTRIES_PREAMBLE0(x)                       \
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"            \
    "<feed xmlns=\"http://www.w3.org/2005/Atom\"\n"           \
    "      xmlns:dc=\"http://purl.org/dc/terms/\"\n"          \
    "      xmlns:opds=\"https://specs.opds.io/opds-1.2\"\n"   \
    "      xmlns:opensearch=\"http://a9.com/-/spec/opensearch/1.1/\">\n"  \
    "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"       \
    "\n"                                                      \
    "  <link rel=\"self\"\n"                                  \
    "        href=\"/ROOT%23%3F/catalog/v2/" x "\"\n"                    \
    "        type=\"application/atom+xml;profile=opds-catalog;kind=acquisition\"/>\n" \
    "  <link rel=\"start\"\n"                                 \
    "        href=\"/ROOT%23%3F/catalog/v2/root.xml\"\n"              \
    "        type=\"application/atom+xml;profile=opds-catalog;kind=navigation\"/>\n" \
    "  <link rel=\"up\"\n"                                    \
    "        href=\"/ROOT%23%3F/catalog/v2/root.xml\"\n"              \
    "        type=\"application/atom+xml;profile=opds-catalog;kind=navigation\"/>\n" \
    "\n"                                                      \

#define CATALOG_V2_ENTRIES_PREAMBLE(q) \
            CATALOG_V2_ENTRIES_PREAMBLE0("entries" q)

#define CATALOG_V2_PARTIAL_ENTRIES_PREAMBLE(q) \
            CATALOG_V2_ENTRIES_PREAMBLE0("partial_entries" q)


////////////////////////////////////////////////////////////////////////////////
// Building blocks for HTML output from nojs endpoint
////////////////////////////////////////////////////////////////////////////////

#define HTML_PREAMBLE \
  "<!DOCTYPE html>\n" \
  "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n" \
  "  <head>\n" \
  "    <meta charset=\"UTF-8\" />\n" \
  "    <meta name=\"viewport\" content=\"width=device-width,initial-scale=1\" />\n" \
  "    <title>Welcome to Kiwix Server</title>\n" \
  "    <link\n" \
  "      type=\"text/css\"\n" \
  "      href=\"/ROOT%23%3F/skin/kiwix.css?cacheid=b4e29e64\"\n" \
  "      rel=\"Stylesheet\"\n" \
  "    />\n" \
  "    <link\n" \
  "      type=\"text/css\"\n" \
  "      href=\"/ROOT%23%3F/skin/index.css?cacheid=469a47e3\"\n" \
  "      rel=\"Stylesheet\"\n" \
  "    />\n" \
  "    <link rel=\"apple-touch-icon\" sizes=\"180x180\" href=\"/ROOT%23%3F/skin/favicon/apple-touch-icon.png?cacheid=f86f8df3\">\n" \
  "    <link rel=\"icon\" type=\"image/png\" sizes=\"32x32\" href=\"/ROOT%23%3F/skin/favicon/favicon-32x32.png?cacheid=79ded625\">\n" \
  "    <link rel=\"icon\" type=\"image/png\" sizes=\"16x16\" href=\"/ROOT%23%3F/skin/favicon/favicon-16x16.png?cacheid=a986fedc\">\n" \
  "    <link rel=\"manifest\" href=\"/ROOT%23%3F/skin/favicon/site.webmanifest?cacheid=bc396efb\">\n" \
  "    <link rel=\"mask-icon\" href=\"/ROOT%23%3F/skin/favicon/safari-pinned-tab.svg?cacheid=8d487e95\" color=\"#5bbad5\">\n" \
  "    <link rel=\"shortcut icon\" href=\"/ROOT%23%3F/skin/favicon/favicon.ico?cacheid=92663314\">\n" \
  "    <meta name=\"msapplication-TileColor\" content=\"#da532c\">\n" \
  "    <meta name=\"msapplication-config\" content=\"/ROOT%23%3F/skin/favicon/browserconfig.xml?cacheid=f29a7c4a\">\n" \
  "    <meta name=\"theme-color\" content=\"#ffffff\">\n" \
  "    <style>\n" \
  "      @font-face {\n" \
  "        font-family: \"poppins\";\n" \
  "        src: url(\"/ROOT%23%3F/skin/fonts/Poppins.ttf?cacheid=af705837\") format(\"truetype\");\n" \
  "      }\n\n" \
  "      @font-face {\n" \
  "          font-family: \"roboto\";\n" \
  "          src: url(\"/ROOT%23%3F/skin/fonts/Roboto.ttf?cacheid=84d10248\") format(\"truetype\");\n" \
  "      }\n\n" \
  "      .book__list {\n" \
  "        display: flex;\n" \
  "        flex-direction: row;\n" \
  "        flex-wrap: wrap;\n" \
  "        align-items: center;\n" \
  "      }\n\n" \
  "      .book__wrapper:hover {\n" \
  "        transform: scale(1.0);\n" \
  "      }\n\n" \
  "      .tag__link {\n" \
  "        pointer-events: none;\n" \
  "      }\n\n" \
  "      .kiwixHomeBody__results {\n" \
  "        flex-basis: 100%;\n" \
  "      }\n\n" \
  "      #book__title>a {\n" \
  "        text-decoration: none;\n" \
  "        all: unset;\n" \
  "      }\n" \
  "    </style>\n" \
  "  </head>\n" \
  "  <body>\n" \
  "    <div class='kiwixNav'>\n"

#define CHARLES_RAY_BOOK_HTML \
  "        <div class=\"book__wrapper\">\n" \
  "            <a class=\"book__link\" href=\"/ROOT%23%3F/content/zimfile%26other\" title=\"Preview\" aria-label=\"Preview\">\n" \
  "            <div class=\"book__link__wrapper\">\n" \
  "            <div class=\"book__icon\" style=background-image:url(/ROOT%23%3F/catalog/v2/illustration/charlesray/?size=48)></div>\n" \
  "            <div class=\"book__header\">\n" \
  "                <div id=\"book__title\">Charles, Ray</div>\n" \
  "            </div>\n" \
  "            <div class=\"book__description\" title=\"Wikipedia articles about Ray Charles or why and when one should go to library\">Wikipedia articles about Ray Charles or why and when one should go to library</div>\n" \
  "            </div>\n" \
  "            </a>\n" \
  "            <div class=\"book__meta\">\n" \
  "              <div class=\"book__languageTag\" title=\"français\" aria-label=\"français\">fra</div>\n" \
  "              <div class=\"book__tags\"><div class=\"book__tags--wrapper\">\n" \
  "                  <span class=\"tag__link\" aria-label='unittest' title='unittest'>unittest</span>\n" \
  "                  <span class=\"tag__link\" aria-label='wikipedia' title='wikipedia'>wikipedia</span>\n" \
  "              </div>\n" \
  "              </div>\n" \
  "            </div>\n" \
  "            <div>\n" \
  "              <a class=\"book__download\" href=\"/ROOT%23%3F/nojs/download/zimfile%26other\">\n" \
  "                <img src=\"/ROOT%23%3F/skin/download-white.svg?cacheid=079ab989\">\n" \
  "                <span>Download</span>\n" \
  "              </a>\n" \
  "            </div>\n" \
  "        </div>\n"

#define RAY_CHARLES_BOOK_HTML \
  "        <div class=\"book__wrapper\">\n" \
  "            <a class=\"book__link\" href=\"/ROOT%23%3F/content/zimfile_raycharles\" title=\"Preview\" aria-label=\"Preview\">\n" \
  "            <div class=\"book__link__wrapper\">\n" \
  "            <div class=\"book__icon\" style=background-image:url(/ROOT%23%3F/catalog/v2/illustration/raycharles/?size=48)></div>\n" \
  "            <div class=\"book__header\">\n" \
  "                <div id=\"book__title\">Ray Charles</div>\n" \
  "            </div>\n" \
  "            <div class=\"book__description\" title=\"Wikipedia articles about Ray Charles (not all of them but near to what an average newborn may find more than enough)\">Wikipedia articles about Ray Charles (not all of them but near to what an average newborn may find more than enough)</div>\n" \
  "            </div>\n" \
  "            </a>\n" \
  "            <div class=\"book__meta\">\n" \
  "              <div class=\"book__languageTag\" title=\"English\" aria-label=\"English\">eng</div>\n" \
  "              <div class=\"book__tags\"><div class=\"book__tags--wrapper\">\n" \
  "                  <span class=\"tag__link\" aria-label='public_tag_without_a_value' title='public_tag_without_a_value'>public_tag_without_a_value</span>\n" \
  "                  <span class=\"tag__link\" aria-label='wikipedia' title='wikipedia'>wikipedia</span>\n" \
  "              </div>\n" \
  "              </div>\n" \
  "            </div>\n" \
  "            <div>\n" \
  "              <a class=\"book__download\" href=\"/ROOT%23%3F/nojs/download/zimfile_raycharles\">\n" \
  "                <img src=\"/ROOT%23%3F/skin/download-white.svg?cacheid=079ab989\">\n" \
  "                <span>Download</span>\n" \
  "              </a>\n" \
  "            </div>\n" \
  "        </div>\n"

#define RAY_CHARLES_UNCTZ_BOOK_HTML \
  "        <div class=\"book__wrapper\">\n" \
  "            <a class=\"book__link\" href=\"/ROOT%23%3F/content/zimfile_raycharles_uncategorized\" title=\"Preview\" aria-label=\"Preview\">\n" \
  "            <div class=\"book__link__wrapper\">\n" \
  "            <div class=\"book__icon\" style=background-image:url(/ROOT%23%3F/catalog/v2/illustration/raycharles_uncategorized/?size=48)></div>\n" \
  "            <div class=\"book__header\">\n" \
  "                <div id=\"book__title\">Ray (uncategorized) Charles</div>\n" \
  "            </div>\n" \
  "            <div class=\"book__description\" title=\"No category is assigned to this library entry (neither adj nor xor was considered a good option)\">No category is assigned to this library entry (neither adj nor xor was considered a good option)</div>\n" \
  "            </div>\n" \
  "            </a>\n" \
  "            <div class=\"book__meta\">\n" \
  "              <div class=\"book__languageTag\" title=\"русский,English\" aria-label=\"русский,English\">mul</div>\n" \
  "              <div class=\"book__tags\"><div class=\"book__tags--wrapper\">\n" \
  "                  <span class=\"tag__link\" aria-label='public_tag_with_a_value:value_of_a_public_tag' title='public_tag_with_a_value:value_of_a_public_tag'>public_tag_with_a_value:value_of_a_public_tag</span>\n" \
  "                  <span class=\"tag__link\" aria-label='wikipedia' title='wikipedia'>wikipedia</span>\n" \
  "              </div>\n" \
  "              </div>\n" \
  "            </div>\n" \
  "            <div>\n" \
  "              <a class=\"book__download\" href=\"/ROOT%23%3F/nojs/download/zimfile_raycharles_uncategorized\">\n" \
  "                <img src=\"/ROOT%23%3F/skin/download-white.svg?cacheid=079ab989\">\n" \
  "                <span>Download</span>\n" \
  "              </a>\n" \
  "            </div>\n" \
  "        </div>\n"

#define INACCESSIBLEZIMFILE_BOOK_HTML \
  "        <div class=\"book__wrapper\">\n" \
  "            <a class=\"book__link\" href=\"/ROOT%23%3F/content/nosuchzimfile\" title=\"Preview\" aria-label=\"Preview\">\n" \
  "            <div class=\"book__link__wrapper\">\n" \
  "            <div class=\"book__icon\" style=background-image:url(/ROOT%23%3F/catalog/v2/illustration/inaccessiblezim/?size=48)></div>\n" \
  "            <div class=\"book__header\">\n" \
  "                <div id=\"book__title\">Catalog of all catalogs</div>\n" \
  "            </div>\n" \
  "            <div class=\"book__description\" title=\"Testing that running kiwix-serve without access to ZIM files doesn&apos;t lead to a catastrophe\">Testing that running kiwix-serve without access to ZIM files doesn&apos;t lead to a catastrophe</div>\n" \
  "            </div>\n" \
  "            </a>\n" \
  "            <div class=\"book__meta\">\n" \
  "              <div class=\"book__languageTag\" title=\"català\" aria-label=\"català\">cat</div>\n" \
  "              <div class=\"book__tags\"><div class=\"book__tags--wrapper\">\n" \
  "                  <span class=\"tag__link\" aria-label='unittest' title='unittest'>unittest</span>\n" \
  "              </div>\n" \
  "              </div>\n" \
  "            </div>\n" \
  "            <div>\n" \
  "              <a class=\"book__download\" href=\"/ROOT%23%3F/nojs/download/nosuchzimfile\">\n" \
  "                <img src=\"/ROOT%23%3F/skin/download-white.svg?cacheid=079ab989\">\n" \
  "                <span>Download</span>\n" \
  "              </a>\n" \
  "            </div>\n" \
  "        </div>\n"

#define FINAL_HTML_TEXT \
  "        </div>\n" \
  "    </div>\n" \
  "    <div id=\"kiwixfooter\" class=\"kiwixfooter\">Powered by&nbsp;<a href=\"https://kiwix.org\">Kiwix</a></div>\n" \
  "    </body>\n" \
  "</html>\n"

#define FILTERS_HTML(SELECTED_ENG) \
  "      <div class=\"kiwixNav__filters\">\n" \
  "        <div class=\"kiwixNav__select\">\n" \
  "          <select name=\"lang\" id=\"languageFilter\" class='kiwixNav__kiwixFilter filter' form=\"kiwixSearchForm\">\n" \
  "            <option value=\"\" selected>All languages</option>\n" \
  "            <option value=\"cat\">català</option>\n" \
  "            <option value=\"eng\"" SELECTED_ENG ">English</option>\n" \
  "            <option value=\"fra\">français</option>\n" \
  "            <option value=\"rus\">русский</option>\n" \
  "          </select>\n" \
  "        </div>\n" \
  "        <div class=\"kiwixNav__select\">\n" \
  "          <select name=\"category\" id=\"categoryFilter\" class='kiwixNav__kiwixFilter filter' form=\"kiwixSearchForm\">\n" \
  "            <option value=\"\">All categories</option>\n" \
  "            <option value=\"cats\">Cats</option>\n" \
  "            <option value=\"jazz\">Jazz</option>\n" \
  "            <option value=\"wikipedia\">Wikipedia</option>\n" \
  "          </select>\n" \
  "        </div>\n" \
  "      </div>\n" \
  "      <form id='kiwixSearchForm' class='kiwixNav__SearchForm' action=\"/ROOT%23%3F/nojs\">\n" \
  "        <input type=\"text\" name=\"q\" accesskey=\"s\" placeholder=\"Search\" id=\"searchFilter\" class='kiwixSearch filter' value=\"\">\n" \
  "        <input type=\"submit\" class=\"kiwixButton kiwixButtonHover\" value=\"Search\"/>\n" \
  "      </form>\n" \
  "    </div>\n"

#define HOME_BODY_TEXT(X) \
  "    <div class=\"kiwixHomeBody\">\n" \
  "        \n" \
  "        <div class=\"book__list\">\n" \
  "        <h3 class=\"kiwixHomeBody__results\">" X " book(s)</h3>\n"

#define HOME_BODY_0_RESULTS \
  "    <div class=\"kiwixHomeBody\">\n" \
  "        <style>\n" \
  "          .book__list {\n" \
  "            display: none;\n" \
  "          }\n" \
  "          .kiwixHomeBody {\n" \
  "            justify-content: center;\n" \
  "          }\n" \
  "          .noResults {\n" \
  "            font-size: 16px;\n" \
  "            font-family: roboto;\n" \
  "          }\n" \
  "        </style>\n" \
  "        <div class=\"noResults\">\n" \
  "          No result. Would you like to <a href=\"?lang=\">reset filter</a>?\n" \
  "        </div>\n" \
  "        </style>\n" \
  "        <div class=\"book__list\">\n" \
  "        <h3 class=\"kiwixHomeBody__results\">0 book(s)</h3>\n" \
  "        \n"

#define RAY_CHARLES_UNCTZ_DOWNLOAD \
  "<!DOCTYPE html>\n" \
  "<html lang=\"en\">\n" \
  "<head>\n" \
  "    <meta charset=\"UTF-8\">\n" \
  "    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n" \
  "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n" \
  "    <title>Download book</title>\n" \
  "</head>\n" \
  "<style>\n" \
  "    .downloadLinksTitle {\n" \
  "        text-align: center;\n" \
  "        font-size: 32px;\n" \
  "        margin-bottom: 8px;\n" \
  "    }\n" \
  "</style>\n" \
  "<body>\n" \
  "    <div class=\"downloadLinksTitle\">\n" \
  "        Download links for <b><i>Ray (uncategorized) Charles</i></b>\n" \
  "    </div>\n" \
  "    <a href=\"https://github.com/kiwix/libkiwix/raw/master/test/data/zimfile_raycharles_uncategorized.zim\" download>\n" \
  "        <div>Direct</div>\n" \
  "    </a>\n" \
  "    <a href=\"https://github.com/kiwix/libkiwix/raw/master/test/data/zimfile_raycharles_uncategorized.zim.sha256\" download>\n" \
  "        <div>SHA-256 checksum</div>\n" \
  "    </a>\n" \
  "    <a href=\"https://github.com/kiwix/libkiwix/raw/master/test/data/zimfile_raycharles_uncategorized.zim.magnet\" target=\"_blank\">\n" \
  "        <div>Magnet link</div>\n" \
  "    </a>\n" \
  "    <a href=\"https://github.com/kiwix/libkiwix/raw/master/test/data/zimfile_raycharles_uncategorized.zim.torrent\" download>\n" \
  "        <div>BitTorrent</div>\n" \
  "    </a>\n" \
  "</body>\n" \
  "</html>"

