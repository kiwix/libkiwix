// Terminology
//
// user url: identifier of the page that has to be displayed in the viewer
//           and that is used as the hash component of the viewer URL. For
//           book resources the user url is {book}/{resource} .
//
// iframe url: the URL to be loaded in the viewer iframe.

let viewerState = {
  uiLanguage: 'en',
};

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

let currentBook = null;
let currentBookTitle = null;

const bookUIGroup = document.getElementById('kiwix_serve_taskbar_book_ui_group');
const homeButton = document.getElementById('kiwix_serve_taskbar_home_button');
const contentIframe = document.getElementById('content_iframe');


function gotoMainPageOfCurrentBook() {
  location.hash = currentBook + '/';
}

function gotoUrl(url) {
  contentIframe.src = root + url;
}

function gotoRandomPage() {
  gotoUrl(`/random?content=${currentBook}`);
}

function performSearch() {
  const searchbox = document.getElementById('kiwixsearchbox');
  const q = encodeURIComponent(searchbox.value);
  gotoUrl(`/search?books.name=${currentBook}&pattern=${q}`);
}

function makeJSLink(jsCodeString, linkText, linkAttr="") {
  // Values of the href attribute are assumed by the browser to be
  // fully URI-encoded (no matter what the scheme is). Therefore, in
  // order to prevent the browser from decoding any URI-encoded parts
  // in the JS code we have to URI-encode a second time.
  // (see https://stackoverflow.com/questions/33721510)
  const uriEncodedJSCode = encodeURIComponent(jsCodeString);
  return `<a ${linkAttr} href="javascript:${uriEncodedJSCode}">${linkText}</a>`;
}

function suggestionsApiURL()
{
  const uriEncodedBookName = encodeURIComponent(currentBook);
  const userLang = viewerState.uiLanguage;
  return `${root}/suggest?userlang=${userLang}&content=${uriEncodedBookName}`;
}

function setTitle(element, text) {
  if ( element ) {
    element.title = text;
    if ( element.hasAttribute("aria-label") ) {
      element.setAttribute("aria-label", text);
    }
  }
}

function setCurrentBook(book, title) {
  currentBook = book;
  currentBookTitle = title;
  setTitle(homeButton, $t("home-button-text", {BOOK_TITLE: title}));
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
    searchbox.title = $t("searchbox-tooltip", {BOOK_TITLE : currentBookTitle});
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
  const wh = window.visualViewport
           ? window.visualViewport.height
           : window.innerHeight;
  contentIframe.height = wh - contentIframe.offsetTop - 4;
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
  history.replaceState(viewerState, null);
}

