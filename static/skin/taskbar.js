function htmlDecode(input) {
    var doc = new DOMParser().parseFromString(input, "text/html");
    return doc.documentElement.textContent;
}

const jq = jQuery.noConflict();
jq(document).ready(() => {
    (function ($) {
        const root = $( `link[type='root']` ).attr("href");
        const bookName = window.location.pathname.split(`${root}/`)[1].split('/')[0];
    
        $( "#kiwixsearchbox" ).autocomplete({
    
            source: `${root}/suggest?content=${bookName}`,
            dataType: "json",
            cache: false,
    
            response: function( event, ui ) {
                  for(const item of ui.content) {
                      item.label = htmlDecode(item.label);
                      item.value = htmlDecode(item.value);
                      if (item.path) item.path = htmlDecode(item.path);
                  }
            },
    
            select: function(event, ui) {
                if (ui.item.kind === 'path') {
                    window.location.href = `${root}/${bookName}/${encodeURI(ui.item.path)}`;
                } else {
                    $( "#kiwixsearchbox" ).val(ui.item.value);
                    $( "#kiwixsearchform" ).submit();
                }
            },
        });
    
        /* cybook hack */
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
    })(jq);
})