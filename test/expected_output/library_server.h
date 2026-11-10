#include "./building_blocks/opds/catalog/v2/charles_ray.h"
#include "./building_blocks/opds/catalog/v2/charles_ray_no_namemapper.h"
#include "./building_blocks/opds/catalog/v2/ray_charles.h"
#include "./building_blocks/opds/catalog/v2/ray_charles_no_namemapper.h"
#include "./building_blocks/opds/catalog/v2/uncategorized_ray_charles.h"
#include "./building_blocks/opds/catalog/v2/inaccessiblezim.h"

#define CATALOG_V2_ENTRIES_PREAMBLE0(x)                       \
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"            \
    "<feed xmlns=\"http://www.w3.org/2005/Atom\"\n"           \
    "      xmlns:dc=\"http://purl.org/dc/terms/\"\n"          \
    "      xmlns:opds=\"https://specs.opds.io/opds-1.2\"\n"   \
    "      xmlns:opensearch=\"http://a9.com/-/spec/opensearch/1.1/\">\n"  \
    "  <id>12345678-90ab-cdef-1234-567890abcdef</id>\n"       \
    "\n"                                                      \
    "  <link rel=\"self\"\n"                                  \
    "        href=\"/ROOT%23%3F/catalog/v2/" x "\"\n"                    \
    "        type=\"application/atom+xml;profile=opds-catalog;kind=acquisition\"/>\n" \
    "  <link rel=\"start\"\n"                                 \
    "        href=\"/ROOT%23%3F/catalog/v2/root.xml\"\n"              \
    "        type=\"application/atom+xml;profile=opds-catalog;kind=navigation\"/>\n" \
    "  <link rel=\"up\"\n"                                    \
    "        href=\"/ROOT%23%3F/catalog/v2/root.xml\"\n"              \
    "        type=\"application/atom+xml;profile=opds-catalog;kind=navigation\"/>\n" \
    "\n"                                                      \

#define CATALOG_V2_ENTRIES_PREAMBLE(q) \
            CATALOG_V2_ENTRIES_PREAMBLE0("entries" q)

#define CATALOG_V2_PARTIAL_ENTRIES_PREAMBLE(q) \
            CATALOG_V2_ENTRIES_PREAMBLE0("partial_entries" q)

