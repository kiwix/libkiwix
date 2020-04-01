var block_path = "/catch/external";
// called only on external links
function capture_event(e, target) { target.setAttribute("href", encodeURI(block_path + "?source=" + target.href)); }

// called on all link clicks. filters external and call capture_event
function on_click_event(e) {
  var target = findParent("a", e.target);
  if (target !== null && "href" in target) {
    var href = target.href;
    if (window.location.pathname.indexOf(block_path) == 0) // already in catch page
      return;
    if (href.indexOf(window.location.origin) == 0)
      return;
    if (href.substr(0, 2) == "//")
      return capture_event(e, target);
    if (href.substr(0, 5) == "http:")
      return capture_event(e, target);
    if (href.substr(0, 6) == "https:")
      return capture_event(e, target);
    return;
  }
}

// script entrypoint (called on document ready)
function run() { live('a', 'click', on_click_event); }

// find first parent with tagname
function findParent(tagname, el) {
  while (el) {
    if ((el.nodeName || el.tagName).toLowerCase() === tagname.toLowerCase()) {
      return el;
    }
    el = el.parentNode;
  }
  return null;
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

// helper for enabling IE 8 event bindings
function addEvent(el, type, handler) {
    if (el.attachEvent) el.attachEvent('on'+type, handler); else el.addEventListener(type, handler);
}

// live binding helper using matchesSelector
function live(selector, event, callback, context) {
    addEvent(context || document, event, function(e) {
        var found, el = e.target || e.srcElement;
        while (el && el.matches && el !== context && !(found = el.matches(selector))) el = el.parentElement;
        if (found) callback.call(el, e);
    });
}

// in case the document is already rendered
if (document.readyState!='loading') run();
// modern browsers
else if (document.addEventListener) document.addEventListener('DOMContentLoaded', run);
// IE <= 8
else document.attachEvent('onreadystatechange', function(){
    if (document.readyState=='complete') run();
});
