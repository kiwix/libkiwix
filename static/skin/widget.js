function disableSearchFilters(widgetStyles) {
    const hideNavRule = `
    .kiwixNav {
        display: none;
    }`;
    const hideResultsLabelRule = `
    .kiwixHomeBody__results {
        display: none;
    }`;
    const hideTagFilterRule = `
    .book__tags {
        pointer-events: none;
    }`;
    insertNewCssRules(widgetStyles, [hideNavRule, hideResultsLabelRule, hideTagFilterRule]);
}

function disableBookClick() {
    kiwixServe.disableBookClick();
}

function disableDownload(widgetStyles) {
    const hideBookDownloadRule = `
    .book__download {
        display: none;
    }`;
    insertNewCssRules(widgetStyles, [hideBookDownloadRule]);
}

function disableDescription(widgetStyles) {
    const decreaseHeightRule = `
    .book__wrapper {
        height:128px;
        grid-template-rows: 70px 0 1fr 1fr;
    }`;
    const hideDescRule = `
    .book__description {
        display: none;
    }`;
    insertNewCssRules(widgetStyles, [decreaseHeightRule, hideDescRule]);
}

function hideFooter(widgetStyles) {
    const hideFooterRule = `
    .kiwixfooter {
        display: none !important;
    }`;
    insertNewCssRules(widgetStyles, [hideFooterRule]);
}

function insertNewCssRules(stylesheet, ruleList) {
    if (stylesheet) {
        for (rule of ruleList) {
            stylesheet.insertRule(rule, 0);
        }
    }
}

function addCustomCss(cssCode) {
    let customCSS = document.createElement('style');
    customCSS.innerHTML = cssCode;
    document.head.appendChild(customCSS);
}

function addCustomJs(jsCode) {
    new Function(`"use strict";${jsCode}`)();
}

function handleMessages(event) {
    if ('css' in event.data) {
        addCustomCss(event.data.css);
    }
    if ('js' in event.data) {
        addCustomJs(event.data.js);
    }
}

function handleWidget() {
    const params = new URLSearchParams(window.location.search || filters || '');
    const widgetStyleElem = document.createElement('style');
    document.head.appendChild(widgetStyleElem);

    const widgetStyles = widgetStyleElem.sheet;

    const disableFilters = params.has('disablefilter');
    const disableClick = params.has('disableclick');
    const disableDwld = params.has('disabledownload');
    const disableDesc = params.has('disabledesc');

    const blankBase = document.createElement('base');
    blankBase.target = '_blank';
    document.head.appendChild(blankBase); // open all links in new tab

    if (disableFilters) 
        disableSearchFilters(widgetStyles);
    if (disableClick)
        disableBookClick();
    if (disableDwld)
        disableDownload(widgetStyles);
    if (disableDesc)
        disableDescription(widgetStyles);

    hideFooter(widgetStyles);
    kiwixServe.updateBookCount();
}

window.addEventListener('message', handleMessages);
handleWidget();