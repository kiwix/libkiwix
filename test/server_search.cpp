
#define CPPHTTPLIB_ZLIB_SUPPORT 1
#include "./httplib.h"
#include "gtest/gtest.h"
#include "../src/tools/stringTools.h"

#define SERVER_PORT 8101
#include "server_testing_tools.h"


std::string makeSearchResultsHtml(const std::string& pattern,
                                  const std::string& header,
                                  const std::string& results,
                                  const std::string& footer)
{
  const char SEARCHRESULTS_HTML_TEMPLATE[] = R"HTML(<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <meta content="text/html; charset=utf-8" http-equiv="content-type" />

    //EOLWHITESPACEMARKER
    <style type="text/css">
      body{
      color: #000000;
      font: small/normal Arial,Helvetica,Sans-Serif;
      margin-top: 0.5em;
      font-size: 90%;
      }

      a{
      color: #04c;
      }

      a:visited {
      color: #639
      }

      a:hover {
      text-decoration: underline
      }

      .header {
      font-size: 120%;
      }

      ul {
      margin:0;
      padding:0
      }

      .results {
      font-size: 110%;
      }

      .results li {
      list-style-type:none;
      margin-top: 0.5em;
      }

      .results a {
      font-size: 110%;
      text-decoration: underline
      }

      cite {
      font-style:normal;
      word-wrap:break-word;
      display: block;
      font-size: 100%;
      }

      .informations {
      color: #388222;
      font-size: 100%;
      }

      .book-title {
      color: #662200;
      font-size: 100%;
      }

      .footer {
      padding: 0;
      margin-top: 1em;
      width: 100%;
      float: left
      }

      .footer a, .footer span {
      display: block;
      padding: .3em .7em;
      margin: 0 .38em 0 0;
      text-align:center;
      text-decoration: none;
      }

      .footer a:hover {
      background: #ededed;
      }

      .footer ul, .footer li {
      list-style:none;
      margin: 0;
      padding: 0;
      }

      .footer li {
      float: left;
      }

      .selected {
      background: #ededed;
      }

    </style>
    <title>%USERLANGMARKER%Search: %PATTERN%</title>
  </head>
  <body bgcolor="white">
    <div class="header">
      %HEADER%
    </div>

    <div class="results">
      <ul>%RESULTS%
      </ul>
    </div>

    <div class="footer">%FOOTER%
    </div>
  </body>
</html>
)HTML";

  std::string html = removeEOLWhitespaceMarkers(SEARCHRESULTS_HTML_TEMPLATE);
  html = replace(html, "%PATTERN%", pattern);
  html = replace(html, "%HEADER%", header);
  html = replace(html, "%RESULTS%", results);
  html = replace(html, "%FOOTER%", footer);
  return html;
}

std::string makeSearchResultsXml(const std::string& header,
                                 const std::string& results)
{
  const char SEARCHRESULTS_XML_TEMPLATE[] = R"XML(<?xml version="1.0" encoding="UTF-8"?>
<rss version="2.0"
     xmlns:opensearch="http://a9.com/-/spec/opensearch/1.1/"
     xmlns:atom="http://www.w3.org/2005/Atom">
  <channel>
    %HEADER%%RESULTS%
  </channel>
</rss>
)XML";

  std::string html = removeEOLWhitespaceMarkers(SEARCHRESULTS_XML_TEMPLATE);
  html = replace(html, "%HEADER%", header);
  html = replace(html, "%RESULTS%", results);
  return html;
}

struct SearchResult
{
  std::string link;
  std::string title;
  std::string snippet;
  std::string bookTitle;
  std::string wordCount;

  std::string getHtml() const
  {
    return std::string()
      + "\n            <a href=\"" + link +  "\">\n"
      + "              " + title + "\n"
      + "            </a>\n"
      + "              <cite>" + snippet + "</cite>\n"
      + "              <div class=\"book-title\">from %USERLANGMARKER%" + bookTitle + "</div>\n"
      + "              <div class=\"informations\">" + wordCount + " %USERLANGMARKER%words</div>\n";
  }

  std::string getXml() const
  {
    return std::string()
      + "      <title>" + title + "</title>\n"
      + "      <link>" + replace(link, "'", "&apos;") + "</link>\n"
      + "        <description>" + snippet + "</description>\n"
      + "        <book>\n"
      + "          <title>" + bookTitle + "</title>\n"
      + "        </book>\n"
      + "        <wordCount>" + wordCount + "</wordCount>";
  }
};

#define SEARCH_RESULT(LINK, TITLE, SNIPPET, BOOK_TITLE, WORDCOUNT) \
        SearchResult{LINK, TITLE, SNIPPET, BOOK_TITLE, WORDCOUNT}


const SearchResult SEARCH_RESULT_FOR_TRAVEL_IN_RAYCHARLESZIM {
  /*link*/   "/ROOT%23%3F/content/zimfile/A/If_You_Go_Away",
  /*title*/  "If You Go Away",
  /*snippet*/    R"SNIPPET(...<b>Travel</b> On" (1965) "If You Go Away" (1966) "Walk Away" (1967) Damita Jo reached #10 on the Adult Contemporary chart and #68 on the Billboard Hot 100 in 1966 for her version of the song. Terry Jacks recorded a version of the song which was released as a single in 1974 and reached #29 on the Adult Contemporary chart, #68 on the Billboard Hot 100, and went to #8 in the UK. The complex melody is partly derivative of classical music - the poignant "But if you stay..." passage comes from Franz Liszt's......)SNIPPET",
  /*bookTitle*/  "Ray Charles",
  /*wordCount*/  "204"
};


const SearchResult SEARCH_RESULT_FOR_TRAVEL_IN_EXAMPLEZIM {
  /*link*/   "/ROOT%23%3F/content/example/Wikibooks.html",
  /*title*/  "Wikibooks",
  /*snippet*/    R"SNIPPET(...<b>Travel</b> guide Wikidata Knowledge database Commons Media repository Meta Coordination MediaWiki MediaWiki software Phabricator MediaWiki bug tracker Wikimedia Labs MediaWiki development The Wikimedia Foundation is a non-profit organization that depends on your voluntarism and donations to operate. If you find Wikibooks or other projects hosted by the Wikimedia Foundation useful, please volunteer or make a donation. Your donations primarily helps to purchase server equipment, launch new projects......)SNIPPET",
  /*bookTitle*/  "Wikibooks",
  /*wordCount*/  "538"
};


