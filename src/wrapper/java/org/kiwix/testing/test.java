
import java.io.*;
import java.util.*;
import org.junit.Test;
import static org.junit.Assert.*;
import org.kiwix.kiwixlib.*;

public class test {
static {
  System.loadLibrary("kiwix");
}

private static String getCatalogContent()
throws IOException
{
  BufferedReader reader = new BufferedReader(new FileReader("catalog.xml"));
  String line;
  StringBuilder sb = new StringBuilder();
  while ((line = reader.readLine()) != null)
  {
    sb.append(line + "\n");
  }
  reader.close();
  return sb.toString();
}

@Test
public void testSome()
throws IOException
{
  Library lib = new Library();
  Manager manager = new Manager(lib);
  String content = getCatalogContent();
  manager.readOpds(content, "https://library.kiwix.org");
  assertEquals(lib.getBookCount(true, true), 10);
  String[] bookIds = lib.getBooksIds();
  assertEquals(bookIds.length, 10);
  Book book = lib.getBookById(bookIds[0]);
  assertEquals(book.getTitle(), "Wikisource");
  assertEquals(book.getTags(), "wikisource;_category:wikisource;_pictures:no;_videos:no;_details:yes;_ftindex:yes");
  assertEquals(book.getFaviconUrl(), "https://library.kiwix.org/meta?name=favicon&content=wikisource_fr_all_nopic_2020-01");
  assertEquals(book.getUrl(), "http://download.kiwix.org/zim/wikisource/wikisource_fr_all_nopic_2020-01.zim.meta4");
}

static
public void main(String[] args) {
  Library lib = new Library();
  lib.getBookCount(true, true);
}



}
