Kiwix serve widget
====================

Introduction
------------

The kiwix-serve widget provides an easy to embed way to show the `kiwix-serve` homepage.

Usage
-----

To use the widget, simply add an iframe with its `src` attribute set to the `widget` endpoint.
Example HTML Page ::

    <!DOCTYPE html>
    <html lang="en">
    <head>
    <title>Widget Test</title>
    </head>
    <body>
    <iframe src="http://192.168.18.8:8080/widget?disabledesc&disablefilter&disabledownload" width=1000 height=1000></iframe>
    </body>
    </html>

This creates an iframe with the kiwix-serve homepage contents.

Arguments are explained below.

Possible Arguments
-------------------

Currently, the following arguments are supported.

disabledesc (value = N/A)
    Disables the description part of a tile.

disablefilter (value = N/A)
    Disables the search filters: language, category, tag and search function.

disableclick (value = N/A)
    Disables clicking the book to open it for reading.

disabledownload (value = N/A)
    Disables the download button (if avaialable at all) on the tile.


Custom CSS and JS
-----------------

You can add your custom CSS rules and Javascript code to the widget.

To do that, use the following code as template::

    <iframe id="receiver" src="http://192.168.18.8:8080/widget?disabledesc=&disablefilter=&disabledownload=" width="1000" height="1000">
    <p>Your browser does not support iframes.</p>
    </iframe>

    <script>
    window.onload = function() {
    var receiver = document.getElementById('receiver').contentWindow;
    function sendMessage() {
        let msg = {
        css: `
        .book__header {
            color:red;
        }`,
        js: `
        function widgetTest() {
        console.log("Testing widget");
        }
        widgetTest();
        `
        }
        receiver.postMessage(msg, 'http://192.168.18.8:8080/widget');
    }
    sendMessage();
    }
    </script>


The CSS/JS fields are optional, you may send both or only one.