const std::vector<SearchResult> LARGE_SEARCH_RESULTS = {
  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Genius_%2B_Soul_%3D_Jazz",
    /*title*/      "Genius + Soul = Jazz",
    /*snippet*/    R"SNIPPET(...Grammy Hall of Fame in 2011. It was re-issued in the UK, first in 1989 on the Castle Communications "Essential Records" label, and by Rhino Records in 1997 on a single CD together with Charles' 1970 My Kind of <b>Jazz</b>. In 2010, Concord Records released a deluxe edition comprising digitally remastered versions of Genius + Soul = <b>Jazz</b>, My Kind of <b>Jazz</b>, <b>Jazz</b> Number II, and My Kind of <b>Jazz</b> Part 3. Professional ratings Review scores Source Rating Allmusic link Warr.org link Encyclopedia of Popular Music...)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "242"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Jazz_Number_II",
    /*title*/      "Jazz Number II",
    /*snippet*/    R"SNIPPET(<b>Jazz</b> Number II <b>Jazz</b> Number II is a 1973 album by Ray Charles. It is a collection of <b>jazz</b>/soul instrumentals featuring Charles on piano backed by his Big Band. Professional ratings Review scores Source Rating Allmusic link <b>Jazz</b> Number II Studio album by Ray Charles Released January 1973 Recorded 1971-72 Studio Charles’ Tangerine/RPM Studios, Los Angeles, CA Genre Soul, <b>jazz</b> Length 39:02 Label Tangerine Producer Ray Charles Ray Charles chronology Through the Eyes of Love (1972) <b>Jazz</b> Number II......)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "87"
  ),

  SEARCH_RESULT(
   /*link*/       "/ROOT%23%3F/content/zimfile/A/My_Kind_of_Jazz_Part_3",
   /*title*/      "My Kind of Jazz Part 3",
   /*snippet*/    R"SNIPPET(My Kind of <b>Jazz</b> Part 3 My Kind of <b>Jazz</b> Part 3 is a 1975 album by Ray Charles released by Crossover Records. Concord Records re-issued the contents in digital form in 2009. Professional ratings Review scores Source Rating Allmusic link My Kind of <b>Jazz</b> Part 3 Studio album by Ray Charles Released October 1975 Recorded 1975 in Los Angeles, CA Genre Soul, <b>jazz</b> Length 38:13 Label Crossover Producer Ray Charles Ray Charles chronology Renaissance (1975) My Kind of <b>Jazz</b> Part 3 (1975) Live In Japan (1975)...)SNIPPET",
   /*bookTitle*/  "Ray Charles",
   /*wordCount*/  "88"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/My_Kind_of_Jazz",
    /*title*/      "My Kind of Jazz",
    /*snippet*/    R"SNIPPET(My Kind of <b>Jazz</b> My Kind of <b>Jazz</b> Studio album by Ray Charles Released April 1970 Recorded January 1-10, 1970 in Los Angeles, CA Genre <b>jazz</b> Length 30:20 Label Tangerine Producer Quincy Jones Ray Charles chronology Doing His Thing (1969) My Kind of <b>Jazz</b> (1970) Love Country Style (1970) Professional ratings Review scores Source Rating Allmusic link My Kind of <b>Jazz</b> is a 1970 album by Ray Charles....)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "69"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Hank_Crawford",
    /*title*/      "Hank Crawford",
    /*snippet*/    R"SNIPPET(...bop, <b>jazz</b>-funk, soul <b>jazz</b> alto saxophonist, arranger and songwriter. Crawford was musical director for Ray Charles before embarking on a solo career releasing many well-regarded albums on Atlantic, CTI and Milestone. Hank Crawford Background information Birth name Bennie Ross Crawford, Jr Born (1934-12-21)December 21, 1934 Memphis, Tennessee, U.S. Died January 29, 2009(2009-01-29) (aged 74) Memphis, Tennessee, U.S. Genres R&amp;B, Hard bop, <b>Jazz</b>-funk, Soul <b>jazz</b> Occupation(s) Saxophonist, Songwriter......)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "102"
),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Catchin'_Some_Rays%3A_The_Music_of_Ray_Charles",
    /*title*/      "Catchin&apos; Some Rays: The Music of Ray Charles",
    /*snippet*/    R"SNIPPET(...<b>jazz</b> singer Roseanna Vitro, released in August 1997 on the Telarc <b>Jazz</b> label. Catchin' Some Rays: The Music of Ray Charles Studio album by Roseanna Vitro Released August 1997 Recorded March 26, 1997 at Sound on Sound, NYC April 4,1997 at Quad Recording Studios, NYC Genre Vocal <b>jazz</b> Length 61:00 Label Telarc <b>Jazz</b> CD-83419 Producer Paul Wickliffe Roseanna Vitro chronology Passion Dance (1996) Catchin' Some Rays: The Music of Ray Charles (1997) The Time of My Life: Roseanna Vitro Sings the Songs of......)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "118"
),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/That's_What_I_Say%3A_John_Scofield_Plays_the_Music_of_Ray_Charles",
    /*title*/      "That&apos;s What I Say: John Scofield Plays the Music of Ray Charles",
    /*snippet*/    R"SNIPPET(That's What I Say: John Scofield Plays the Music of Ray Charles Studio album by John Scofield Released June 7, 2005 (2005-06-07) Recorded December 2004 Studio Avatar Studios, New York City Genre <b>Jazz</b> Length 65:21 Label Verve Producer Steve Jordan John Scofield chronology EnRoute: John Scofield Trio LIVE (2004) That's What I Say: John Scofield Plays the Music of Ray Charles (2005) Out Louder (2006) Professional ratings Review scores Source Rating Allmusic All About <b>Jazz</b> All About <b>Jazz</b>...)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "109"
),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Tribute_to_Uncle_Ray",
    /*title*/      "Tribute to Uncle Ray",
    /*snippet*/    R"SNIPPET(...Stevie Wonder" with the successful and popular Ray Charles who was also a blind African American musician. Like his debut, this album failed to generate hit singles as Motown struggled to find a sound to fit Wonder, who was just 12 when this album was released. Tribute to Uncle Ray Studio album by Little Stevie Wonder Released October 1962 Recorded 1962 Studio Studio A, Hitsville USA, Detroit Genre Soul, <b>jazz</b> Label Tamla Producer Henry Cosby, Clarence Paul Stevie Wonder chronology The <b>Jazz</b> Soul of Little Stevie (1962) Tribute to Uncle Ray (1962) Recorded Live: The 12 Year Old Genius (1963) Professional ratings Review scores Source Rating Allmusic...)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "165"
),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/The_Best_of_Ray_Charles",
    /*title*/      "The Best of Ray Charles",
    /*snippet*/    R"SNIPPET(The Best of Ray Charles The Best of Ray Charles is a compilation album released in 1970 on the Atlantic <b>Jazz</b> label, featuring previously released instrumental (non-vocal) tracks recorded by Ray Charles between November 1956 and November 1958. The Best of Ray Charles Greatest hits album by Ray Charles Released 1970 Genre R&amp;B, <b>Jazz</b> Length 34:06 Label Atlantic The instrumental, "Rockhouse" would later be covered, as "Ray's Rockhouse" (1985), by The Manhattan Transfer with lyrics by Jon Hendricks....)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "79"
),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/The_Genius_Hits_the_Road",
    /*title*/      "The Genius Hits the Road",
    /*snippet*/    R"SNIPPET(...a hit single, "Georgia on My Mind". The Genius Hits the Road Studio album by Ray Charles Released September 1960 Recorded March 25 and 29, 1960 in New York City Genre R&amp;B, blues, <b>jazz</b> Length 33:37 Label ABC-Paramount 335 Producer Sid Feller Ray Charles chronology Genius + Soul = <b>Jazz</b> (1961) The Genius Hits the Road (1960) Dedicated to You (1961) Singles from The Genius Hits the Road "Georgia on My Mind" Released: September 1960 Professional ratings Review scores Source Rating Allmusic Warr.org...)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "127"
),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Ray_Charles_at_Newport",
    /*title*/      "Ray Charles at Newport",
    /*snippet*/    R"SNIPPET(...Ray Charles at Newport is a 1958 live album of Ray Charles' July 5, 1958 performance at the Newport <b>Jazz</b> Festival. The detailed liner notes on the album were written by Kenneth Lee Karpe. All tracks from this Newport album, along with all tracks from his 1959 Herndon Stadium performance in Atlanta, were also released on the Atlantic compilation LP, Ray Charles Live. A later CD reissue of that compilation album included a previously unissued song from the 1958 Newport concert, "Swanee River Rock". Professional ratings Review scores Source Rating Allmusic link Discogs link Ray Charles at Newport Live album by Ray Charles Released November 1958 Recorded July 5, 1958 Venue Newport <b>Jazz</b> Festival, Newport, Rhode Island Genre R&amp;B Length 40:28 Label Atlantic Producer Tom Dowd (engineer) Ray Charles chronology The Great Ray Charles (1957) Ray Charles at Newport (1958) Yes Indeed! (1958) Re-issue cover 1987 re-issue/compilation...)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "152"
),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Here_We_Go_Again%3A_Celebrating_the_Genius_of_Ray_Charles",
    /*title*/      "Here We Go Again: Celebrating the Genius of Ray Charles",
    /*snippet*/    R"SNIPPET(...and <b>jazz</b> trumpeter Wynton Marsalis. It was recorded during concerts at the Rose Theater in New York City, on February 9 and 10, 2009. The album received mixed reviews, in which the instrumentation of Marsalis' orchestra was praised by the critics. Here We Go Again: Celebrating the Genius of Ray Charles Live album by Willie Nelson and Wynton Marsalis Released March 29, 2011 (2011-03-29) Recorded February 9 –10 2009 Venue Rose Theater, New York Genre <b>Jazz</b>, country Length 61:49 Label Blue Note......)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "167"
),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Confession_Blues",
    /*title*/      "Confession Blues",
    /*snippet*/    R"SNIPPET(...<b>jazz</b> Length 2:31 Label Down Beat Records Songwriter(s) R. C. Robinson (Ray Charles) Charles moved to Seattle in 1948, where he formed The McSon Trio with guitarist G. D. "Gossie" McKee and bass player Milton S. Garret. In late 1948, Jack Lauderdale of Down Beat Records heard Charles play at the Seattle <b>jazz</b> club, The Rocking Chair. The next day, Lauderdale took Charles and his trio to a Seattle recording studio where they recorded "Confession Blues" and "I Love You, I Love You". In February......)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "284"
),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Genius_Loves_Company",
    /*title*/      "Genius Loves Company",
    /*snippet*/    R"SNIPPET(...<b>jazz</b> and pop standards performed by Charles and several guest musicians, such as Natalie Cole, Elton John, James Taylor, Norah Jones, B.B. King, Gladys Knight, Diana Krall, Van Morrison, Willie Nelson and Bonnie Raitt. Genius Loves Company was the last album recorded and completed by Charles before his death in June 2004. Genius Loves Company Studio album by Ray Charles Released August 31, 2004 Recorded June 2003–March 2004 Genre Rhythm and blues, soul, country, blues, <b>jazz</b>, pop Length 54:03......)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "325"
),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Love_Country_Style",
    /*title*/      "Love Country Style",
    /*snippet*/    R"SNIPPET(Love Country Style Love Country Style is a studio album by Ray Charles released in June 1970 on Charles' Tangerine Records label. Love Country Style Studio album by Ray Charles Released June 1970 Genre R&amp;B Length 35:25 Label ABC/Tangerine Producer Joe Adams Ray Charles chronology My Kind of <b>Jazz</b> (1970) Love Country Style (1970) Volcanic Action of My Soul (1971) Professional ratings Review scores Source Rating Allmusic Christgau's Record Guide B...)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "72"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Doing_His_Thing",
    /*title*/      "Doing His Thing",
    /*snippet*/    R"SNIPPET(Doing His Thing Doing His Thing is a 1969 studio album by Ray Charles, released by Tangerine Records. The cover artwork was by Lafayette Chew. Doing His Thing Studio album by Ray Charles Released May 1969 Recorded RPM Studios, Los Angeles, California Genre R&amp;B, soul Length 32:33 Label ABC/Tangerine Producer Joe Adams Ray Charles chronology I'm All Yours Baby (1969) Doing His Thing (1969) My Kind of <b>Jazz</b> (1970)...)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "70"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/The_Inspiration_I_Feel",
    /*title*/      "The Inspiration I Feel",
    /*snippet*/    R"SNIPPET(The Inspiration I Feel The Inspiration I Feel is an album by flautist Herbie Mann featuring tunes associated with Ray Charles recorded in 1968 and released on the Atlantic label. The Inspiration I Feel Studio album by Herbie Mann Released 1968 Recorded May 6 &amp; 7, 1968 New York City Genre <b>Jazz</b> Length 34:28 Label Atlantic SD 1513 Producer Nesuhi Ertegun, Joel Dorn Herbie Mann chronology Windows Opened (1968) The Inspiration I Feel (1968) Memphis Underground (1968)...)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "78"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Milt_Turner",
    /*title*/      "Milt Turner",
    /*snippet*/    R"SNIPPET(...Turner After graduating from Pearl High School, he attended Tennessee State University, where he coincided with Hank Crawford, who he later recommended to join him in Ray Charles' band when he took over from William Peeples in the late 1950s. Milton Turner (1930-1993) was a <b>jazz</b> drummer. In 1962, he was a member of Phineas Newborn's trio with Leroy Vinnegar, on whose solo albums he would later appear, and in the early 1960s, Turner also recorded with Teddy Edwards. He never recorded as a leader....)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "87"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Rare_Genius",
    /*title*/      "Rare Genius",
    /*snippet*/    R"SNIPPET(...studio recordings and demos made in the 1970s, 1980s and 1990s together with some contemporary instrumental and backing vocal parts. Rare Genius: The Undiscovered Masters Remix album by Ray Charles Released 2010 Genre Soul Length 41:36 Label Concord Producer Ray Charles, John Burk Ray Charles chronology Ray Sings, Basie Swings (2006) Rare Genius: The Undiscovered Masters (2010) Professional ratings Review scores Source Rating Allmusic (link) PopMatters (link) All About <b>Jazz</b> (link) favorable...)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "91"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Tangerine_Records_(1962)",
    /*title*/      "Tangerine Records (1962)",
    /*snippet*/    R"SNIPPET(...in 1962. ABC-Paramount Records promoted and distributed it. Early singles labels were orange and later became black, red and white. Many of the later recordings are now sought after in "Northern Soul" circles. In 1973 Charles left ABC, closed Tangerine and started Crossover Records. Ray Charles Enterprises owns the catalog. Tangerine Records Parent company ABC-Paramount Records Founded 1962 Founder Ray Charles Defunct 1973 Distributor(s) ABC-Paramount Records Genre R&amp;B, soul music, <b>jazz</b> music...)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "87"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Ray_Sings%2C_Basie_Swings",
    /*title*/      "Ray Sings, Basie Swings",
    /*snippet*/    R"SNIPPET(...from 1973 with newly recorded instrumental tracks by the contemporary Count Basie Orchestra. Professional ratings Review scores Source Rating AllMusic Ray Sings, Basie Swings Compilation album by Ray Charles, Count Basie Orchestra Released October 3, 2006 (2006-10-03) Recorded Mid-1970s, February - May 2006 Studio Los Angeles Genre Soul, <b>jazz</b>, Swing Label Concord/Hear Music Producer Gregg Field Ray Charles chronology Genius &amp; Friends (2005) Ray Sings, Basie Swings (2006) Rare Genius: The Undiscovered Masters (2010)...)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "91"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/I_Remember_Brother_Ray",
    /*title*/      "I Remember Brother Ray",
    /*snippet*/    R"SNIPPET(...is an album by saxophonist David "Fathead" Newman, paying tribute to his bandleader and mentor Ray Charles, which was recorded in 2004 and released on the HighNote label the following year. I Remember Brother Ray Studio album by David "Fathead" Newman Released January 11, 2005 Recorded August 14, 2004 Studio Van Gelder Studio, Englewood Cliffs, NJ Genre <b>Jazz</b> Length 50:39 Label HighNote HCD 7135 Producer David "Fathead" Newman, Houston Person David "Fathead" Newman chronology Song for the New Man (2004) I Remember Brother Ray (2005) Cityscape (2006)...)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "96"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Light_Out_of_Darkness_(A_Tribute_to_Ray_Charles)",
    /*title*/      "Light Out of Darkness (A Tribute to Ray Charles)",
    /*snippet*/    R"SNIPPET(...to Ray Charles) is a 1993 studio album by Shirley Horn, recorded in tribute to Ray Charles. Light Out of Darkness (A Tribute to Ray Charles) Studio album by Shirley Horn Released 1993 Recorded April 30 and May 1–3, 1993, Clinton Recording Studios, New York City Genre Vocal <b>jazz</b> Length 62:53 Label Verve Producer Shirley Horn, Sheila Mathis, Richard Seidel, Lynn Butterer Shirley Horn chronology Here's to Life (1992) Light Out of Darkness (A Tribute to Ray Charles) (1993) I Love You, Paris (1994)...)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "100"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Soul_Meeting",
    /*title*/      "Soul Meeting",
    /*snippet*/    R"SNIPPET(...in 1957 and released in 1961 on Atlantic Records. The album was later re-issued together with the other Charles–Jackson recording, Soul Brothers, on a 2 CD compilation together with other 'bonus' tracks from the same recording sessions. Professional ratings Review scores Source Rating Down Beat (Original Lp release) AllMusic link Soul Meeting Studio album by Ray Charles, Milt Jackson Released 1961 Recorded April 10, 1958 Genre R&amp;B, <b>jazz</b> Length 37:43 Label Atlantic Producer Tom Dowd Ray Charles chronology The Genius Sings the Blues (1961) Soul Meeting (1961) The Genius After Hours (1961) Alternative cover compilation CD re-issue...)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "114"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Ray_Charles_in_Concert",
    /*title*/      "Ray Charles in Concert",
    /*snippet*/    R"SNIPPET(...between 1958 and 1975. In Concert Compilation album by Ray Charles Released 2003 Recorded Newport <b>Jazz</b> Festival (1958 July 5), Herndon Stadium Atlanta (1959 May 19), Sportpalast Berlin (1962 March 6), Shrine Auditorium Los Angeles (1964 September 20), Tokyo (1975 November 27) and Yokohama (1975 November 30) Genre R&amp;B, soul Length 2 hours Label Rhino Handmade Producer Nesuhi Ertegun (Newport), Zenas Sears (Atlanta), Norman Granz (Berlin), Sid Feller (Los Angeles) and Ray Charles (Tokyo / Yokohama)...)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "118"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Sticks_and_Stones_(Titus_Turner_song)",
    /*title*/      "Sticks and Stones (Titus Turner song)",
    /*snippet*/    R"SNIPPET(...in a 1960 version by Ray Charles, who added the Latin drum part. It was his first R&amp;B hit with ABC-Paramount, followed in 1961 with "Hit The Road Jack". The song was also covered by Jerry Lee Lewis, The Zombies, Wanda Jackson and The Kingsmen, as well as Joe Cocker on Mad Dogs and Englishmen, and Elvis Costello in 1994 on the extended play version of Kojak Variety. In 1997, <b>jazz</b> singer Roseanna Vitro included the tune in her tribute to Charles, Catchin’ Some Rays: The Music of Ray Charles....)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "113"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Do_the_Twist!_with_Ray_Charles",
    /*title*/      "Do the Twist! with Ray Charles",
    /*snippet*/    R"SNIPPET(...peaked at #11. Do the Twist! with Ray Charles Greatest hits album by Ray Charles Released 1961 Recorded 1954-1960 Genre R&amp;B, Soul, <b>Jazz</b> Length 32:39 Label Atlantic Ray Charles chronology The Genius Sings the Blues (1961) Do the Twist! with Ray Charles (1961) Soul Meeting (1961) Professional ratings Review scores Source Rating Allmusic (link) The Rolling Stone Record Guide In 1963, the album got a new cover and was renamed The Greatest Ray Charles. Track listing and catalog number (Atlantic 8054) remained the same....)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "120"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/The_Great_Ray_Charles",
    /*title*/      "The Great Ray Charles",
    /*snippet*/    R"SNIPPET(...<b>jazz</b> album. Later CD re-issues often include as a bonus, six of eight tracks from The Genius After Hours. The original cover was by Marvin Israel. Professional ratings Review scores Source Rating Allmusic The Great Ray Charles Studio album by Ray Charles Released August 1957 Recorded April 30 - November 26, 1956 in New York City Genre Bebop Length 37:37 Label Atlantic Producer Ahmet Ertegün, Jerry Wexler Ray Charles chronology Ray Charles (or, Hallelujah I Love Her So) (1957) The Great Ray......)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "127"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Ray_Charles_Live",
    /*title*/      "Ray Charles Live",
    /*snippet*/    R"SNIPPET(...<b>Jazz</b> Festival in 1958 and at Herndon Stadium in Atlanta in 1959, respectively). Later CD re-issues of this compilation include an additional, previously unreleased, track from the 1958 Newport concert, "Swanee River Rock." Live Live album by Ray Charles Released 1973 Recorded July 5, 1958 / May 28, 1959 Genre Soul, R&amp;B Length 71:55 Label Atlantic 503 Producer Nesuhi Ertegün / Zenas Sears Ray Charles chronology From the Pages of My Mind (1986) Live (1973) Just Between Us (1988) Professional......)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "133"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Blue_Funk_(Ray_Charles_song)",
    /*title*/      "Soul Brothers",
    /*snippet*/    R"SNIPPET(...on the original LP releases. Soul Brothers Studio album by Ray Charles, Milt Jackson Released June 1958 Recorded September 12, 1957 (Tracks 1-2) and April 10, 1958 (Tracks 3-7), in New York City Genre R&amp;B, <b>jazz</b> Length 38:42 Label Atlantic, Studio One Producer Nesuhi Ertegun Ray Charles chronology Yes Indeed! (1958) Soul Brothers (1958) What'd I Say (1959) alternate release cover compilation CD / re-issue Professional ratings Review scores Source Rating AllMusic Down Beat (Original Lp release)...)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "135"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Soul_Brothers",
    /*title*/      "Soul Brothers",
    /*snippet*/    R"SNIPPET(...and the eleventh studio album by Milt Jackson and released by Atlantic Records in 1958. The album was later re-issued in a 2 CD compilation together with the other Charles–Jackson album Soul Meeting and included additional tracks from the same recording sessions not present on the original LP releases. Soul Brothers Studio album by Ray Charles, Milt Jackson Released June 1958 Recorded September 12, 1957 (Tracks 1-2) and April 10, 1958 (Tracks 3-7), in New York City Genre R&amp;B, <b>jazz</b> Length 38:42 Label Atlantic, Studio One Producer Nesuhi Ertegun Ray Charles chronology Yes Indeed! (1958) Soul Brothers (1958) What'd I Say (1959) alternate release cover compilation CD / re-issue Professional ratings Review scores Source Rating AllMusic Down Beat (Original Lp release)...)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "135"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Ray_Charles_and_Betty_Carter",
    /*title*/      "Ray Charles and Betty Carter",
    /*snippet*/    R"SNIPPET(...Betty Carter Studio album by Ray Charles and Betty Carter Released August 1961 Recorded August 23, 1960 - June 14, 1961 Genre <b>Jazz</b> Length 41:38 Label ABC Producer Sid Feller Ray Charles chronology Dedicated to You (1961) Ray Charles and Betty Carter (1961) The Genius Sings the Blues (1961) Betty Carter chronology The Modern Sound of Betty Carter (1960) Ray Charles and Betty Carter (1961) 'Round Midnight (1962) Alternative cover / re-issue 1998 Rhino CD re-issue with Dedicated to You Professional ratings Review scores Source Rating Allmusic...)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "158"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Ingredients_in_a_Recipe_for_Soul",
    /*title*/      "Ingredients in a Recipe for Soul",
    /*snippet*/    R"SNIPPET(...6, 1960–April 28, 1963 Genre R&amp;B, soul, country soul, vocal <b>jazz</b> Label ABC 465 Producer Sid Feller Ray Charles chronology Modern Sounds in Country and Western Music, Vol. 2 (1962) Ingredients in a Recipe for Soul (1963) Sweet &amp; Sour Tears (1964) Alternative cover 1997 Rhino CD re-issue with Have a Smile with Me In 1990, the album was released on compact disc by DCC with four bonus tracks. In 1997, it was packaged together with 1964's Have a Smile with Me on a two-for-one CD reissue on Rhino with historical liner notes. Professional ratings Review scores......)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "162"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/The_Genius_Sings_the_Blues",
    /*title*/      "The Genius Sings the Blues",
    /*snippet*/    R"SNIPPET(...<b>jazz</b>, and southern R&amp;B. The photo for the album cover was taken by renowned photographer Lee Friedlander. The Genius Sings the Blues was reissued in 2003 by Rhino Entertainment with liner notes by Billy Taylor. The Genius Sings the Blues Compilation album by Ray Charles Released October 1961 Recorded 1952–1960 Genre Rhythm and blues, piano blues, soul Length 34:19 Label Atlantic SD-8052 Producer Ahmet Ertegün, Jerry Wexler Ray Charles chronology Ray Charles and Betty Carter (1961) The Genius......)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "162"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/The_Genius_of_Ray_Charles",
    /*title*/      "The Genius of Ray Charles",
    /*snippet*/    R"SNIPPET(...the sixth studio album by American recording artist Ray Charles, released in 1959 by Atlantic Records. The album eschewed the soul sound of his 1950s recordings, which fused <b>jazz</b>, gospel, and blues, for swinging pop with big band arrangements. It comprises a first half of big band songs and a second half of string-backed ballads. The Genius of Ray Charles sold fewer than 500,000 copies and charted at number 17 on the Billboard 200. "Let the Good Times Roll" and "Don't Let the Sun Catch You Cryin'" were released as singles in 1959. The Genius of Ray Charles Studio album by Ray Charles Released October 1959 Recorded May 6 and June 23, 1959 at 6 West Recording in New......)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "172"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Ray_Charles_in_Person",
    /*title*/      "Ray Charles in Person",
    /*snippet*/    R"SNIPPET(...night in Atlanta, Georgia at Morris Brown College's Herndon Stadium. All tracks from this album together with those from Ray Charles at Newport were also released on the 1987 Atlantic compilation CD, Ray Charles Live. Ray Charles: In Person Live album by Ray Charles Released July 1960 Recorded May 28, 1959 Genre R&amp;B Length 29:19 Label Atlantic Producer Harris Zenas Ray Charles chronology The Genius of Ray Charles (1959) Ray Charles: In Person (1960) Genius + Soul = <b>Jazz</b> (1961) Re-issue cover 1987 re-issue / compilation Professional ratings Review scores Source Rating Allmusic The album was recorded by the concert sponsor, radio station WAOK. The station's lead disk jockey, Zenas "Daddy" Sears, recorded the album from the audience using a single microphone. The album is noted for its technical excellence in balancing band, singer, and audience, and also for its documentation of the jazzy R&amp;B Ray Charles sound prior to his great crossover success....)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "176"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Don't_Let_the_Sun_Catch_You_Cryin'",
    /*title*/      "Don&apos;t Let the Sun Catch You Cryin&apos;",
    /*snippet*/    R"SNIPPET(...R&amp;B Sides" and No. 95 on the Billboard Hot 100. It was also recorded by Jackie DeShannon on her 1965 album This is Jackie De Shannon, Paul McCartney on his 1990 live album Tripping the Live Fantastic, Jex Saarelaht and Kate Ceberano on their album Open the Door - Live at Mietta's (1992) and <b>jazz</b> singer Roseanna Vitro on her 1997 album Catchin’ Some Rays: The Music of Ray Charles. Karin Krog and Steve Kuhn include it on their 2005 album, Together Again. Steve Alaimo released a version in 1963...)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "185"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/I_Don't_Need_No_Doctor",
    /*title*/      "I Don&apos;t Need No Doctor",
    /*snippet*/    R"SNIPPET(...<b>jazz</b> guitar player John Scofield recorded a version for his album That's What I Say: John Scofield Plays the Music of Ray Charles in 2005, featuring the blues guitarist John Mayer on additional guitar and vocals. Mayer covered the song again with his band during his tour in summer 2007. A recorded live version from a Los Angeles show during that tour is available on Mayer's CD/DVD release Where the Light Is. A Ray Charles tribute album also provided the impetus for <b>jazz</b> singer Roseanna Vitro's......)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "558"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/If_You_Go_Away",
    /*title*/      "If You Go Away",
    /*snippet*/    R"SNIPPET(...<b>Jazz</b> Length 3:49 Label Epic Records Songwriter(s) Jacques Brel, Rod McKuen Producer(s) Bob Morgan Damita Jo singles chronology "Gotta Travel On" (1965) "If You Go Away" (1966) "Walk Away" (1967) Damita Jo reached #10 on the Adult Contemporary chart and #68 on the Billboard Hot 100 in 1966 for her version of the song. Terry Jacks recorded a version of the song which was released as a single in 1974 and reached #29 on the Adult Contemporary chart, #68 on the Billboard Hot 100, and went to #8 in......)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "204"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Anthology_(Ray_Charles_album)",
    /*title*/      "Anthology (Ray Charles album)",
    /*snippet*/    R"SNIPPET(...Charles' '60s and '70s ABC-Paramount material", while Rhino Records, the issuing label, refers to it in the liner notes as "the compact disc edition of Ray Charles' Greatest Hits", alluding to the two Rhino LPs issued the same year. It is one of the first CDs to be released by Rhino. Anthology Greatest hits album by Ray Charles Released 1988 Recorded 1960-1972 Genre R&amp;B soul <b>jazz</b> piano blues Length 67:25 (original), 66:18 (re-release) Label Rhino Producer Ray Charles Steve Hoffman Richard Foos Ray Charles chronology Just Between Us (1988) Anthology (1988) Would You Believe? (1990) Posthumous cover Professional ratings Review scores Source Rating AllMusic Charles, who retained the master rights (currently controlled by his estate since his June 2004 passing) to his ABC-Paramount recordings, supervised a remixing of the 20 songs on this compilation especially for this......)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "265"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Ray_Charles",
    /*title*/      "Ray Charles",
    /*snippet*/    R"SNIPPET(...1960s Background information Birth name Ray Charles Robinson Born (1930-09-23)September 23, 1930 Albany, Georgia, U.S. Died June 10, 2004(2004-06-10) (aged 73) Beverly Hills, California, U.S. Genres R&amp;B soul blues gospel country <b>jazz</b> rock and roll Occupation(s) musician singer songwriter composer Instruments Vocals piano Years active 1947–2004 Labels Atlantic ABC Tangerine Warner Bros. Swing Time Concord Columbia Flashback Associated acts The Raelettes USA for Africa Billy Joel Gladys Knight Website raycharles.com Charles pioneered the soul music genre during the 1950s by combining blues, rhythm and blues, and gospel styles into the music he recorded for Atlantic. He contributed to the integration of country music......)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "416"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/The_Pages_of_My_Mind",
    /*title*/      "Ray Charles",
    /*snippet*/    R"SNIPPET(...<b>jazz</b> rock and roll Occupation(s) musician singer songwriter composer Instruments Vocals piano Years active 1947–2004 Labels Atlantic ABC Tangerine Warner Bros. Swing Time Concord Columbia Flashback Associated acts The Raelettes USA for Africa Billy Joel Gladys Knight Website raycharles.com Charles pioneered the soul music genre during the 1950s by combining blues, rhythm and blues, and gospel styles into the music he recorded for Atlantic. He contributed to the integration of country music......)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "416"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Here_We_Go_Again_(Ray_Charles_song)",
    /*title*/      "Here We Go Again (Ray Charles song)",
    /*snippet*/    R"SNIPPET(...was first covered in an instrumental <b>jazz</b> format, and many of the more recent covers have been sung as duets, such as one with Willie Nelson and Norah Jones with Wynton Marsalis accompanying. The song was released on their 2011 tribute album Here We Go Again: Celebrating the Genius of Ray Charles. The song lent its name to Red Steagall's 2007 album as well. Cover versions have appeared on compilation albums by a number of artists, even some who did not release "Here We Go Again" as a single....)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "417"
  ),

  SEARCH_RESULT(
    /*link*/       "/ROOT%23%3F/content/zimfile/A/Modern_Sounds_in_Country_and_Western_Music",
    /*title*/      "Modern Sounds in Country and Western Music",
    /*snippet*/    R"SNIPPET(...<b>jazz</b>. Charles produced the album with Sid Feller, who helped the singer select songs to record, and performed alongside saxophonist Hank Crawford, a string section conducted by Marty Paich, and a big band arranged by Gil Fuller and Gerald Wilson. Modern Sounds in Country and Western Music was an immediate critical and commercial success. The album and its four hit singles brought Charles greater mainstream notice and recognition in the pop market, as well as airplay on both R&amp;B and country radio......)SNIPPET",
    /*bookTitle*/  "Ray Charles",
    /*wordCount*/  "424"
  )
};

