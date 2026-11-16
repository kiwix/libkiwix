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
    "<feed xmlns=\"http://www.w3.org/2005/Atom\"\n"           \
    "      xmlns:dc=\"http://purl.org/dc/terms/\"\n"          \
    "      xmlns:opds=\"http://opds-spec.org/2010/catalog\">\n"  \

