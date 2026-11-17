#include "./registry.h"

ExpectedOutputRegistry::ExpectedOutputRegistry()
{
#define DEFINE_EXPECTED_OUTPUT(ID, CONTENT) \
  ExpectedOutputRegistry::set(#ID, CONTENT);

#include "./building_blocks/opds/library/all.inc"

#undef DEFINE_EXPECTED_OUTPUT
}
// Resolve Expected Output string via its identifier
#define EO(ID) "" + ExpectedOutputRegistry::get(#ID) + ""

////////////////////////////////////////////////////////////////////////////////
// Building blocks for OPDS output
////////////////////////////////////////////////////////////////////////////////

#define LIBRARY_ENTRIES_PREAMBLE                              \
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"            \
    "<feed xmlns=\"http://www.w3.org/2005/Atom\"\n"           \
    "      xmlns:dc=\"http://purl.org/dc/terms/\"\n"          \
    "      xmlns:kiwix=\"https://kiwix.org/namespace/opds\"\n" \
    "      xmlns:opds=\"https://specs.opds.io/opds-1.2\">\n"  \