// Snippets (i.e. the contents of the <cite> element) in the search results can
// slightly vary depending on
//
// - the version of libxapian (for example, in various Packages CI builds)
// - the parameters of the pagination (if using libzim before v7.2.2).
//
// In order to be able to share the same expected output data
// LARGE_SEARCH_RESULTS between multiple build platforms and test-points
// of the ServerSearchTest.searchResults test-case
//
// 1. Snippets are excluded from the plain-text comparison of actual and
//    expected HTML strings. This is done with the help of the
//    function maskSnippetsInHtmlSearchResults()
//
// 2. Snippets are checked separately. If a plain-text comparison fails
//    then a weaker comparison is attempted. Currently it works by testing
//    that the actual snippet is a substring of the "expected" snippet
//    (the "..." omitted text markes on the snippet boundaries are taken
//    into account). The implementation of that approach is via the
//    isSubSnippet() function.
//
//    Therefore the "expected" snippets in the test data must be a union of
//    all possible snippets produced at runtime for a given (document, search
//    terms) pair on all platforms of interest:
//
//    - Overlapping snippets must be properly merged
//
//    - Non-overlapping snippets can be joined with a " ... " in between.
//

typedef std::vector<std::string> Snippets;

const char SNIPPET_REGEX_FOR_HTML[] = "<cite>(.+)</cite>";

