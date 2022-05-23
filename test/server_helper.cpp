
#include <mustache.hpp>
#include "gtest/gtest.h"
#include "../src/tools/stringTools.h"

namespace {
  struct ExpectedPage {
    ExpectedPage(const std::string& label, unsigned int start, bool current)
     : label(label),
       start(start),
       current(current)
    {}

    testing::AssertionResult isEqual(const kainjow::mustache::data& data) {
      if (!data.is_object()
         || data.get("label") == nullptr
         || data.get("start") == nullptr
         || data.get("current") == nullptr) {
        return testing::AssertionFailure() << "data is not a valid object";
      }

      if (data.get("label")->string_value() != label) {
        return testing::AssertionFailure() << data.get("label")->string_value()
                                           << " is not equal to "
                                           << label;
      }

      if (data.get("start")->string_value() != kiwix::to_string(start)) {
        return testing::AssertionFailure() << data.get("start")->string_value()
                                           << " is not equal to "
                                           << kiwix::to_string(start);
      }

      if (current && !data.get("current")->is_true()) {
        return testing::AssertionFailure() << "data is not true";
      }
      if (!current && !data.get("current")->is_false()) {
        return testing::AssertionFailure() << "data is not false";
      }
      return testing::AssertionSuccess();
    }

    std::string label;
    unsigned int start;
    bool current;
  };

  class ExpectedPages : public std::vector<ExpectedPage>
  {
    public:
      ExpectedPages(std::vector<ExpectedPage>&& v)
        : std::vector<ExpectedPage>(v)
      {}

      testing::AssertionResult isEqual(const kainjow::mustache::data& data) {
        if (empty()) {
          if (data.is_empty_list()) {
            return testing::AssertionSuccess();
          } else {
            return testing::AssertionFailure() << "data is not an empty list.";
          }
        }

        if (! data.is_non_empty_list()) {
          return testing::AssertionFailure() << "data is not a non empty list.";
        }
        auto& data_pages = data.list_value();
        if (size() != data_pages.size()) {
          return testing::AssertionFailure() << "data (size " << data_pages.size() << ")"
                                             << " and expected (size " << size() << ")"
                                             << " must have the same size";
        }
        auto it1 = begin();
        auto it2 = data_pages.begin();
        while (it1!=end()) {
          auto result = it1->isEqual(*it2);
          if(!result) {
            return result;
          }
          it1++; it2++;
        }
        return testing::AssertionSuccess();
      }
  };
}

namespace kiwix {
kainjow::mustache::data buildPagination(
  unsigned int pageLength,
  unsigned int resultsCount,
  unsigned int resultsStart
);
}

TEST(SearchRenderer, buildPagination) {
  {
    auto pagination = kiwix::buildPagination(10, 0, 0);
    ASSERT_EQ(pagination.get("itemsPerPage")->string_value(), "10");
    ASSERT_TRUE(pagination.get("hasPages")->is_false());
    ASSERT_TRUE(ExpectedPages({}).isEqual(*pagination.get("pages")));
  }
  {
    auto pagination = kiwix::buildPagination(10, 1, 0);
    ASSERT_EQ(pagination.get("itemsPerPage")->string_value(), "10");
    ASSERT_TRUE(pagination.get("hasPages")->is_false());
    ASSERT_TRUE(ExpectedPages({}).isEqual(*pagination.get("pages")));
  }

  {
    auto pagination = kiwix::buildPagination(10, 10, 0);
    ASSERT_EQ(pagination.get("itemsPerPage")->string_value(), "10");
    ASSERT_TRUE(pagination.get("hasPages")->is_false());
    ASSERT_TRUE(ExpectedPages({}).isEqual(*pagination.get("pages")));
  }

  {
    auto pagination = kiwix::buildPagination(10, 11, 0);
    ASSERT_EQ(pagination.get("itemsPerPage")->string_value(), "10");
    ASSERT_TRUE(pagination.get("hasPages")->is_true());
    ASSERT_TRUE(ExpectedPages({
      {"1", 0, true},
      {"2", 10, false},
    }).isEqual(*pagination.get("pages")));
  }

  {
    auto pagination = kiwix::buildPagination(10, 20, 0);
    ASSERT_EQ(pagination.get("itemsPerPage")->string_value(), "10");
    ASSERT_TRUE(pagination.get("hasPages")->is_true());
    ASSERT_TRUE(ExpectedPages({
      {"1", 0, true},
      {"2", 10, false},
    }).isEqual(*pagination.get("pages")));
  }

  {
    auto pagination = kiwix::buildPagination(10, 21, 0);
    ASSERT_EQ(pagination.get("itemsPerPage")->string_value(), "10");
    ASSERT_TRUE(pagination.get("hasPages")->is_true());
    ASSERT_TRUE(ExpectedPages({
      {"1", 0, true},
      {"2", 10, false},
      {"3", 20, false}
    }).isEqual(*pagination.get("pages")));
  }

  {
    auto pagination = kiwix::buildPagination(10, 21, 11);
    ASSERT_EQ(pagination.get("itemsPerPage")->string_value(), "10");
    ASSERT_TRUE(pagination.get("hasPages")->is_true());
    ASSERT_TRUE(ExpectedPages({
      {"1", 0, false},
      {"2", 10, true},
      {"3", 20, false}
    }).isEqual(*pagination.get("pages")));
  }

  {
    auto pagination = kiwix::buildPagination(10, 21, 21);
    ASSERT_EQ(pagination.get("itemsPerPage")->string_value(), "10");
    ASSERT_TRUE(pagination.get("hasPages")->is_true());
    ASSERT_TRUE(ExpectedPages({
      {"1", 0, false},
      {"2", 10, false},
      {"3", 20, true}
    }).isEqual(*pagination.get("pages")));
  }

  {
    auto pagination = kiwix::buildPagination(10, 200, 0);
    ASSERT_EQ(pagination.get("itemsPerPage")->string_value(), "10");
    ASSERT_TRUE(pagination.get("hasPages")->is_true());
    ASSERT_TRUE(ExpectedPages({
      {"1", 0, true},
      {"2", 10, false},
      {"3", 20, false},
      {"4", 30, false},
      {"5", 40, false},
      {"▶", 190, false}
    }).isEqual(*pagination.get("pages")));
  }

  {
    auto pagination = kiwix::buildPagination(10, 200, 105);
    ASSERT_EQ(pagination.get("itemsPerPage")->string_value(), "10");
    ASSERT_TRUE(pagination.get("hasPages")->is_true());
    ASSERT_TRUE(ExpectedPages({
      {"◀", 0, false},
      {"7", 60, false},
      {"8", 70, false},
      {"9", 80, false},
      {"10", 90, false},
      {"11", 100, true},
      {"12", 110, false},
      {"13", 120, false},
      {"14", 130, false},
      {"15", 140, false},
      {"▶", 190, false}
    }).isEqual(*pagination.get("pages")));
  }

  {
    auto pagination = kiwix::buildPagination(10, 200, 199);
    ASSERT_EQ(pagination.get("itemsPerPage")->string_value(), "10");
    ASSERT_TRUE(pagination.get("hasPages")->is_true());
    ASSERT_TRUE(ExpectedPages({
      {"◀", 0, false},
      {"16", 150, false},
      {"17", 160, false},
      {"18", 170, false},
      {"19", 180, false},
      {"20", 190, true}
    }).isEqual(*pagination.get("pages")));
  }
}
