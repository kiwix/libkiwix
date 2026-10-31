#include "../src/server/response.h"
#include "gtest/gtest.h"

#include "../src/server/request_context.h"

#include <algorithm>

namespace
{

using namespace kiwix;

RequestContext makeHttpGetRequest(const std::string& url,
                                  const RequestContext::NameValuePairs& headers,
                                  const RequestContext::NameValuePairs& queryArgs)
{
  return RequestContext(url, 0, "GET", "1.1", headers, queryArgs);
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

TEST(HTTPErrorResponse, shouldNotAllowBreakingOutOfTheScriptTag) {
  const RequestContext req = makeHttpGetRequest("/asdf", {}, {});
  HTTPErrorResponse errResp(req, MHD_HTTP_NOT_FOUND,
                            "404-page-title",
                            "404-page-heading",
                            "/css/error.css",
                            /*includeKiwixResponseData=*/true);

  errResp += ParameterizedMessage("suggest-search",
                              {
                                { "PATTERN",    "</SCRIPT><script>alert(1)</script>" },
                                { "SEARCH_URL", "/search?q=asdf" }
             });

  std::string content = getResponseContent(errResp);
  std::transform(content.begin(), content.end(), content.begin(), ::tolower);

  // The only "</script>" in the page must be the one closing the inline
  // script tag. The HTML parser matches that sequence case-insensitively.
  ASSERT_NE(content.find("</script>"), std::string::npos);
  EXPECT_EQ(content.find("</script>"), content.rfind("</script>")) << content;
}

TEST(ETag, defaultConstructed) {
  const ETag etag;
  EXPECT_FALSE(bool(etag));
  EXPECT_EQ(etag.get_etag(), "");
  EXPECT_FALSE(etag.get_option(ETag::ZIM_CONTENT));
  EXPECT_FALSE(etag.get_option(ETag::COMPRESSED_CONTENT));
}

TEST(ETag, setBodyAndGetETag) {
  // Diverse body formats: UUID, single-char, pure numeric, and hyphenated/underscored IDs
  ETag etag;
  etag.set_body("6f1d19d0-633f-087b-fb55-7ac324ff9baf");
  EXPECT_TRUE(bool(etag));
  EXPECT_EQ(etag.get_etag(), R"("6f1d19d0-633f-087b-fb55-7ac324ff9baf/")");

  etag.set_body("x");
  EXPECT_TRUE(bool(etag));
  EXPECT_EQ(etag.get_etag(), R"("x/")");

  etag.set_body("1234567890");
  EXPECT_TRUE(bool(etag));
  EXPECT_EQ(etag.get_etag(), R"("1234567890/")");

  etag.set_body("archive_2026-content");
  EXPECT_TRUE(bool(etag));
  EXPECT_EQ(etag.get_etag(), R"("archive_2026-content/")");
}

TEST(ETag, optionsManagement) {
  ETag etag;
  etag.set_body("6f1d19d0-633f-087b-fb55-7ac324ff9baf");

  // Options are added and automatically kept sorted ("Zz")
  etag.set_option(ETag::COMPRESSED_CONTENT);
  EXPECT_TRUE(etag.get_option(ETag::COMPRESSED_CONTENT));
  EXPECT_FALSE(etag.get_option(ETag::ZIM_CONTENT));
  EXPECT_EQ(etag.get_etag(), R"("6f1d19d0-633f-087b-fb55-7ac324ff9baf/z")");

  etag.set_option(ETag::ZIM_CONTENT);
  EXPECT_TRUE(etag.get_option(ETag::ZIM_CONTENT));
  EXPECT_TRUE(etag.get_option(ETag::COMPRESSED_CONTENT));
  EXPECT_EQ(etag.get_etag(), R"("6f1d19d0-633f-087b-fb55-7ac324ff9baf/Zz")");

  // Duplicate option calls must not duplicate characters
  etag.set_option(ETag::ZIM_CONTENT);
  EXPECT_EQ(etag.get_etag(), R"("6f1d19d0-633f-087b-fb55-7ac324ff9baf/Zz")");
}

TEST(ETag, matchStrongETag) {
  // Without options (using pure numeric string)
  auto etag = ETag::match(R"("1234567890/")", "1234567890");
  EXPECT_TRUE(bool(etag));
  EXPECT_EQ(etag.get_etag(), R"("1234567890/")");
  EXPECT_FALSE(etag.get_option(ETag::ZIM_CONTENT));
  EXPECT_FALSE(etag.get_option(ETag::COMPRESSED_CONTENT));

  // With single option (using UUID)
  etag = ETag::match(R"("6f1d19d0-633f-087b-fb55-7ac324ff9baf/Z")", "6f1d19d0-633f-087b-fb55-7ac324ff9baf");
  EXPECT_TRUE(bool(etag));
  EXPECT_EQ(etag.get_etag(), R"("6f1d19d0-633f-087b-fb55-7ac324ff9baf/Z")");
  EXPECT_TRUE(etag.get_option(ETag::ZIM_CONTENT));
  EXPECT_FALSE(etag.get_option(ETag::COMPRESSED_CONTENT));

  // With multiple options (using hyphenated/underscored name)
  etag = ETag::match(R"("archive_2026-content/Zz")", "archive_2026-content");
  EXPECT_TRUE(bool(etag));
  EXPECT_EQ(etag.get_etag(), R"("archive_2026-content/Zz")");
  EXPECT_TRUE(etag.get_option(ETag::ZIM_CONTENT));
  EXPECT_TRUE(etag.get_option(ETag::COMPRESSED_CONTENT));

  // Non-matching body
  EXPECT_FALSE(bool(ETag::match(R"("other_body/Z")", "expected_body")));
}

TEST(ETag, matchWeakETag) {
  // Without options (using alphabetic string)
  auto etag = ETag::match(R"(W/"abcdefghijklmn/")", "abcdefghijklmn");
  EXPECT_TRUE(bool(etag));
  EXPECT_EQ(etag.get_etag(), R"("abcdefghijklmn/")");

  // With option (using UUID)
  etag = ETag::match(R"(W/"7c9e6679-7425-40de-944b-e07fc1f90ae7/Z")", "7c9e6679-7425-40de-944b-e07fc1f90ae7");
  EXPECT_TRUE(bool(etag));
  EXPECT_EQ(etag.get_etag(), R"("7c9e6679-7425-40de-944b-e07fc1f90ae7/Z")");
  EXPECT_TRUE(etag.get_option(ETag::ZIM_CONTENT));

  // With multiple options (using alphanumeric ID)
  etag = ETag::match(R"(W/"doc-entry-42/Zz")", "doc-entry-42");
  EXPECT_TRUE(bool(etag));
  EXPECT_EQ(etag.get_etag(), R"("doc-entry-42/Zz")");
  EXPECT_TRUE(etag.get_option(ETag::ZIM_CONTENT));
  EXPECT_TRUE(etag.get_option(ETag::COMPRESSED_CONTENT));

  // Non-matching body
  EXPECT_FALSE(bool(ETag::match(R"(W/"weak-tag-1/Z")", "weak-tag-2")));
}

TEST(ETag, matchInList) {
  // Matching at the beginning of the list
  auto etag = ETag::match(
      R"(W/"first-uuid-1111/Zz", "second-uuid-2222/Z", "third-uuid-3333/")",
      "first-uuid-1111");
  EXPECT_TRUE(bool(etag));
  EXPECT_EQ(etag.get_etag(), R"("first-uuid-1111/Zz")");
  EXPECT_TRUE(etag.get_option(ETag::ZIM_CONTENT));
  EXPECT_TRUE(etag.get_option(ETag::COMPRESSED_CONTENT));

  // Matching in the middle of the list
  etag = ETag::match(
      R"("first-uuid-1111/Z", W/"second-uuid-2222/Zz", "third-uuid-3333/")",
      "second-uuid-2222");
  EXPECT_TRUE(bool(etag));
  EXPECT_EQ(etag.get_etag(), R"("second-uuid-2222/Zz")");
  EXPECT_TRUE(etag.get_option(ETag::ZIM_CONTENT));
  EXPECT_TRUE(etag.get_option(ETag::COMPRESSED_CONTENT));

  // Matching at the end of the list
  etag = ETag::match(
      R"("first-uuid-1111/Z", "second-uuid-2222/", W/"third-uuid-3333/Zz")",
      "third-uuid-3333");
  EXPECT_TRUE(bool(etag));
  EXPECT_EQ(etag.get_etag(), R"("third-uuid-3333/Zz")");
  EXPECT_TRUE(etag.get_option(ETag::ZIM_CONTENT));
  EXPECT_TRUE(etag.get_option(ETag::COMPRESSED_CONTENT));

  // Matching single-element list (both start and end)
  etag = ETag::match(R"("only-uuid-4444/Z")", "only-uuid-4444");
  EXPECT_TRUE(bool(etag));
  EXPECT_EQ(etag.get_etag(), R"("only-uuid-4444/Z")");
  EXPECT_TRUE(etag.get_option(ETag::ZIM_CONTENT));

  // No match in list
  EXPECT_FALSE(bool(ETag::match(
      R"("first-uuid-1111/Z", W/"second-uuid-2222/Zz", "third-uuid-3333/")",
      "non-existent-uuid")));
}

TEST(ETag, matchMalformedOrInvalid) {
  // Empty, whitespace, or bare prefixes
  EXPECT_FALSE(bool(ETag::match("", "target")));
  EXPECT_FALSE(bool(ETag::match("   ", "target")));
  EXPECT_FALSE(bool(ETag::match("W", "target")));
  EXPECT_FALSE(bool(ETag::match("W/", "target")));
  EXPECT_FALSE(bool(ETag::match(R"(W/"")", "target")));

  // Prefixes other than W/ (case-sensitive check: lowercase "w/", other single letters, or multiple characters)
  EXPECT_FALSE(bool(ETag::match(R"(w/"target/")", "target")));
  EXPECT_FALSE(bool(ETag::match(R"(X/"target/")", "target")));
  EXPECT_FALSE(bool(ETag::match(R"(V/"target/")", "target")));
  EXPECT_FALSE(bool(ETag::match(R"(INVALID/"target/")", "target")));
  EXPECT_FALSE(bool(ETag::match(R"(W//"target/")", "target")));
  EXPECT_FALSE(bool(ETag::match(R"(/W/"target/")", "target")));

  // Commas and quotes
  EXPECT_FALSE(bool(ETag::match(",", "target")));
  EXPECT_FALSE(bool(ETag::match(R"(")", "target")));
  EXPECT_FALSE(bool(ETag::match(R"("")", "target")));
  EXPECT_FALSE(bool(ETag::match(R"("""")", "target")));

  // Missing slash
  EXPECT_FALSE(bool(ETag::match(R"("6f1d19d0-633f-087b-fb55-7ac324ff9baf")", "6f1d19d0-633f-087b-fb55-7ac324ff9baf")));
  EXPECT_FALSE(bool(ETag::match(R"(W/"6f1d19d0-633f-087b-fb55-7ac324ff9baf")", "6f1d19d0-633f-087b-fb55-7ac324ff9baf")));

  // Extra slashes or empty body
  EXPECT_FALSE(bool(ETag::match(R"("nested/path/to/resource/Z")", "nested/path/to/resource")));
  EXPECT_FALSE(bool(ETag::match(R"("12345//")", "12345")));
  EXPECT_FALSE(bool(ETag::match(R"("/Z")", "")));

  // Invalid or unsorted option characters
  EXPECT_FALSE(bool(ETag::match(R"("custom-id/X")", "custom-id")));
  EXPECT_FALSE(bool(ETag::match(R"("custom-id/1")", "custom-id")));
  EXPECT_FALSE(bool(ETag::match(R"("custom-id/zZ")", "custom-id")));
  EXPECT_FALSE(bool(ETag::match(R"("custom-id/ZZ")", "custom-id")));
}