std::string maskSnippetsInHtmlSearchResults(std::string s)
{
  return replace(s, SNIPPET_REGEX_FOR_HTML, "<cite>SNIPPET TEXT WAS MASKED</cite>");
}

Snippets extractSearchResultSnippetsFromHtml(const std::string& html)
{
  Snippets snippets;
  const std::regex snippetRegex(SNIPPET_REGEX_FOR_HTML);
  std::sregex_iterator snippetIt(html.begin(), html.end(), snippetRegex);
  const std::sregex_iterator end;
  for ( ; snippetIt != end; ++snippetIt)
  {
    const std::smatch snippetMatch = *snippetIt;
    snippets.push_back(snippetMatch[1].str());
  }
  return snippets;
}

const char SNIPPET_REGEX_FOR_XML[] = "<description>(?!Search result for)(.+)</description>";

std::string maskSnippetsInXmlSearchResults(std::string s)
{
  return replace(s, SNIPPET_REGEX_FOR_XML, "<description>SNIPPET TEXT WAS MASKED</description>");
}

Snippets extractSearchResultSnippetsFromXml(const std::string& xml)
{
  Snippets snippets;
  const std::regex snippetRegex(SNIPPET_REGEX_FOR_XML);
  std::sregex_iterator snippetIt(xml.begin(), xml.end(), snippetRegex);
  const std::sregex_iterator end;
  for ( ; snippetIt != end; ++snippetIt)
  {
    const std::smatch snippetMatch = *snippetIt;
    snippets.push_back(snippetMatch[1].str());
  }
  return snippets;
}

