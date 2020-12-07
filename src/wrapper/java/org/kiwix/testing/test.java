
import java.io.*;
import java.util.*;
import org.junit.Test;
import static org.junit.Assert.*;
import org.kiwix.kiwixlib.*;

public class test {
static {
  System.loadLibrary("kiwix");
}

private static byte[] getFileContent(String path)
throws IOException
{
  File file = new File(path);
  DataInputStream in = new DataInputStream(
                         new BufferedInputStream(
                           new FileInputStream(file)));
  byte[] data = new byte[(int)file.length()];
  in.read(data);
  return data;
}

private static String getTextFileContent(String path)
throws IOException
{
  return new String(getFileContent(path));
}

@Test
public void testReader()
throws JNIKiwixException, IOException
{
  JNIKiwixReader reader = new JNIKiwixReader("small.zim");
  assertEquals("Test ZIM file", reader.getTitle());
  assertEquals(45, reader.getFileSize()); // The file size is in KiB
  assertEquals("A/main.html", reader.getMainPage());
  String s = getTextFileContent("small_zimfile_data/main.html");
  byte[] c = reader.getContent(new JNIKiwixString("A/main.html"),
                                new JNIKiwixString(),
                                new JNIKiwixString(),
                                new JNIKiwixInt());
  assertEquals(s, new String(c));

  byte[] faviconData = getFileContent("small_zimfile_data/favicon.png");
  assertEquals(faviconData.length, reader.getArticleSize("I/favicon.png"));
  c = reader.getContent(new JNIKiwixString("I/favicon.png"),
                                new JNIKiwixString(),
                                new JNIKiwixString(),
                                new JNIKiwixInt());
  assertTrue(Arrays.equals(faviconData, c));
}

@Test
public void testReaderByFd()
throws JNIKiwixException, IOException
{
  FileInputStream fis = new FileInputStream("small.zim");
  JNIKiwixReader reader = new JNIKiwixReader(fis.getFD());
  assertEquals("Test ZIM file", reader.getTitle());
  assertEquals(45, reader.getFileSize()); // The file size is in KiB
  assertEquals("A/main.html", reader.getMainPage());
  String s = getTextFileContent("small_zimfile_data/main.html");
  byte[] c = reader.getContent(new JNIKiwixString("A/main.html"),
                                new JNIKiwixString(),
                                new JNIKiwixString(),
                                new JNIKiwixInt());
  assertEquals(s, new String(c));

  byte[] faviconData = getFileContent("small_zimfile_data/favicon.png");
  assertEquals(faviconData.length, reader.getArticleSize("I/favicon.png"));
  c = reader.getContent(new JNIKiwixString("I/favicon.png"),
                                new JNIKiwixString(),
                                new JNIKiwixString(),
                                new JNIKiwixInt());
  assertTrue(Arrays.equals(faviconData, c));
}

@Test
public void testReaderWithAnEmbeddedArchive()
throws JNIKiwixException, IOException
{
  File plainArchive = new File("small.zim");
  FileInputStream fis = new FileInputStream("small.zim.embedded");
  JNIKiwixReader reader = new JNIKiwixReader(fis.getFD(), 8, plainArchive.length());
  assertEquals("Test ZIM file", reader.getTitle());
  assertEquals(45, reader.getFileSize()); // The file size is in KiB
  assertEquals("A/main.html", reader.getMainPage());
  String s = getTextFileContent("small_zimfile_data/main.html");
  byte[] c = reader.getContent(new JNIKiwixString("A/main.html"),
                                new JNIKiwixString(),
                                new JNIKiwixString(),
                                new JNIKiwixInt());
  assertEquals(s, new String(c));

  byte[] faviconData = getFileContent("small_zimfile_data/favicon.png");
  assertEquals(faviconData.length, reader.getArticleSize("I/favicon.png"));
  c = reader.getContent(new JNIKiwixString("I/favicon.png"),
                                new JNIKiwixString(),
                                new JNIKiwixString(),
                                new JNIKiwixInt());
  assertTrue(Arrays.equals(faviconData, c));
}

@Test
public void testLibrary()
throws IOException
{
  Library lib = new Library();
  Manager manager = new Manager(lib);
  String content = getTextFileContent("catalog.xml");
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
