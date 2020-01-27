
package org.kiwix.kiwixlib;

public class Book
{
  public Book() { allocate(); }

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
  public native String getTags();

  public native long getArticleCount();
  public native long getMediaCount();
  public native long getSize();

  public native String getFavicon();
  public native String getFaviconUrl();
  public native String getFaviconMimeType();

  private native void allocate();
  private native void dispose();
  private long nativeHandle;
}
