#include "gtest/gtest.h"
#include "../include/searcher.h"
#include "../include/reader.h"

namespace kiwix
{

TEST(Searcher, add_reader) {
  Reader reader1("./test/example.zim");
  Reader reader2("./test/example.zim");
  Reader reader3("./test/../test/example.zim");

  Searcher searcher;
  ASSERT_TRUE (searcher.add_reader(&reader1));
  ASSERT_FALSE(searcher.add_reader(&reader1));
  ASSERT_FALSE(searcher.add_reader(&reader2));
  ASSERT_FALSE(searcher.add_reader(&reader3));
}

TEST(Searcher, search) {
  Reader reader("./test/example.zim");

  Searcher searcher;
  searcher.add_reader(&reader);
  ASSERT_EQ(searcher.get_reader(0)->getTitle(), reader.getTitle());

  searcher.search("wiki", 0, 2);
  searcher.restart_search();
  ASSERT_EQ(searcher.getEstimatedResultCount(), (unsigned int)2);

  auto result = searcher.getNextResult();
  ASSERT_EQ(result->get_title(), "FreedomBox for Communities/Offline Wikipedia - Wikibooks, open books for an open world");
  result = searcher.getNextResult();
  ASSERT_EQ(result->get_title(), "Wikibooks");
}

TEST(Searcher, suggestion) {
  Reader reader("./test/zimfile.zim");

  Searcher searcher;
  searcher.add_reader(&reader);
  ASSERT_EQ(searcher.get_reader(0)->getTitle(), reader.getTitle());

  std::string query = "ray";
  searcher.suggestions(query, true);
  searcher.restart_search();

  auto result = searcher.getNextResult();
  ASSERT_EQ(result->get_title(), "Charles, Ray");
  ASSERT_EQ(result->get_url(), "A/Charles,_Ray");
  ASSERT_EQ(result->get_snippet(), "Charles, <b>Ray</b>");
  ASSERT_EQ(result->get_score(), 0);
  ASSERT_EQ(result->get_content(), "");
  ASSERT_EQ(result->get_size(), 0);
  ASSERT_EQ(result->get_wordCount(), 0);
  ASSERT_EQ(result->get_zimId(), "");

  result = searcher.getNextResult();
  ASSERT_EQ(result->get_title(), "Ray (film)");
}

TEST(Searcher, incrementalRange) {
  // Attempt to get 50 results in steps of 5
  zim::Archive archive("./test/zimfile.zim");
  zim::Searcher ftsearcher(archive);
  zim::Query query;
  query.setQuery("ray");
  auto search = ftsearcher.search(query);

  int suggCount = 0;
  for (int i = 0; i < 10; i++) {
    auto srs = search.getResults(i*5, 5); // get 5 results
    ASSERT_EQ(srs.size(), 5);
    suggCount += srs.size();
  }
  ASSERT_EQ(suggCount, 50);
}

}
