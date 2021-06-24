
package org.kiwix.kiwixlib;

public class Book
{
  public Book() { allocate(); }


  public native void update(Book book);
  public native void update(JNIKiwixReader reader);

  @Override
  protected void finalize() { dispose();  }

  public native String getId();
  public native String getPath();
  public native boolean isPathValid();
  public native String getTitle();
  public native String getDescription();
  public native String getLanguage();
  public native String getCreator();
  public native String getPublisher();
  public native String getDate();
  public native String getUrl();
  public native String getName();
  public native String getFlavour();
  public native String getCategory();
  public native String getTags();
  /**
   * Return the value associated to the tag tagName
   *
   * @param tagName the tag name to search for.
   * @return The value of the tag. If the tag is not found, return empty string.
   */
  public native String getTagStr(String tagName);

  public native long getArticleCount();
  public native long getMediaCount();
  public native long getSize();

  public native String getFavicon();
  public native String getFaviconUrl();
  public native String getFaviconMimeType();

  private native void allocate();
  private native void dispose();
  private long nativeHandle;

    /**
     * Only use the book's id to generate a hash code
     * Returns a hash code for this string in this case we should use the id of book
     */
  public native long hashCode();

   /**
    * Two books are equal if their ids match
    * @param object will assume as [Book] object.
    * @return true if their id's match
    */
  public native boolean equals(Object object);
}
