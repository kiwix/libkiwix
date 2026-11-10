#define CATALOG_ENTRY(UUID, TITLE, SUMMARY, LANG, NAME, CATEGORY, TAGS, EXTRA_LINK, CONTENT_NAME, FILE_NAME, LENGTH) \
    "  <entry>\n"                                                       \
    "    <id>urn:uuid:" UUID "</id>\n"                                \
    "    <title>" TITLE "</title>\n"                                 \
    "    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n"                     \
    "    <summary>" SUMMARY "</summary>\n"     \
    "    <language>" LANG "</language>\n"                                    \
    "    <name>" NAME "</name>\n"                       \
    "    <flavour></flavour>\n"                                         \
    "    <category>" CATEGORY "</category>\n"                                   \
    "    <tags>" TAGS "</tags>\n" \
    "    <articleCount>284</articleCount>\n"                            \
    "    <mediaCount>2</mediaCount>\n"                                  \
    EXTRA_LINK "    <link type=\"text/html\" href=\"/ROOT%23%3F/content/" CONTENT_NAME "\" />\n"               \
    "    <author>\n"                                                    \
    "      <name>Wikipedia</name>\n"                                    \
    "    </author>\n"                                                   \
    "    <publisher>\n"                                                 \
    "      <name>Kiwix</name>\n"                                        \
    "    </publisher>\n"                                                \
    "    <dc:issued>2020-03-31T00:00:00Z</dc:issued>\n"                 \
    "    <link rel=\"http://opds-spec.org/acquisition/open-access\" type=\"application/x-zim\" href=\"https://github.com/kiwix/libkiwix/raw/master/test/data/" FILE_NAME ".zim\" length=\"" LENGTH "\" />\n" \
    "  </entry>\n"


#define _CHARLES_RAY_CATALOG_ENTRY(CONTENT_NAME) CATALOG_ENTRY(  \
  "charlesray",   \
  "Charles, Ray", \
  "Wikipedia articles about Ray Charles or why and when one should go to library", \
  "fra", \
  "wikipedia_fr_ray_charles",\
  "jazz",\
  "unittest;wikipedia;_category:jazz;_pictures:no;_videos:no;_details:no;_ftindex:yes",\
  "", \
  CONTENT_NAME, \
  "zimfile%26other", \
  "569304" \
)

#define CHARLES_RAY_CATALOG_ENTRY           _CHARLES_RAY_CATALOG_ENTRY("zimfile%26other")
#define CHARLES_RAY_CATALOG_ENTRY_NO_MAPPER _CHARLES_RAY_CATALOG_ENTRY("charlesray")

#define _RAY_CHARLES_CATALOG_ENTRY(CONTENT_NAME) CATALOG_ENTRY(\
  "raycharles",\
  "Ray Charles",\
  "Wikipedia articles about Ray Charles (not all of them but near to what an average newborn may find more than enough)",\
  "eng",\
  "wikipedia_en_ray_charles",\
  "wikipedia",\
  "public_tag_without_a_value;_private_tag_without_a_value;wikipedia;_category:wikipedia;_pictures:no;_videos:no;_details:no;_ftindex:yes",\
  "    <link rel=\"http://opds-spec.org/image/thumbnail\"\n"          \
  "          href=\"/ROOT%23%3F/catalog/v2/illustration/raycharles/?size=48\"\n" \
  "          type=\"image/png;width=48;height=48;scale=1\"/>\n", \
  CONTENT_NAME, \
  "zimfile_raycharles", \
  "569304"\
)

#define RAY_CHARLES_CATALOG_ENTRY           _RAY_CHARLES_CATALOG_ENTRY("zimfile_raycharles")
#define RAY_CHARLES_CATALOG_ENTRY_NO_MAPPER _RAY_CHARLES_CATALOG_ENTRY("raycharles")

#define UNCATEGORIZED_RAY_CHARLES_CATALOG_ENTRY CATALOG_ENTRY(\
  "raycharles_uncategorized",\
  "Ray (uncategorized) Charles",\
  "No category is assigned to this library entry (neither adj nor xor was considered a good option)",\
  "rus,eng",\
  "wikipedia_ru_ray_charles",\
  "",\
  "public_tag_with_a_value:value_of_a_public_tag;_private_tag_with_a_value:value_of_a_private_tag;wikipedia;_pictures:no;_videos:no;_details:no",\
  "",\
  "zimfile_raycharles_uncategorized", \
  "zimfile_raycharles_uncategorized", \
  "125952"\
)

#define INACCESSIBLEZIMFILE_CATALOG_ENTRY \
"  <entry>\n" \
"    <id>urn:uuid:inaccessiblezim</id>\n" \
"    <title>Catalog of all catalogs</title>\n" \
"    <updated>YYYY-MM-DDThh:mm:ssZ</updated>\n" \
"    <summary>Testing that running kiwix-serve without access to ZIM files doesn&apos;t lead to a catastrophe</summary>\n" \
"    <language>cat</language>\n" \
"    <name>catalog_of_all_catalogs</name>\n" \
"    <flavour></flavour>\n" \
"    <category>cats</category>\n" \
"    <tags>unittest;_category:cats</tags>\n" \
"    <articleCount>12107</articleCount>\n" \
"    <mediaCount>8</mediaCount>\n" \
"    <link type=\"text/html\" href=\"/ROOT%23%3F/content/nosuchzimfile\" />\n" \
"    <author>\n" \
"      <name>Catherine of Catalonia</name>\n" \
"    </author>\n" \
"    <publisher>\n" \
"      <name>Caterpillar</name>\n" \
"    </publisher>\n" \
"    <dc:issued>2025-09-04T00:00:00Z</dc:issued>\n" \
"    <link rel=\"http://opds-spec.org/acquisition/open-access\" type=\"application/x-zim\" href=\"https://github.com/kiwix/libkiwix/raw/master/test/data/nosuchzimfile.zim\" length=\"20736925696\" />\n" \
"  </entry>\n"

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

