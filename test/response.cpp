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
      window.KIWIX_RESPONSE_TEMPLATE = "&lt;!DOCTYPE html&gt;\n&lt;html xmlns=&quot;http://www.w3.org/1999/xhtml&quot;&gt;\n  &lt;head&gt;\n    &lt;meta content=&quot;text/html;charset=UTF-8&quot; http-equiv=&quot;content-type&quot; /&gt;\n    &lt;title&gt;{{PAGE_TITLE}}&lt;/title&gt;\n{{#CSS_URL}}\n    &lt;link type=&quot;text/css&quot; href=&quot;{{{CSS_URL}}}&quot; rel=&quot;Stylesheet&quot; /&gt;\n{{/CSS_URL}}{{#KIWIX_RESPONSE_DATA}}    &lt;script&gt;\n      window.KIWIX_RESPONSE_TEMPLATE = &quot;{{KIWIX_RESPONSE_TEMPLATE}}&quot;;\n      window.KIWIX_RESPONSE_DATA = {{{KIWIX_RESPONSE_DATA}}};\n    &lt;/script&gt;{{/KIWIX_RESPONSE_DATA}}\n  &lt;/head&gt;\n  &lt;body&gt;\n    &lt;h1&gt;{{PAGE_HEADING}}&lt;/h1&gt;\n{{#details}}\n    &lt;p&gt;\n      {{{p}}}\n    &lt;/p&gt;\n{{/details}}\n  &lt;/body&gt;\n&lt;/html&gt;\n";
      window.KIWIX_RESPONSE_DATA = { "CSS_URL" : "/css/error.css", "PAGE_HEADING" : { "msgid" : "404-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "404-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "suggest-search", "params" : { "PATTERN" : "asdf", "SEARCH_URL" : "/search?q=asdf" } } } ] };
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
      window.KIWIX_RESPONSE_TEMPLATE = "&lt;!DOCTYPE html&gt;\n&lt;html xmlns=&quot;http://www.w3.org/1999/xhtml&quot;&gt;\n  &lt;head&gt;\n    &lt;meta content=&quot;text/html;charset=UTF-8&quot; http-equiv=&quot;content-type&quot; /&gt;\n    &lt;title&gt;{{PAGE_TITLE}}&lt;/title&gt;\n{{#CSS_URL}}\n    &lt;link type=&quot;text/css&quot; href=&quot;{{{CSS_URL}}}&quot; rel=&quot;Stylesheet&quot; /&gt;\n{{/CSS_URL}}{{#KIWIX_RESPONSE_DATA}}    &lt;script&gt;\n      window.KIWIX_RESPONSE_TEMPLATE = &quot;{{KIWIX_RESPONSE_TEMPLATE}}&quot;;\n      window.KIWIX_RESPONSE_DATA = {{{KIWIX_RESPONSE_DATA}}};\n    &lt;/script&gt;{{/KIWIX_RESPONSE_DATA}}\n  &lt;/head&gt;\n  &lt;body&gt;\n    &lt;h1&gt;{{PAGE_HEADING}}&lt;/h1&gt;\n{{#details}}\n    &lt;p&gt;\n      {{{p}}}\n    &lt;/p&gt;\n{{/details}}\n  &lt;/body&gt;\n&lt;/html&gt;\n";
      window.KIWIX_RESPONSE_DATA = { "CSS_URL" : "/css/error.css", "PAGE_HEADING" : { "msgid" : "404-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "404-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "suggest-search", "params" : { "PATTERN" : "asdf", "SEARCH_URL" : "/search?q=asdf" } } } ] };
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