bool isValidSnippet(const std::string& s)
{
  return s.size() >= 250
      && s.find("<b>")  != std::string::npos
      && s.find("</b>") != std::string::npos;
}

size_t leadingDotCount(const std::string& s)
{
  return s.find_first_not_of(".");
}

size_t trailingDotCount(const std::string& s)
{
  return s.size() - 1 - s.find_last_not_of(".");
}

bool isSubSnippet(std::string subSnippet, const std::string& superSnippet)
{
  const auto leadingDotCountInSubSnippet = leadingDotCount(subSnippet);
  const auto trailingDotCountInSubSnippet = trailingDotCount(subSnippet);
  const bool subSnippetIsHeadless = leadingDotCountInSubSnippet >= 3;
  const bool subSnippetIsTailless = trailingDotCountInSubSnippet >= 3;
  if ( subSnippetIsHeadless )
  {
    subSnippet = subSnippet.substr(leadingDotCountInSubSnippet);
  }

  if ( subSnippetIsTailless )
  {
    subSnippet = subSnippet.substr(0, subSnippet.size() - trailingDotCountInSubSnippet);
  }

  const auto pos = superSnippet.find(subSnippet);
  if ( pos == std::string::npos )
    return false;

  if ( subSnippetIsHeadless == (pos == 0) )
    return false;

  if ( subSnippetIsTailless == (pos + subSnippet.size() == superSnippet.size()) )
    return false;

  return true;
}

#define  RAYCHARLESZIMID "6f1d19d0-633f-087b-fb55-7ac324ff9baf"
#define  EXAMPLEZIMID    "5dc0b3af-5df2-0925-f0ca-d2bf75e78af6"

struct TestData
{
  struct PaginationEntry
  {
    std::string label;
    size_t start;
    bool selected;
  };

  std::string query;
  int start;
  size_t resultsPerPage;
  size_t totalResultCount;
  size_t firstResultIndex;
  std::vector<SearchResult> results;
  std::vector<PaginationEntry> pagination;

  static std::string makeUrl(const std::string& query, int start, size_t resultsPerPage)
  {
    std::string url = "/ROOT%23%3F/search?" + query;

    if ( start >= 0 ) {
      url += "&start=" + std::to_string(start);
    }

    if ( resultsPerPage != 0 ) {
      url += "&pageLength=" + std::to_string(resultsPerPage);
    }

    return url;
  }

  std::string extractQueryValue(const std::string& key) const
  {
    const std::string p = key + "=";
    const size_t i = query.find(p);
    if (i == std::string::npos) {
      return "";
    }
    std::string r = query.substr(i + p.size());
    return r.substr(0, r.find("&"));
  }

  std::string getPattern() const
  {
    return kiwix::urlDecode(extractQueryValue("pattern"), true);
  }

  std::string getLang() const
  {
    return extractQueryValue("books.filter.lang");
  }

  std::string url() const
  {
    return makeUrl(query, start, resultsPerPage);
  }

  std::string xmlSearchUrl() const
  {
    return url() + "&format=xml";
  }

  std::string expectedHtmlHeader() const
  {
    std::string header = totalResultCount == 0
                       ? R"(No results were found for <b>"PATTERN"</b>)"
                       : R"(Results <b>FIRSTRESULT-LASTRESULT</b> of <b>RESULTCOUNT</b> for <b>"PATTERN"</b>)";

    const size_t lastResultIndex = std::min(totalResultCount, firstResultIndex + results.size() - 1);
    header = replace(header, "FIRSTRESULT", std::to_string(firstResultIndex));
    header = replace(header, "LASTRESULT",  std::to_string(lastResultIndex));
    header = replace(header, "RESULTCOUNT", std::to_string(totalResultCount));
    header = replace(header, "PATTERN",     getPattern());
    return "%USERLANGMARKER%" + header;
  }

  std::string expectedHtmlResultsString() const
  {
    if ( results.empty() ) {
      return "\n        ";
    }

    std::string s;
    for ( const auto& r : results ) {
      s += "\n          <li>";
      s += maskSnippetsInHtmlSearchResults(r.getHtml());
      s += "          </li>";
    }
    return s;
  }

  std::string expectedHtmlFooter() const
  {
    if ( pagination.empty() ) {
      return "\n      ";
    }

    std::ostringstream oss;
    oss << "\n        <ul>\n";
    for ( const auto& p : pagination ) {
      const auto url = makeUrl(query, p.start, resultsPerPage);
      oss << "            <li>\n";
      oss << "              <a ";
      if ( p.selected ) {
        oss << "class=\"selected\"";
      }
      oss << "\n                 href=\"" << url << "\">\n";
      oss << "                " << p.label << "\n";
      oss << "              </a>\n";
      oss << "            </li>\n";
    }
    oss << "        </ul>";
    return oss.str();
  }

  std::string expectedHtml() const
  {
    const std::string html = makeSearchResultsHtml(
                                 getPattern(),
                                 expectedHtmlHeader(),
                                 expectedHtmlResultsString(),
                                 expectedHtmlFooter()
    );

    const std::string userlangMarker = extractQueryValue("userlang") == "test"
                                     ? "[I18N TESTING] "
                                     : "";

    return replace(html, "%USERLANGMARKER%", userlangMarker);
  }

    std::string expectedXmlHeader() const
    {
      std::string header = R"(<title>Search: PATTERN</title>
    <link>URL</link>
    <description>Search result for PATTERN</description>
    <opensearch:totalResults>RESULTCOUNT</opensearch:totalResults>
    <opensearch:startIndex>FIRSTRESULT</opensearch:startIndex>
    <opensearch:itemsPerPage>ITEMCOUNT</opensearch:itemsPerPage>
    <atom:link rel="search" type="application/opensearchdescription+xml" href="/ROOT%23%3F/search/searchdescription.xml"/>
    <opensearch:Query role="request"
      searchTerms="PATTERN"LANGQUERY
      startIndex="FIRSTRESULT"
      count="ITEMCOUNT"
    />)";

      const auto realResultsPerPage = resultsPerPage?resultsPerPage:25;
      const auto cleanedUpQuery = replace(query, "&userlang=test", "");
      const auto url = makeUrl(cleanedUpQuery + "&format=xml", firstResultIndex, realResultsPerPage);
      header = replace(header, "URL", replace(url, "&", "&amp;"));
      header = replace(header, "FIRSTRESULT", std::to_string(firstResultIndex));
      header = replace(header, "ITEMCOUNT",  std::to_string(realResultsPerPage));
      header = replace(header, "RESULTCOUNT", std::to_string(totalResultCount));
      header = replace(header, "PATTERN",     getPattern());
      auto queryLang = getLang();
      if (queryLang.empty()) {
        header = replace(header, "LANGQUERY", "");
      } else {
        header = replace(header, "LANGQUERY", "\n      language=\""+queryLang+"\"");
      }
      return header;
    }

    std::string expectedXmlResultsString() const
    {
      if ( results.empty() ) {
        return "\n    ";
      }

      std::string s;
      for ( const auto& r : results ) {
        s += "\n    <item>\n";
        s += maskSnippetsInXmlSearchResults(r.getXml());
        s += "\n    </item>";
      }
      return s;
    }

  std::string expectedXml() const
  {
    return makeSearchResultsXml(
             expectedXmlHeader(),
             expectedXmlResultsString()
    );
  }

  TestContext testContext() const
  {
    return TestContext{ { "url", url() } };
  }

  TestContext xmlTestContext() const
  {
    return TestContext{ { "url", xmlSearchUrl() } };
  }

  void checkHtml(const std::string& html) const
  {
    EXPECT_EQ(maskSnippetsInHtmlSearchResults(html), expectedHtml())
      << testContext();

    checkSnippets(extractSearchResultSnippetsFromHtml(html));
  }

  void checkXml(const std::string& xml) const
  {
    EXPECT_EQ(maskSnippetsInXmlSearchResults(xml), expectedXml())
      << xmlTestContext();

    checkSnippets(extractSearchResultSnippetsFromXml(xml));
  }

  void checkSnippets(const Snippets& snippets) const
  {
    ASSERT_EQ(snippets.size(), results.size());
    for ( size_t i = 0; i < results.size(); ++i )
    {
      const auto& r = results[i];
      if ( snippets[i] != r.snippet ) {
        std::cout << "Trying a weaker check for a mismatching snippet...\n";
        checkMismatchingSnippet(snippets[i], r.snippet);
      }
    }
  }

  void checkMismatchingSnippet(std::string actual, std::string expected) const
  {
    TestContext testContext{
                      { "url", url() },
                      { "actual snippet", actual },
                      { "expected snippet", expected }
    };

    ASSERT_TRUE(isValidSnippet(actual))   << testContext;
    ASSERT_TRUE(isValidSnippet(expected)) << testContext;

    if ( !isSubSnippet(actual, expected) ) {
      EXPECT_EQ(actual, expected) << testContext;
    }
  }
};

