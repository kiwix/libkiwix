DEFINE_EXPECTED_OUTPUT(CATALOG_PARTIAL_ENTRIES,
R"(<?xml version="1.0" encoding="UTF-8"?>
<feed xmlns="http://www.w3.org/2005/Atom"
      xmlns:dc="http://purl.org/dc/terms/"
      xmlns:opds="https://specs.opds.io/opds-1.2"
      xmlns:opensearch="http://a9.com/-/spec/opensearch/1.1/">
  <id>12345678-90ab-cdef-1234-567890abcdef</id>

  <link rel="self"
        href="/ROOT%23%3F/catalog/v3/partial_entries"
        type="application/atom+xml;profile=opds-catalog;kind=acquisition"/>
  <link rel="start"
        href="/ROOT%23%3F/catalog/v3/root.xml"
        type="application/atom+xml;profile=opds-catalog;kind=navigation"/>
  <link rel="up"
        href="/ROOT%23%3F/catalog/v3/root.xml"
        type="application/atom+xml;profile=opds-catalog;kind=navigation"/>

  <title>All Entries</title>
  <updated>YYYY-MM-DDThh:mm:ssZ</updated>

  <entry>
    <id>urn:uuid:charlesray</id>
    <title>Charles, Ray</title>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <link rel="alternate"
          href="/ROOT%23%3F/catalog/v2/entry/charlesray"
          type="application/atom+xml;type=entry;profile=opds-catalog"/>
  </entry>
  <entry>
    <id>urn:uuid:raycharles</id>
    <title>Ray Charles</title>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <link rel="alternate"
          href="/ROOT%23%3F/catalog/v2/entry/raycharles"
          type="application/atom+xml;type=entry;profile=opds-catalog"/>
  </entry>
  <entry>
    <id>urn:uuid:raycharles_uncategorized</id>
    <title>Ray (uncategorized) Charles</title>
    <updated>YYYY-MM-DDThh:mm:ssZ</updated>
    <link rel="alternate"
          href="/ROOT%23%3F/catalog/v2/entry/raycharles_uncategorized"
          type="application/atom+xml;type=entry;profile=opds-catalog"/>
  </entry>
</feed>
)");
