// Terminology
//
// user url: identifier of the page that has to be displayed in the viewer
//           and that is used as the hash component of the viewer URL. For
//           book resources the address url is {book}/{resource} .
//
// iframe url: the URL to be loaded in the viewer iframe.

function userUrl2IframeUrl(url) {
  if ( url == '' ) {
    return blankPageUrl;
  }

  if ( url.startsWith('search?') ) {
    return `${root}/${url}`;
  }

  return `${root}/content/${url}`;
}

function getBookFromUserUrl(url) {
  if ( url == '' ) {
    return null;
  }

  if ( url.startsWith('search?') ) {
    const p = new URLSearchParams(url.slice("search?".length));
    return p.get('books.name') || p.get('content');
  }
  return url.split('/')[0];
}

let currentBook = getBookFromUserUrl(location.hash.slice(1));
let currentBookTitle = null;

const bookUIGroup = document.getElementById('kiwix_serve_taskbar_book_ui_group');
const homeButton = document.getElementById('kiwix_serve_taskbar_home_button');
const contentIframe = document.getElementById('content_iframe');


function gotoMainPageOfCurrentBook() {
  location.hash = currentBook + '/';
}

function gotoUrl(url) {
  contentIframe.src = url;
}

function gotoRandomPage() {
  gotoUrl(`${root}/random?content=${currentBook}`);
}

function performSearch() {
  const searchbox = document.getElementById('kiwixsearchbox');
  const q = encodeURIComponent(searchbox.value);
  gotoUrl(`${root}/search?books.name=${currentBook}&pattern=${q}`);
}

function suggestionsApiURL()
{
  return `${root}/suggest?content=${encodeURIComponent(currentBook)}`;
}

function setCurrentBook(book, title) {
  currentBook = book;
  currentBookTitle = title;
  homeButton.title = `Go to the main page of '${title}'`;
  homeButton.setAttribute("aria-label", homeButton.title);
  homeButton.innerHTML = `<button>${title}</button>`;
  bookUIGroup.style.display = 'inline';
  updateSearchBoxForBookChange();
}

function noCurrentBook() {
  currentBook = null;
  currentBookTitle = null;
  bookUIGroup.style.display = 'none';
  updateSearchBoxForBookChange();
}

function updateCurrentBookIfNeeded(userUrl) {
  const book = getBookFromUserUrl(userUrl);
  if ( currentBook != book ) {
    updateCurrentBook(book);
  }
}

function updateCurrentBook(book) {
  if ( book == null ) {
    noCurrentBook();
  } else {
    fetch(`./raw/${book}/meta/Title`).then(async (resp) => {
      if ( resp.ok ) {
        setCurrentBook(book, await resp.text());
      } else {
        noCurrentBook();
      }
    }).catch((err) => {
      console.log("Error fetching book title: " + err);
      noCurrentBook();
    });
  }
}

function iframeUrl2UserUrl(url, query) {
  if ( url == blankPageUrl ) {
    return '';
  }

  if ( url == `${root}/search` ) {
    return `search${query}`;
  }

  url = url.slice(root.length);

  return url.split('/').slice(2).join('/');
}

function getSearchPattern() {
  const url = window.location.hash.slice(1);
  if ( url.startsWith('search?') ) {
    const p = new URLSearchParams(url.slice("search?".length));
    return p.get("pattern");
  }
  return null;
}


let autoCompleteJS = null;

function closeSuggestions() {
  if ( autoCompleteJS ) {
    autoCompleteJS.close();
  }
}

function updateSearchBoxForLocationChange() {
  closeSuggestions();
  document.getElementById("kiwixsearchbox").value = getSearchPattern();
}

function updateSearchBoxForBookChange() {
  const searchbox = document.getElementById('kiwixsearchbox');
  const kiwixSearchFormWrapper = document.querySelector('.kiwix_searchform');
  if ( currentBookTitle ) {
    searchbox.title = `Search '${currentBookTitle}'`;
    searchbox.placeholder = searchbox.title;
    searchbox.setAttribute("aria-label", searchbox.title);
    kiwixSearchFormWrapper.style.display = 'inline';
  } else {
    kiwixSearchFormWrapper.style.display = 'none';
  }
}

