DEFINE_EXPECTED_OUTPUT(CATALOG_ROOT,
R"(<?xml version="1.0" encoding="UTF-8"?>
<feed xmlns="http://www.w3.org/2005/Atom"
      xmlns:opds="https://specs.opds.io/opds-1.2">
  <id>12345678-90ab-cdef-1234-567890abcdef</id>
  <link rel="self"
        href="/ROOT%23%3F/catalog/v2/root.xml"
        type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
  <link rel="start"
        href="/ROOT%23%3F/catalog/v2/root.xml"
        type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
  <link rel="search"
        href="/ROOT%23%3F/catalog/v2/searchdescription.xml"
        type="application/opensearchdescription+xml"/>
  <title>OPDS Catalog Root</title>
  <updated>YYYY-MM-DDThh:mm:ssZ</updated>

  <entry>
    <title>All entries</title>
    <link rel="subsection"
          href="/ROOT%23%3F/catalog/v2/entries"
          type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <id>12345678-90ab-cdef-1234-567890abcdef</id>
    <content type="text">All entries from this catalog.</content>
  </entry>
  <entry>
    <title>All entries (partial)</title>
    <link rel="subsection"
          href="/ROOT%23%3F/catalog/v2/partial_entries"
          type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <id>12345678-90ab-cdef-1234-567890abcdef</id>
    <content type="text">All entries from this catalog in partial format.</content>
  </entry>
  <entry>
    <title>List of categories</title>
    <link rel="subsection"
          href="/ROOT%23%3F/catalog/v2/categories"
          type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <id>12345678-90ab-cdef-1234-567890abcdef</id>
    <content type="text">List of all categories in this catalog.</content>
  </entry>
  <entry>
    <title>List of languages</title>
    <link rel="subsection"
          href="/ROOT%23%3F/catalog/v2/languages"
          type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <id>12345678-90ab-cdef-1234-567890abcdef</id>
    <content type="text">List of all languages in this catalog.</content>
  </entry>
</feed>
)");
