#include "../src/server/response.h"
#include "gtest/gtest.h"

#include "../src/server/request_context.h"

namespace
{

using namespace kiwix;

RequestContext makeHttpGetRequest(const std::string& url,
                                  const RequestContext::NameValuePairs& headers,
                                  const RequestContext::NameValuePairs& queryArgs)
{
  return RequestContext("", url, "GET", "1.1", headers, queryArgs);
}

std::string getResponseContent(const ContentResponseBlueprint& crb)
{
  return crb.generateResponseObject()->getContent();
}

} // unnamed namespace



TEST(HTTPErrorResponse, shouldBeInEnglishByDefault) {
  const RequestContext req = makeHttpGetRequest("/asdf", {}, {});
  HTTPErrorResponse errResp(req, MHD_HTTP_NOT_FOUND,
                            "404-page-title",
                            "404-page-heading",
                            "/css/error.css",
                            /*includeKiwixResponseData=*/true);

  errResp += ParameterizedMessage("suggest-search",
                              {
                                { "PATTERN",    "asdf"   },
                                { "SEARCH_URL", "/search?q=asdf" }
             });

  EXPECT_EQ(getResponseContent(errResp),
R"(<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <meta content="text/html;charset=UTF-8" http-equiv="content-type" />
    <title>Content not found</title>
    <link type="text/css" href="/css/error.css" rel="Stylesheet" />
    <script>
      const KIWIX_RESPONSE_TEMPLATE = "<!DOCTYPE html>\n<html xmlns=\"http://www.w3.org/1999/xhtml\">\n  <head>\n    <meta content=\"text/html;charset=UTF-8\" http-equiv=\"content-type\" />\n    <title>{{PAGE_TITLE}}</title>\n{{#CSS_URL}}\n    <link type=\"text/css\" href=\"{{{CSS_URL}}}\" rel=\"Stylesheet\" />\n{{/CSS_URL}}{{#KIWIX_RESPONSE_DATA}}    <script>\n      const KIWIX_RESPONSE_TEMPLATE = \"{{{KIWIX_RESPONSE_TEMPLATE}}}\";\n      const KIWIX_RESPONSE_DATA = {{{KIWIX_RESPONSE_DATA}}};\n    </script>{{/KIWIX_RESPONSE_DATA}}\n  </head>\n  <body>\n    <h1>{{PAGE_HEADING}}</h1>\n{{#details}}\n    <p>\n      {{{p}}}\n    </p>\n{{/details}}\n  </body>\n</html>\n";
      const KIWIX_RESPONSE_DATA = { "CSS_URL" : "/css/error.css", "PAGE_HEADING" : { "msgid" : "404-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "404-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "suggest-search", "params" : { "PATTERN" : "asdf", "SEARCH_URL" : "/search?q=asdf" } } } ] };
    </script>
  </head>
  <body>
    <h1>Not Found</h1>
    <p>
      Make a full text search for <a href="/search?q=asdf">asdf</a>
    </p>
  </body>
</html>
)");
}

TEST(HTTPErrorResponse, shouldBeTranslatable) {
  const RequestContext req = makeHttpGetRequest("/asdf",
                              /* headers */     {},
                              /* query args */  {{"userlang", "test"}}
  );

  HTTPErrorResponse errResp(req, MHD_HTTP_NOT_FOUND,
                            "404-page-title",
                            "404-page-heading",
                            "/css/error.css",
                            /*includeKiwixResponseData=*/true);

  errResp += ParameterizedMessage("suggest-search",
                              {
                                { "PATTERN",    "asdf"   },
                                { "SEARCH_URL", "/search?q=asdf" }
             });

  EXPECT_EQ(getResponseContent(errResp),
R"(<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <meta content="text/html;charset=UTF-8" http-equiv="content-type" />
    <title>[I18N TESTING] Not Found - Try Again</title>
    <link type="text/css" href="/css/error.css" rel="Stylesheet" />
    <script>
      const KIWIX_RESPONSE_TEMPLATE = "<!DOCTYPE html>\n<html xmlns=\"http://www.w3.org/1999/xhtml\">\n  <head>\n    <meta content=\"text/html;charset=UTF-8\" http-equiv=\"content-type\" />\n    <title>{{PAGE_TITLE}}</title>\n{{#CSS_URL}}\n    <link type=\"text/css\" href=\"{{{CSS_URL}}}\" rel=\"Stylesheet\" />\n{{/CSS_URL}}{{#KIWIX_RESPONSE_DATA}}    <script>\n      const KIWIX_RESPONSE_TEMPLATE = \"{{{KIWIX_RESPONSE_TEMPLATE}}}\";\n      const KIWIX_RESPONSE_DATA = {{{KIWIX_RESPONSE_DATA}}};\n    </script>{{/KIWIX_RESPONSE_DATA}}\n  </head>\n  <body>\n    <h1>{{PAGE_HEADING}}</h1>\n{{#details}}\n    <p>\n      {{{p}}}\n    </p>\n{{/details}}\n  </body>\n</html>\n";
      const KIWIX_RESPONSE_DATA = { "CSS_URL" : "/css/error.css", "PAGE_HEADING" : { "msgid" : "404-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "404-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "suggest-search", "params" : { "PATTERN" : "asdf", "SEARCH_URL" : "/search?q=asdf" } } } ] };
    </script>
  </head>
  <body>
    <h1>[I18N TESTING] Content not found, but at least the server is alive</h1>
    <p>
      [I18N TESTING] Make a full text search for <a href="/search?q=asdf">asdf</a>
    </p>
  </body>
</html>
)");
}