let previousScrollTop = Infinity;

function updateToolbarVisibilityState() {
  const iframeDoc = contentIframe.contentDocument;
  const st = iframeDoc.documentElement.scrollTop || iframeDoc.body.scrollTop;
  if ( Math.abs(previousScrollTop - st) <= 5 )
      return;

  const kiwixToolBar = document.querySelector('#kiwixtoolbar');

  if (st > previousScrollTop) {
      kiwixToolBar.style.position = 'fixed';
      kiwixToolBar.style.top = '-100%';
  } else {
      kiwixToolBar.style.position = 'static';
      kiwixToolBar.style.top = '0';
  }

  previousScrollTop = st;
}

function handle_visual_viewport_change() {
  contentIframe.height = window.visualViewport.height - contentIframe.offsetTop - 4;
}

function handle_location_hash_change() {
  const hash = window.location.hash.slice(1);
  console.log("handle_location_hash_change: " + hash);
  updateCurrentBookIfNeeded(hash);
  const iframeContentUrl = userUrl2IframeUrl(hash);
  if ( iframeContentUrl != contentIframe.contentWindow.location.pathname ) {
    contentIframe.contentWindow.location.replace(iframeContentUrl);
  }
  updateSearchBoxForLocationChange();
  previousScrollTop = Infinity;
}

function handle_content_url_change() {
  const iframeLocation = contentIframe.contentWindow.location;
  console.log('handle_content_url_change: ' + iframeLocation.href);
  document.title = contentIframe.contentDocument.title;
  const iframeContentUrl = iframeLocation.pathname;
  const iframeContentQuery = iframeLocation.search;
  const newHash = iframeUrl2UserUrl(iframeContentUrl, iframeContentQuery);
  const viewerURL = location.origin + location.pathname + location.search;
  window.location.replace(viewerURL + '#' + newHash);
  updateCurrentBookIfNeeded(newHash);
};

////////////////////////////////////////////////////////////////////////////////
// External link blocking
////////////////////////////////////////////////////////////////////////////////

function matchingAncestorElement(el, context, selector) {
  while (el && el.matches && el !== context) {
    if ( el.matches(selector) )
      return el;
    el = el.parentElement;
  }
  return null;
}

const block_path = `${root}/catch/external`;

function blockLink(target) {
  const encodedHref = encodeURIComponent(target.href);
  target.setAttribute("href", block_path + "?source=" + encodedHref);
}

function isExternalUrl(url) {
  if ( url.startsWith(window.location.origin) )
    return false;

  return url.startsWith("//")
      || url.startsWith("http:")
      || url.startsWith("https:");
}

function onClickEvent(e) {
  const iframeDocument = contentIframe.contentDocument;
  const target = matchingAncestorElement(e.target, iframeDocument, "a");
  if (target !== null && "href" in target) {
    if ( isExternalUrl(target.href) ) {
      target.setAttribute("target", "_top");
      if ( viewerSettings.linkBlockingEnabled ) {
        return blockLink(target);
      }
    }
  }
}

// helper for enabling IE 8 event bindings
function addEventHandler(el, eventType, handler) {
  if (el.attachEvent)
    el.attachEvent('on'+eventType, handler);
  else
    el.addEventListener(eventType, handler);
}

function setupEventHandler(context, selector, eventType, callback) {
  addEventHandler(context, eventType, function(e) {
    const eventElement = e.target || e.srcElement;
    const el = matchingAncestorElement(eventElement, context, selector);
    if (el)
      callback.call(el, e);
  });
}

// matches polyfill
this.Element && function(ElementPrototype) {
    ElementPrototype.matches = ElementPrototype.matches ||
    ElementPrototype.matchesSelector ||
    ElementPrototype.webkitMatchesSelector ||
    ElementPrototype.msMatchesSelector ||
    function(selector) {
        var node = this, nodes = (node.parentNode || node.document).querySelectorAll(selector), i = -1;
        while (nodes[++i] && nodes[i] != node);
        return !!nodes[i];
    }
}(Element.prototype);