TEST(ServerSearchTest, searchResults)
{
  const TestData testData[] = {
    {
      /* query */          "pattern=velomanyunkan&books.id=" RAYCHARLESZIMID,
      /* start */            -1,
      /* resultsPerPage */   0,
      /* totalResultCount */ 0,
      /* firstResultIndex */ 1,
      /* results */          {},
      /* pagination */       {}
    },

    {
      /* query */          "pattern=velomanyunkan&books.id=" RAYCHARLESZIMID
                           "&userlang=test",
      /* start */            -1,
      /* resultsPerPage */   0,
      /* totalResultCount */ 0,
      /* firstResultIndex */ 1,
      /* results */          {},
      /* pagination */       {}
    },

    {
      /* query */          "pattern=razaf&books.id=" RAYCHARLESZIMID,
      /* start */            -1,
      /* resultsPerPage */   0,
      /* totalResultCount */ 1,
      /* firstResultIndex */ 1,
      /* results */ {
        SEARCH_RESULT(
          /*link*/       "/ROOT%23%3F/content/zimfile/A/We_Gonna_Move_to_the_Outskirts_of_Town",
          /*title*/      "We Gonna Move to the Outskirts of Town",
          /*snippet*/    R"SNIPPET(...to the Outskirts of Town "We Gonna Move to the Outskirts of Town" is a country blues song recorded September 3, 1936 by Casey Bill Weldon (voice and guitar). The song has been covered by many other musicians, most often under the title "I'm Gonna Move to the Outskirts of Town", and sometimes simply Outskirts of Town. All recordings seem to credit Weldon as songwriter, often as Weldon or as Will Weldon or as William Weldon. Some cover versions give credit also to Andy <b>Razaf</b> and/or to Roy Jacobs....)SNIPPET",
          /*bookTitle*/  "Ray Charles",
          /*wordCount*/  "93"
        )
      },
      /* pagination */ {}
    },

    {
      /* query */          "pattern=yellow&books.id=" RAYCHARLESZIMID,
      /* start */            -1,
      /* resultsPerPage */   0,
      /* totalResultCount */ 2,
      /* firstResultIndex */ 1,
      /* results */ {
        SEARCH_RESULT(
          /*link*/       "/ROOT%23%3F/content/zimfile/A/Eleanor_Rigby",
          /*title*/      "Eleanor Rigby",
          /*snippet*/    R"SNIPPET(...-side "<b>Yellow</b> Submarine" (double A-side) Released 5 August 1966 (1966-08-05) Format 7-inch single Recorded 28–29 April &amp; 6 June 1966 Studio EMI, London Genre Baroque pop, art rock Length 2:08 Label Parlophone (UK), Capitol (US) Songwriter(s) Lennon–McCartney Producer(s) George Martin The Beatles singles chronology "Paperback Writer" (1966) "Eleanor Rigby" / "<b>Yellow</b> Submarine" (1966) "Strawberry Fields Forever" / "Penny Lane" (1967) Music video "Eleanor Rigby" on YouTube The song continued the......)SNIPPET",
          /*bookTitle*/  "Ray Charles",
          /*wordCount*/  "201"
        ),

        SEARCH_RESULT(
          /*link*/       "/ROOT%23%3F/content/zimfile/A/If_You_Go_Away",
          /*title*/      "If You Go Away",
          /*snippet*/    R"SNIPPET(...standard and has been recorded by many artists, including Greta Keller, for whom some say McKuen wrote the lyrics. "If You Go Away" Single by Damita Jo from the album If You Go Away B-side "<b>Yellow</b> Days" Released 1966 Genre Jazz Length 3:49 Label Epic Records Songwriter(s) Jacques Brel, Rod McKuen Producer(s) Bob Morgan Damita Jo singles chronology "Gotta Travel On" (1965) "If You Go Away" (1966) "Walk Away" (1967) Damita Jo reached #10 on the Adult Contemporary chart and #68 on the Billboard Hot 100 in 1966 for her version of the song. Terry Jacks recorded a version of the song which was released as a single in 1974 and reached #29 on the Adult Contemporary chart, #68 on the......)SNIPPET",
          /*bookTitle*/  "Ray Charles",
          /*wordCount*/  "204"
        )
      },
      /* pagination */ {}
    },

    {
      /* query */          "pattern=yellow&books.id=" RAYCHARLESZIMID,
      /* start */            0,
      /* resultsPerPage */   0,
      /* totalResultCount */ 2,
      /* firstResultIndex */ 1,
      /* results */ {
        SEARCH_RESULT(
          /*link*/       "/ROOT%23%3F/content/zimfile/A/Eleanor_Rigby",
          /*title*/      "Eleanor Rigby",
          /*snippet*/    R"SNIPPET(...-side "<b>Yellow</b> Submarine" (double A-side) Released 5 August 1966 (1966-08-05) Format 7-inch single Recorded 28–29 April &amp; 6 June 1966 Studio EMI, London Genre Baroque pop, art rock Length 2:08 Label Parlophone (UK), Capitol (US) Songwriter(s) Lennon–McCartney Producer(s) George Martin The Beatles singles chronology "Paperback Writer" (1966) "Eleanor Rigby" / "<b>Yellow</b> Submarine" (1966) "Strawberry Fields Forever" / "Penny Lane" (1967) Music video "Eleanor Rigby" on YouTube The song continued the......)SNIPPET",
          /*bookTitle*/  "Ray Charles",
          /*wordCount*/  "201"
        ),

        SEARCH_RESULT(
          /*link*/       "/ROOT%23%3F/content/zimfile/A/If_You_Go_Away",
          /*title*/      "If You Go Away",
          /*snippet*/    R"SNIPPET(...standard and has been recorded by many artists, including Greta Keller, for whom some say McKuen wrote the lyrics. "If You Go Away" Single by Damita Jo from the album If You Go Away B-side "<b>Yellow</b> Days" Released 1966 Genre Jazz Length 3:49 Label Epic Records Songwriter(s) Jacques Brel, Rod McKuen Producer(s) Bob Morgan Damita Jo singles chronology "Gotta Travel On" (1965) "If You Go Away" (1966) "Walk Away" (1967) Damita Jo reached #10 on the Adult Contemporary chart and #68 on the Billboard Hot 100 in 1966 for her version of the song. Terry Jacks recorded a version of the song which was released as a single in 1974 and reached #29 on the Adult Contemporary chart, #68 on the......)SNIPPET",
          /*bookTitle*/  "Ray Charles",
          /*wordCount*/  "204"
        )
      },
      /* pagination */ {}
    },

    {
      /* query */          "pattern=yellow&books.id=" RAYCHARLESZIMID,
      /* start */            1,
      /* resultsPerPage */   0,
      /* totalResultCount */ 2,
      /* firstResultIndex */ 1,
      /* results */ {
        SEARCH_RESULT(
          /*link*/       "/ROOT%23%3F/content/zimfile/A/Eleanor_Rigby",
          /*title*/      "Eleanor Rigby",
          /*snippet*/    R"SNIPPET(...-side "<b>Yellow</b> Submarine" (double A-side) Released 5 August 1966 (1966-08-05) Format 7-inch single Recorded 28–29 April &amp; 6 June 1966 Studio EMI, London Genre Baroque pop, art rock Length 2:08 Label Parlophone (UK), Capitol (US) Songwriter(s) Lennon–McCartney Producer(s) George Martin The Beatles singles chronology "Paperback Writer" (1966) "Eleanor Rigby" / "<b>Yellow</b> Submarine" (1966) "Strawberry Fields Forever" / "Penny Lane" (1967) Music video "Eleanor Rigby" on YouTube The song continued the......)SNIPPET",
          /*bookTitle*/  "Ray Charles",
          /*wordCount*/  "201"
        ),

        SEARCH_RESULT(
          /*link*/       "/ROOT%23%3F/content/zimfile/A/If_You_Go_Away",
          /*title*/      "If You Go Away",
          /*snippet*/    R"SNIPPET(...standard and has been recorded by many artists, including Greta Keller, for whom some say McKuen wrote the lyrics. "If You Go Away" Single by Damita Jo from the album If You Go Away B-side "<b>Yellow</b> Days" Released 1966 Genre Jazz Length 3:49 Label Epic Records Songwriter(s) Jacques Brel, Rod McKuen Producer(s) Bob Morgan Damita Jo singles chronology "Gotta Travel On" (1965) "If You Go Away" (1966) "Walk Away" (1967) Damita Jo reached #10 on the Adult Contemporary chart and #68 on the Billboard Hot 100 in 1966 for her version of the song. Terry Jacks recorded a version of the song which was released as a single in 1974 and reached #29 on the Adult Contemporary chart, #68 on the......)SNIPPET",
          /*bookTitle*/  "Ray Charles",
          /*wordCount*/  "204"
        )
      },
      /* pagination */ {}
    },

    {
      /* query */          "pattern=jazz&books.id=" RAYCHARLESZIMID,
      /* start */            -1,
      /* resultsPerPage */   100,
      /* totalResultCount */ 44,
      /* firstResultIndex */ 1,
      /* results */ LARGE_SEARCH_RESULTS,
      /* pagination */ {}
    },

    {
      /* query */          "pattern=jazz&books.id=" RAYCHARLESZIMID
                           "&userlang=test",
      /* start */            -1,
      /* resultsPerPage */   100,
      /* totalResultCount */ 44,
      /* firstResultIndex */ 1,
      /* results */ LARGE_SEARCH_RESULTS,
      /* pagination */ {}
    },

    {
      /* query */          "pattern=jazz&books.id=" RAYCHARLESZIMID,
      /* start */            -1,
      /* resultsPerPage */   5,
      /* totalResultCount */ 44,
      /* firstResultIndex */ 1,
      /* results */ {
        LARGE_SEARCH_RESULTS[0],
        LARGE_SEARCH_RESULTS[1],
        LARGE_SEARCH_RESULTS[2],
        LARGE_SEARCH_RESULTS[3],
        LARGE_SEARCH_RESULTS[4],
      },

      /* pagination */ {
        { "1", 0,  true  },
        { "2", 5,  false },
        { "3", 10, false },
        { "4", 15, false },
        { "5", 20, false },
        { "▶", 40, false },
      }
    },

    {
      /* query */          "pattern=jazz&books.id=" RAYCHARLESZIMID,
      /* start */            6,
      /* resultsPerPage */   5,
      /* totalResultCount */ 44,
      /* firstResultIndex */ 6,
      /* results */ {
        LARGE_SEARCH_RESULTS[5],
        LARGE_SEARCH_RESULTS[6],
        LARGE_SEARCH_RESULTS[7],
        LARGE_SEARCH_RESULTS[8],
        LARGE_SEARCH_RESULTS[9],
      },

      /* pagination */ {
        { "1", 0,  false },
        { "2", 5,  true  },
        { "3", 10, false },
        { "4", 15, false },
        { "5", 20, false },
        { "6", 25, false },
        { "▶", 40, false },
      }
    },

    {
      /* query */          "pattern=jazz&books.id=" RAYCHARLESZIMID,
      /* start */            11,
      /* resultsPerPage */   5,
      /* totalResultCount */ 44,
      /* firstResultIndex */ 11,
      /* results */ {
        LARGE_SEARCH_RESULTS[10],
        LARGE_SEARCH_RESULTS[11],
        LARGE_SEARCH_RESULTS[12],
        LARGE_SEARCH_RESULTS[13],
        LARGE_SEARCH_RESULTS[14],
      },

      /* pagination */ {
        { "1", 0,  false },
        { "2", 5,  false },
        { "3", 10, true  },
        { "4", 15, false },
        { "5", 20, false },
        { "6", 25, false },
        { "7", 30, false },
        { "▶", 40, false },
      }
    },

    {
      /* query */          "pattern=jazz&books.id=" RAYCHARLESZIMID,
      /* start */            16,
      /* resultsPerPage */   5,
      /* totalResultCount */ 44,
      /* firstResultIndex */ 16,
      /* results */ {
        LARGE_SEARCH_RESULTS[15],
        LARGE_SEARCH_RESULTS[16],
        LARGE_SEARCH_RESULTS[17],
        LARGE_SEARCH_RESULTS[18],
        LARGE_SEARCH_RESULTS[19],
      },

      /* pagination */ {
        { "1", 0,  false },
        { "2", 5,  false },
        { "3", 10, false },
        { "4", 15, true  },
        { "5", 20, false },
        { "6", 25, false },
        { "7", 30, false },
        { "8", 35, false },
        { "▶", 40, false },
      }
    },

    {
      /* query */            "pattern=jazz&books.id=" RAYCHARLESZIMID,
      /* start */            21,
      /* resultsPerPage */   5,
      /* totalResultCount */ 44,
      /* firstResultIndex */ 21,
      /* results */ {
        LARGE_SEARCH_RESULTS[20],
        LARGE_SEARCH_RESULTS[21],
        LARGE_SEARCH_RESULTS[22],
        LARGE_SEARCH_RESULTS[23],
        LARGE_SEARCH_RESULTS[24],
      },

      /* pagination */ {
        { "1", 0,  false },
        { "2", 5,  false },
        { "3", 10, false },
        { "4", 15, false },
        { "5", 20, true  },
        { "6", 25, false },
        { "7", 30, false },
        { "8", 35, false },
        { "9", 40, false },
      }
    },

    {
      /* query */          "pattern=jazz&books.id=" RAYCHARLESZIMID,
      /* start */            26,
      /* resultsPerPage */   5,
      /* totalResultCount */ 44,
      /* firstResultIndex */ 26,
      /* results */ {
        LARGE_SEARCH_RESULTS[25],
        LARGE_SEARCH_RESULTS[26],
        LARGE_SEARCH_RESULTS[27],
        LARGE_SEARCH_RESULTS[28],
        LARGE_SEARCH_RESULTS[29],
      },

      /* pagination */ {
        { "◀", 0,  false },
        { "2", 5,  false },
        { "3", 10, false },
        { "4", 15, false },
        { "5", 20, false },
        { "6", 25, true  },
        { "7", 30, false },
        { "8", 35, false },
        { "9", 40, false },
      }
    },

    {
      /* query */          "pattern=jazz&books.id=" RAYCHARLESZIMID,
      /* start */            31,
      /* resultsPerPage */   5,
      /* totalResultCount */ 44,
      /* firstResultIndex */ 31,
      /* results */ {
        LARGE_SEARCH_RESULTS[30],
        LARGE_SEARCH_RESULTS[31],
        LARGE_SEARCH_RESULTS[32],
        LARGE_SEARCH_RESULTS[33],
        LARGE_SEARCH_RESULTS[34],
      },

      /* pagination */ {
        { "◀", 0,  false },
        { "3", 10, false },
        { "4", 15, false },
        { "5", 20, false },
        { "6", 25, false },
        { "7", 30, true  },
        { "8", 35, false },
        { "9", 40, false },
      }
    },

    {
      /* query */          "pattern=jazz&books.id=" RAYCHARLESZIMID,
      /* start */            36,
      /* resultsPerPage */   5,
      /* totalResultCount */ 44,
      /* firstResultIndex */ 36,
      /* results */ {
        LARGE_SEARCH_RESULTS[35],
        LARGE_SEARCH_RESULTS[36],
        LARGE_SEARCH_RESULTS[37],
        LARGE_SEARCH_RESULTS[38],
        LARGE_SEARCH_RESULTS[39],
      },

      /* pagination */ {
        { "◀", 0,  false },
        { "4", 15, false },
        { "5", 20, false },
        { "6", 25, false },
        { "7", 30, false },
        { "8", 35, true  },
        { "9", 40, false },
      }
    },

    {
      /* query */          "pattern=jazz&books.id=" RAYCHARLESZIMID,
      /* start */            41,
      /* resultsPerPage */   5,
      /* totalResultCount */ 44,
      /* firstResultIndex */ 41,
      /* results */ {
        LARGE_SEARCH_RESULTS[40],
        LARGE_SEARCH_RESULTS[41],
        LARGE_SEARCH_RESULTS[42],
        LARGE_SEARCH_RESULTS[43],
      },

      /* pagination */ {
        { "◀", 0,  false },
        { "5", 20, false },
        { "6", 25, false },
        { "7", 30, false },
        { "8", 35, false },
        { "9", 40, true  },
      }
    },

    {
      /* query */          "pattern=jazz&books.id=" RAYCHARLESZIMID,
      /* start */            22,
      /* resultsPerPage */   3,
      /* totalResultCount */ 44,
      /* firstResultIndex */ 22,
      /* results */ {
        LARGE_SEARCH_RESULTS[21],
        LARGE_SEARCH_RESULTS[22],
        LARGE_SEARCH_RESULTS[23],
      },

      /* pagination */ {
        {  "◀", 0,  false },
        {  "4", 9,  false },
        {  "5", 12, false },
        {  "6", 15, false },
        {  "7", 18, false },
        {  "8", 21, true  },
        {  "9", 24, false },
        { "10", 27, false },
        { "11", 30, false },
        { "12", 33, false },
        {  "▶", 42, false },
      }
    },

    // This test-point only documents how the current implementation
    // works, not how it should work!
    {
      /* query */          "pattern=jazz&books.id=" RAYCHARLESZIMID,
      /* start */            46,
      /* resultsPerPage */   5,
      /* totalResultCount */ 44,
      /* firstResultIndex */ 46,
      /* results */ {},

      /* pagination */ {
        { "◀", 0,  false },
        { "6", 25, false },
        { "7", 30, false },
        { "8", 35, false },
        { "9", 40, false  },
      }
    },

    // We must return results from the two books
    {
      /* query */          "pattern=travel"
                           "&books.id=" RAYCHARLESZIMID
                           "&books.id=" EXAMPLEZIMID,
      /* start */            0,
      /* resultsPerPage */   10,
      /* totalResultCount */ 2,
      /* firstResultIndex */ 1,
      /* results */          {
        SEARCH_RESULT_FOR_TRAVEL_IN_RAYCHARLESZIM,
        SEARCH_RESULT_FOR_TRAVEL_IN_EXAMPLEZIM
      },
      /* pagination */       {}
    },

    {
       /* query */          "pattern=travel"
                            "&books.filter.lang=eng",
       /* start */            0,
       /* resultsPerPage */   10,
       /* totalResultCount */ 2,
       /* firstResultIndex */ 1,
       /* results */          {
        SEARCH_RESULT_FOR_TRAVEL_IN_RAYCHARLESZIM,
        SEARCH_RESULT_FOR_TRAVEL_IN_EXAMPLEZIM
      },
      /* pagination */       {}
    },

    // books.name filters by the name of the ZIM file
    {
      /* query */          "pattern=travel"
                           "&books.name=zimfile",
      /* start */            0,
      /* resultsPerPage */   10,
      /* totalResultCount */ 1,
      /* firstResultIndex */ 1,
      /* results */          {
        SEARCH_RESULT_FOR_TRAVEL_IN_RAYCHARLESZIM
      },

      /* pagination */       {}
    },

    // books.name filters by the name of the ZIM file
    {
      /* query */          "pattern=travel"
                           "&books.name=example",
      /* start */            0,
      /* resultsPerPage */   10,
      /* totalResultCount */ 1,
      /* firstResultIndex */ 1,
      /* results */          {
        SEARCH_RESULT_FOR_TRAVEL_IN_EXAMPLEZIM
      },

      /* pagination */       {}
    },

    // books.filter.name filters by the book name
    {
       /* query */          "pattern=travel"
                            "&books.filter.name=wikipedia_en_ray_charles",
       /* start */            0,
       /* resultsPerPage */   10,
       /* totalResultCount */ 1,
       /* firstResultIndex */ 1,
       /* results */          {
        SEARCH_RESULT_FOR_TRAVEL_IN_RAYCHARLESZIM
      },

      /* pagination */       {}
    },

    // books.filter.name filters by the book name
    {
      /* query */          "pattern=travel"
                           "&books.filter.name=bookname_of_example_zim",
      /* start */            0,
      /* resultsPerPage */   10,
      /* totalResultCount */ 1,
      /* firstResultIndex */ 1,
      /* results */          {
        SEARCH_RESULT_FOR_TRAVEL_IN_EXAMPLEZIM
      },

      /* pagination */       {}
    },

    // Adding a book (without match) doesn't change the results
    {
      /* query */          "pattern=jazz"
                           "&books.id=" RAYCHARLESZIMID
                           "&books.id=" EXAMPLEZIMID,
      /* start */            -1,
      /* resultsPerPage */   100,
      /* totalResultCount */ 44,
      /* firstResultIndex */ 1,
      /* results */ LARGE_SEARCH_RESULTS,
      /* pagination */ {}
    },

    {
      /* query */          "pattern=jazz"
                           "&books.filter.lang=eng",
      /* start */            -1,
      /* resultsPerPage */   5,
      /* totalResultCount */ 44,
      /* firstResultIndex */ 1,
      /* results */ {
        LARGE_SEARCH_RESULTS[0],
        LARGE_SEARCH_RESULTS[1],
        LARGE_SEARCH_RESULTS[2],
        LARGE_SEARCH_RESULTS[3],
        LARGE_SEARCH_RESULTS[4],
      },

      /* pagination */ {
        { "1", 0,  true  },
        { "2", 5,  false },
        { "3", 10, false },
        { "4", 15, false },
        { "5", 20, false },
        { "▶", 40, false },
      }
    },

    {
      /* query */          "pattern=jazz"
                           "&books.filter.tag=wikipedia",
      /* start */            -1,
      /* resultsPerPage */   5,
      /* totalResultCount */ 44,
      /* firstResultIndex */ 1,
      /* results */ {
        LARGE_SEARCH_RESULTS[0],
        LARGE_SEARCH_RESULTS[1],
        LARGE_SEARCH_RESULTS[2],
        LARGE_SEARCH_RESULTS[3],
        LARGE_SEARCH_RESULTS[4],
      },

      /* pagination */ {
        { "1", 0,  true  },
        { "2", 5,  false },
        { "3", 10, false },
        { "4", 15, false },
        { "5", 20, false },
        { "▶", 40, false },
      }
    },

    {
      /* query */          "pattern=jazz"
                           "&books.filter.lang=eng"
                           "&books.filter.title=Ray%20Charles",
      /* start */            -1,
      /* resultsPerPage */   5,
      /* totalResultCount */ 44,
      /* firstResultIndex */ 1,
      /* results */ {
        LARGE_SEARCH_RESULTS[0],
        LARGE_SEARCH_RESULTS[1],
        LARGE_SEARCH_RESULTS[2],
        LARGE_SEARCH_RESULTS[3],
        LARGE_SEARCH_RESULTS[4],
      },

      /* pagination */ {
        { "1", 0,  true  },
        { "2", 5,  false },
        { "3", 10, false },
        { "4", 15, false },
        { "5", 20, false },
        { "▶", 40, false },
      }
    },

    // Be sure that searching with accented query return the same things than non accented query.
    {
      /* query */          "pattern=j%C3%A0zz"
                           "&books.filter.lang=eng"
                           "&books.filter.title=Ray%20Charles",
      /* start */            -1,
      /* resultsPerPage */   5,
      /* totalResultCount */ 44,
      /* firstResultIndex */ 1,
      /* results */ {
        LARGE_SEARCH_RESULTS[0],
        LARGE_SEARCH_RESULTS[1],
        LARGE_SEARCH_RESULTS[2],
        LARGE_SEARCH_RESULTS[3],
        LARGE_SEARCH_RESULTS[4],
      },

      /* pagination */ {
        { "1", 0,  true  },
        { "2", 5,  false },
        { "3", 10, false },
        { "4", 15, false },
        { "5", 20, false },
        { "▶", 40, false },
      }
    },
  };

  ZimFileServer zfs(SERVER_PORT, ZimFileServer::DEFAULT_OPTIONS,
                    "./test/lib_for_server_search_test.xml");

  for ( const auto& t : testData ) {
    const std::string htmlSearchUrl = t.url();
    const auto htmlRes = zfs.GET(htmlSearchUrl.c_str());
    EXPECT_EQ(htmlRes->status, 200);
    t.checkHtml(htmlRes->body);

    const std::string xmlSearchUrl = t.xmlSearchUrl();
    const auto xmlRes = zfs.GET(xmlSearchUrl.c_str());
    EXPECT_EQ(xmlRes->status, 200);
    t.checkXml(xmlRes->body);
  }
}

