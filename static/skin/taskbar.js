function htmlDecode(input) {
    var doc = new DOMParser().parseFromString(input, "text/html");
    return doc.documentElement.textContent;
}

function on_ready() {
    const root = document.querySelector(`link[type='root']`).getAttribute("href");
    const bookName = (window.location.pathname == `${root}/search`)
      ? (new URLSearchParams(window.location.search)).get('content')
      : window.location.pathname.split(`${root}/`)[1].split('/')[0];

    const autoCompleteJS = new autoComplete(
      {
        selector: "#kiwixsearchbox",
        placeHolder: "Search for content ...",
        threshold: 1,
        debounce: 300,
        data : {
          src: async (query) => {
            try {
              // Fetch Data from external Source
              const source = await fetch(`${root}/suggest?content=${bookName}&term=${query}`);
              const data = await source.json();
              return data;
            } catch (error) {
              return error;
            }
          }
        },
        searchEngine: (query, record) => {
          // We accept all records
          return true;
        },
        keys: ['label'],
        resultsList: {
          noResults: true,
          /* We must display 10 results (requested) + 1 potential link to do a full text search. */
          maxResults: 11,
          highlight: true
        },
        resultItem: {
          element: (item, data) => {
            item.innerHTML = `${htmlDecode(data.value.label)}`;
          }
        }
      }
    );

  /*
        // cybook hack
        if (navigator.userAgent.indexOf("bookeen/cybook") != -1) {
            $("html").addClass("cybook");
        }
    
        if ($(window).width() < 520) {
            var didScroll;
            var lastScrollTop = 0;
            var delta = 5;
            // on scroll, let the interval function know the user has scrolled
            $(window).scroll(function (event) {
                didScroll = true;
            });
            // run hasScrolled() and reset didScroll status
            setInterval(function () {
                if (didScroll) {
                    hasScrolled();
                    didScroll = false;
                }
            }, 250);
            function hasScrolled() {
                var st = $(this).scrollTop();
    
                // Make sure they scroll more than delta
                if (Math.abs(lastScrollTop - st) <= delta)
                    return;
    
                // If they scrolled down and are past the navbar, add class .nav-up.
                // This is necessary so you never see what is "behind" the navbar.
                if (st > lastScrollTop) {
                    // Scroll Down
                    $('#kiwixtoolbar').css({ top: '-100%' });
                } else {
                    // Scroll Up
                    $('#kiwixtoolbar').css({ top: '0' });
                }
    
                lastScrollTop = st;
            }
        }
    
        $('#kiwixsearchbox').on({
            focus: function () {
                $('.kiwix_searchform').addClass('full_width');
                $('label[for="kiwix_button_show_toggle"], .kiwix_button_cont').addClass('searching');
            },
            blur: function () {
                $('.kiwix_searchform').removeClass('full_width');
                $('label[for="kiwix_button_show_toggle"], .kiwix_button_cont').removeClass('searching');
            }
        });
    };*/
}

document.addEventListener('DOMContentLoaded', on_ready);