function setup_external_link_blocker() {
  setupEventHandler(contentIframe.contentDocument, 'a', 'click', onClickEvent);
}

////////////////////////////////////////////////////////////////////////////////
// End of external link blocking
////////////////////////////////////////////////////////////////////////////////

function on_content_load() {
  handle_content_url_change();
  setup_external_link_blocker();
}

window.onresize = handle_visual_viewport_change;
window.onhashchange = handle_location_hash_change;

updateCurrentBook(currentBook);
handle_location_hash_change();

function htmlDecode(input) {
    var doc = new DOMParser().parseFromString(input, "text/html");
    return doc.documentElement.textContent;
}

function setupAutoHidingOfTheToolbar() {
  setInterval(updateToolbarVisibilityState, 250);
}

function setupSuggestions() {
  const kiwixSearchBox = document.querySelector('#kiwixsearchbox');
  const kiwixSearchFormWrapper = document.querySelector('.kiwix_searchform');

  autoCompleteJS = new autoComplete(
    {
      selector: "#kiwixsearchbox",
      placeHolder: kiwixSearchBox.title,
      threshold: 1,
      debounce: 300,
      data : {
        src: async (query) => {
          try {
            // Fetch Data from external Source
            const source = await fetch(`${suggestionsApiURL()}&term=${encodeURIComponent(query)}`);
            const data = await source.json();
            return data;
          } catch (error) {
            return error;
          }
        },
        keys: ['label'],
      },
      submit: true,
      searchEngine: (query, record) => {
        // We accept all records
        return true;
      },
      resultsList: {
        noResults: true,
        // We must display 10 results (requested) + 1 potential link to do a full text search.
        maxResults: 11,
      },
      resultItem: {
        element: (item, data) => {
          let searchLink;
          if (data.value.kind == "path") {
            searchLink = `${root}/${currentBook}/${htmlDecode(data.value.path)}`;
          } else {
            searchLink = `${root}/search?content=${encodeURIComponent(currentBook)}&pattern=${encodeURIComponent(htmlDecode(data.value.value))}`;
          }
          item.innerHTML = `<a class="suggest" href="javascript:gotoUrl('${searchLink}')">${htmlDecode(data.value.label)}</a>`;
        },
        highlight: "autoComplete_highlight",
        selected: "autoComplete_selected"
      }
    }
  );

  document.querySelector('#kiwixsearchform').addEventListener('submit', function(event) {
    closeSuggestions();
    try {
      const selectedElem = document.querySelector('.autoComplete_selected > a');
      if (selectedElem) {
        event.preventDefault();
        selectedElem.click();
      }
    } catch (err) {}
  });

  kiwixSearchBox.addEventListener('focus', () => {
      kiwixSearchFormWrapper.classList.add('full_width');
      document.querySelector('label[for="kiwix_button_show_toggle"]').classList.add('searching');
      document.querySelector('.kiwix_button_cont').classList.add('searching');
  });
  kiwixSearchBox.addEventListener('blur', () => {
      kiwixSearchFormWrapper.classList.remove('full_width');
      document.querySelector('label[for="kiwix_button_show_toggle"]').classList.remove('searching');
      document.querySelector('.kiwix_button_cont').classList.remove('searching');
  });
}

function setupViewer() {
  // Defer the call of handle_visual_viewport_change() until after the
  // presence or absence of the taskbar as determined by this function
  // has been settled.
  setTimeout(handle_visual_viewport_change, 0);

  const kiwixToolBarWrapper = document.getElementById('kiwixtoolbarwrapper');
  if ( ! viewerSettings.toolbarEnabled ) {
    return;
  }

  kiwixToolBarWrapper.style.display = 'block';
  if ( ! viewerSettings.libraryButtonEnabled ) {
    document.getElementById("kiwix_serve_taskbar_library_button").remove();
  }

  setupSuggestions();

  // cybook hack
  if (navigator.userAgent.indexOf("bookeen/cybook") != -1) {
      document.querySelector('html').classList.add('cybook');
  }

  if (document.body.clientWidth < 520) {
    setupAutoHidingOfTheToolbar();
  }
}
