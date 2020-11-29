
import java.io.*;
import java.util.*;
import org.junit.Test;
import static org.junit.Assert.*;
import org.kiwix.kiwixlib.*;

public class test {
static {
  System.loadLibrary("kiwix");
}

private static String getFileContent(String path)
throws IOException
{
  BufferedReader reader = new BufferedReader(new FileReader(path));
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
public void testReader()
throws JNIKiwixException, IOException
{
  JNIKiwixReader reader = new JNIKiwixReader("small.zim");
  assertEquals("Test ZIM file", reader.getTitle());
  assertEquals(45, reader.getFileSize()); // The file size is in KiB
  assertEquals("A/main.html", reader.getMainPage());
  String s = getFileContent("small_zimfile_data/main.html");
  byte[] c = reader.getContent(new JNIKiwixString("A/main.html"),
                                new JNIKiwixString(),
                                new JNIKiwixString(),
                                new JNIKiwixInt());
  assertEquals(s, new String(c));

}

@Test
public void testLibrary()
throws IOException
{
  Library lib = new Library();
  Manager manager = new Manager(lib);
  String content = getFileContent("catalog.xml");
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
