#include "gtest/gtest.h"
#include "../include/searcher.h"
#include "../include/reader.h"

namespace kiwix
{

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

}