#include "../src/server/i18n_utils.h"
#include "gtest/gtest.h"

using namespace kiwix;

TEST(ParameterizedMessage, parameterlessMessages)
{
  {
    const ParameterizedMessage msg("404-page-title", {});

    EXPECT_EQ(msg.getText("en"),   "Content not found");
    EXPECT_EQ(msg.getText("test"), "[I18N TESTING] Not Found - Try Again");
  }

  {
    // Make sure that msgId influences the result of getText()
    const ParameterizedMessage msg("random-page-button-text", {});

    EXPECT_EQ(msg.getText("en"),   "Go to a randomly selected page");
    EXPECT_EQ(msg.getText("test"), "[I18N TESTING] I am tired of determinism");
  }

  {
    // Demonstrate that unwanted parameters are silently ignored
    const ParameterizedMessage msg("404-page-title", {{"abc", "xyz"}});

    EXPECT_EQ(msg.getText("en"),   "Content not found");
    EXPECT_EQ(msg.getText("test"), "[I18N TESTING] Not Found - Try Again");
  }
}

TEST(ParameterizedMessage, messagesWithParameters)
{
  {
    const ParameterizedMessage msg("filter-by-tag",
                                   {{"TAG", "scifi"}}
    );

    EXPECT_EQ(msg.getText("en"),   "Filter by tag \"scifi\"");
    EXPECT_EQ(msg.getText("test"), "Filter [I18N] by [TESTING] tag \"scifi\"");
  }

  {
    // Omitting expected parameters amounts to using empty values for them
    const ParameterizedMessage msg("filter-by-tag", {});

    EXPECT_EQ(msg.getText("en"),   "Filter by tag \"\"");
    EXPECT_EQ(msg.getText("test"), "Filter [I18N] by [TESTING] tag \"\"");
  }
}

TEST(I18n, translateBookCategory)
{

  EXPECT_EQ(translateBookCategory("en",   "ted"), "Ted");
  EXPECT_EQ(translateBookCategory("test", "ted"), "[I18N] Ted [TESTING]");

  EXPECT_EQ(translateBookCategory("en",   "stack_exchange"), "Stack Exchange");
  EXPECT_EQ(translateBookCategory("test", "stack_exchange"), "[I18N] Stack Exchange [TESTING]");

  // unknown categories are simply not translated
  EXPECT_EQ(translateBookCategory("en", "Qwerty"), "Qwerty");
  EXPECT_EQ(translateBookCategory("test", "Qwerty"), "Qwerty");
}
