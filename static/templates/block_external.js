  function capture(e) { $(e.target).attr("href", encodeURI("/external?source=" + e.target.href)); }
  jk( document ).ready(function() {
    jk("a").on({click: function(e) {
      if ("target" in e && "href" in e.target) {
        var href = e.target.href;
        if (href.indexOf(window.location.origin) == 0)
          return;
        if (href.substr(0, 2) == "//")
          return capture(e);
        if (href.substr(0, 5) == "http:")
          return capture(e);
        if (href.substr(0, 6) == "https:")
          return capture(e);
        return;
      }
    }});
  });
