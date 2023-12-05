#include "../src/server/response.h"
#include "gtest/gtest.h"

#include "../src/server/request_context.h"

namespace
{

using namespace kiwix;

RequestContext makeHttpGetRequest(const std::string& url)
{
  return RequestContext(nullptr, "", url, "GET", "1.1");
}

std::string getResponseContent(const ContentResponseBlueprint& crb)
{
  return crb.generateResponseObject()->getContent();
}

} // unnamed namespace



TEST(HTTPErrorResponse, shouldBeInEnglishByDefault) {
  const RequestContext req = makeHttpGetRequest("/asdf");
  HTTPErrorResponse errResp(req, MHD_HTTP_NOT_FOUND,
                            "404-page-title",
                            "404-page-heading",
                            "/css/error.css");

  EXPECT_EQ(getResponseContent(errResp),
R"(<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <meta content="text/html;charset=UTF-8" http-equiv="content-type" />
    <title>Content not found</title>
    <link type="text/css" href="/css/error.css" rel="Stylesheet" />
  </head>
  <body>
    <h1>Not Found</h1>

  </body>
</html>
)");
}