std::string invalidRequestErrorHtml(std::string url,
                                    std::string errorMsgId,
                                    std::string errorMsgParamsJSON,
                                    std::string errorText)
{
  return R"(<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <meta content="text/html;charset=UTF-8" http-equiv="content-type" />
    <title>Invalid request</title>
    <script>
      window.KIWIX_RESPONSE_TEMPLATE = )" + ERROR_HTML_TEMPLATE_JS_STRING + R"(;
      window.KIWIX_RESPONSE_DATA = { "CSS_URL" : false, "PAGE_HEADING" : { "msgid" : "400-page-heading", "params" : { } }, "PAGE_TITLE" : { "msgid" : "400-page-title", "params" : { } }, "details" : [ { "p" : { "msgid" : "invalid-request", "params" : { "url" : ")" + url + R"(" } } }, { "p" : { "msgid" : ")" + errorMsgId + R"(", "params" : )" + errorMsgParamsJSON + R"( } } ] };
    </script>
  </head>
  <body>
    <h1>Invalid request</h1>
    <p>
      The requested URL ")" + url + R"(" is not a valid request.
    </p>
    <p>
      )" + errorText + R"(
    </p>
  </body>
</html>
)";
}

const char CONFUSION_OF_TONGUES_ERROR_TEXT[] = "Two or more books in different languages would participate in search, which may lead to confusing results.";

