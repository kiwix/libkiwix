
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
  manager.readOpds(content, "http://localhost");
  assertEquals(lib.getBookCount(true, true), 1);
  String[] bookIds = lib.getBooksIds();
  assertEquals(bookIds.length, 1);
  Book book = lib.getBookById(bookIds[0]);
  assertEquals(book.getTitle(), "Test ZIM file");
  assertEquals(book.getTags(), "unit;test");
  assertEquals(book.getFaviconUrl(), "http://localhost/meta?name=favicon&content=small");
  assertEquals(book.getUrl(), "http://localhost/small.zim");
}

static
public void main(String[] args) {
  Library lib = new Library();
  lib.getBookCount(true, true);
}



}