function handle_content_url_change() {
  const iframeLocation = contentIframe.contentWindow.location;
  console.log('handle_content_url_change: ' + iframeLocation.href);
  document.title = contentIframe.contentDocument.title;
  const iframeContentUrl = iframeLocation.pathname;
  const iframeContentQuery = iframeLocation.search;
  const newHash = iframeUrl2UserUrl(iframeContentUrl, iframeContentQuery);
  history.replaceState(viewerState, null, makeURL(location.search, newHash));
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

function blockLink(url) {
  return viewerSettings.linkBlockingEnabled
       ? block_path + "?source=" + encodeURIComponent(url)
       : url;
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
      target.setAttribute("href", blockLink(target.href));
      contentIframe.contentWindow.parent.postMessage({ externalURL : target.href }, "*");
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

let viewerSetupComplete = false;

function on_content_load() {
  if ( viewerSetupComplete ) {
    handle_content_url_change();
    setup_external_link_blocker();
  }
}

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
          const uriEncodedBookName = encodeURIComponent(currentBook);
          let url;
          if (data.value.kind == "path") {
            const path = encodeURIComponent(htmlDecode(data.value.path));
            url = `/content/${uriEncodedBookName}/${path}`;
          } else {
            const pattern = encodeURIComponent(htmlDecode(data.value.value));
            url = `/search?content=${uriEncodedBookName}&pattern=${pattern}`;
          }
          // url can't contain any double quote and/or backslash symbols
          // since they should have been URI-encoded. Therefore putting it
          // inside double quotes should result in valid javascript.
          const jsAction = `gotoUrl("${url}")`;
          const linkText = htmlDecode(data.value.label);
          item.innerHTML = makeJSLink(jsAction, linkText, 'class="suggest"');
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

function makeURL(search, hash) {
  let url = location.origin + location.pathname;
  if (search != "") {
    url += (search[0] == '?' ? search : '?' + search);
  }

  url += (hash[0] == '#' ? hash : '#' + hash);
  return url;
}

function updateUILanguageSelector(userLang) {
  console.log(`updateUILanguageSelector(${userLang})`);
  const languageSelector = document.getElementById("ui_language");
  for (const opt of languageSelector.children ) {
    if ( opt.value == userLang ) {
      opt.selected = true;
    }
  }
}

function handle_history_state_change(event) {
  console.log(`handle_history_state_change`);
  if ( event.state ) {
    viewerState = event.state;
    updateUILanguageSelector(viewerState.uiLanguage);
    setUserLanguage(viewerState.uiLanguage, updateUIText);
  }
}

function changeUILanguage() {
  window.modalUILanguageSelector.close();
  const s = document.getElementById("ui_language");
  const lang = s.options[s.selectedIndex].value;
  viewerState.uiLanguage = lang;
  setUserLanguage(lang, () => {
    updateUIText();
    history.pushState(viewerState, null);
  });
}

function handleMessage(event) {
  console.log("handleMessage");
  if ( event.data.externalURL ) {
    window.location = event.data.externalURL;
  }
}

function setupViewer() {
  // Defer the call of handle_visual_viewport_change() until after the
  // presence or absence of the taskbar as determined by this function
  // has been settled.
  setTimeout(handle_visual_viewport_change, 0);

  window.onresize = handle_visual_viewport_change;
  window.addEventListener("message", handleMessage);

  const kiwixToolBarWrapper = document.getElementById('kiwixtoolbarwrapper');
  if ( ! viewerSettings.toolbarEnabled ) {
    return;
  }

  const lang = getUserLanguage();
  setUserLanguage(lang, finishViewerSetupOnceTranslationsAreLoaded);
  viewerState.uiLanguage = lang;
  const q = new URLSearchParams(window.location.search);
  q.delete('userlang');
  const rewrittenURL = makeURL(q.toString(), location.hash);
  history.replaceState(viewerState, null, rewrittenURL);

  kiwixToolBarWrapper.style.display = 'block';
  if ( ! viewerSettings.libraryButtonEnabled ) {
    document.getElementById("kiwix_serve_taskbar_library_button").remove();
  }

  initUILanguageSelector(viewerState.uiLanguage, changeUILanguage);
  setupSuggestions();

  // cybook hack
  if (navigator.userAgent.indexOf("bookeen/cybook") != -1) {
      document.querySelector('html').classList.add('cybook');
  }

  if (document.body.clientWidth < 520) {
    setupAutoHidingOfTheToolbar();
  }
}

function updateUIText() {
  currentBook = getBookFromUserUrl(location.hash.slice(1));
  updateCurrentBook(currentBook);

  setTitle(document.getElementById("kiwix_serve_taskbar_library_button"),
           $t("library-button-text"));

  setTitle(document.getElementById("kiwix_serve_taskbar_random_button"),
           $t("random-page-button-text"));
}

function finishViewerSetupOnceTranslationsAreLoaded()
{
  updateUIText();
  handle_location_hash_change();

  window.onhashchange = handle_location_hash_change;
  window.onpopstate = handle_history_state_change;

  viewerSetupComplete = true;
}

function setPermanentGlobalCookie(name, value) {
  document.cookie = `${name}=${value};path=${root};max-age=31536000`;
}