std::string expectedConfusionOfTonguesErrorHtml(std::string url)
{
  return invalidRequestErrorHtml(url,
    /* errorMsgId */             "confusion-of-tongues",
    /* errorMsgParamsJSON */     "{ }",
    /* errorText */              CONFUSION_OF_TONGUES_ERROR_TEXT
  );
}

std::string expectedConfusionOfTonguesErrorXml(std::string url)
{
  return R"(<?xml version="1.0" encoding="UTF-8">
<error>Invalid request</error>
<detail>The requested URL ")" + url + R"(" is not a valid request.</detail>
<detail>)" + CONFUSION_OF_TONGUES_ERROR_TEXT + R"(</detail>
)";
}

TEST(ServerSearchTest, searchInMultilanguageBookSetIsDenied)
{
  const std::string testQueries[] = {
      "pattern=towerofbabel",
      "pattern=babylon&books.filter.maxsize=1000000",
      "pattern=baby&books.id=" RAYCHARLESZIMID "&books.id=" EXAMPLEZIMID,
  };

  // The default limit on the number of books in a multi-zim search is 3
  const ZimFileServer::FilePathCollection ZIMFILES{
    "./test/zimfile.zim",       // eng
    "./test/example.zim",       // en
    "./test/corner_cases#&.zim" // =en
  };

  ZimFileServer zfs(SERVER_PORT, ZimFileServer::DEFAULT_OPTIONS, ZIMFILES);
  for ( const auto& q : testQueries ) {
    {
      // HTML mode
      const std::string url = "/ROOT%23%3F/search?" + q;
      const auto r = zfs.GET(url.c_str());
      const TestContext ctx{ {"url", url} };
      EXPECT_EQ(r->status, 400) << ctx;
      EXPECT_EQ(r->body, expectedConfusionOfTonguesErrorHtml(url)) << ctx;
    }

    {
      // XML mode
      const std::string url = "/ROOT%23%3F/search?" + q + "&format=xml";
      const auto r = zfs.GET(url.c_str());
      const TestContext ctx{ {"url", url} };
      EXPECT_EQ(r->status, 400) << ctx;
      EXPECT_EQ(r->body, expectedConfusionOfTonguesErrorXml(url)) << ctx;
    }
  }
}

std::string noSuchBookErrorHtml(std::string url, std::string bookName)
{
  return invalidRequestErrorHtml(url,
    /* errorMsgId */             "no-such-book",
    /* errorMsgParamsJSON */     "{ \"BOOK_NAME\" : \"" + bookName + "\" }",
    /* errorText */              "No such book: " + bookName
  );
}

std::string noBookFoundErrorHtml(std::string url)
{
  return invalidRequestErrorHtml(url,
    /* errorMsgId */             "no-book-found",
    /* errorMsgParamsJSON */     "{ }",
    /* errorText */              "No book matches selection criteria"
  );
}

TEST(ServerSearchTest, bookSelectionNegativeTests)
{
  ZimFileServer zfs(SERVER_PORT, ZimFileServer::DEFAULT_OPTIONS,
                    "./test/lib_for_server_search_test.xml");

  {
    // books.name (unlike books.filter.name) DOESN'T consider the book name
    // and reports an error (surprise!)
    const std::string bookName = "wikipedia_en_ray_charles";
    const std::string q = "pattern=travel&books.name=" + bookName;
    const std::string url = "/ROOT%23%3F/search?" + q;

    const auto r = zfs.GET(url.c_str());
    EXPECT_EQ(r->status, 400);
    EXPECT_EQ(r->body, noSuchBookErrorHtml(url, bookName));
  }

  {
    // books.filter.name (unlike books.name) DOESN'T consider the ZIM file name
    // and reports an error (differently from books.name)
    const std::string q = "pattern=travel&books.filter.name=zimfile";
    const std::string url = "/ROOT%23%3F/search?" + q;

    const auto r = zfs.GET(url.c_str());
    EXPECT_EQ(r->status, 400);
    EXPECT_EQ(r->body, noBookFoundErrorHtml(url));
  }
}
