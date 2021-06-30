(function() {
    const root = $(`link[type='root']`).attr('href');
    const incrementalLoadingParams = {
        start: 0,
        count: viewPortToCount()
    };
    const filterTypes = ['lang', 'category', 'q'];
    const bookOrderMap = new Map();
    const filterCookieName = 'filters';
    const oneDayDelta = 86400000;
    let loader;
    let footer;
    let fadeOutDiv;
    let iso;
    let isFetching = false;
    let noResultInjected = false;
    let filters = getCookie(filterCookieName);
    let params = new URLSearchParams(window.location.search || filters || '');
    let timer;

    function queryUrlBuilder() {
        let url = `${root}/catalog/search?`;
        url += Object.keys(incrementalLoadingParams).map(key => `${key}=${incrementalLoadingParams[key]}`).join("&");
        params.forEach((value, key) => {url+= value ? `&${key}=${value}` : ''});
        return (url);
    }

    function setCookie(cookieName, cookieValue) {
        const date = new Date();
        date.setTime(date.getTime() + oneDayDelta);
        document.cookie = `${cookieName}=${cookieValue};expires=${date.toUTCString()};sameSite=Strict`;
    }

    function getCookie(cookieName) {
        const name = cookieName + "=";
        let result;
        decodeURIComponent(document.cookie).split('; ').forEach(val => {
            if (val.indexOf(name) === 0) {
                result = val.substring(name.length);
            }
        });
        return result;
    }

    const humanFriendlySize = (fileSize) => {
        if (fileSize === 0) {
            return '';
        }
        const units = ['bytes', 'kB', 'MB', 'GB', 'TB'];
        let quotient = Math.floor(Math.log10(fileSize) / 3);
        quotient = quotient < units.length ? quotient : units.length - 1;
        fileSize /= (1000 ** quotient);
        return `${+fileSize.toFixed(2)} ${units[quotient]}`;
      };      

    function htmlEncode(str) {
        return str.replace(/[\u00A0-\u9999<>\&]/gim, (i) => `&#${i.charCodeAt(0)};`);
    }

    function viewPortToCount(){
        return Math.floor(window.innerHeight/300 + 1)*(window.innerWidth>1000 ? 4 : 3);
    }

    function getInnerHtml(node, query) {
        return node.querySelector(query).innerHTML;
    }

    function generateBookHtml(book, sort = false) {
        const link = book.querySelector('link').getAttribute('href');
        const title =  getInnerHtml(book, 'title');
        const description = getInnerHtml(book, 'summary');
        const id = getInnerHtml(book, 'id');
        const iconUrl = getInnerHtml(book, 'icon');
        const language = getInnerHtml(book, 'language');
        const tags = getInnerHtml(book, 'tags');
        let tagHtml = tags.split(';').filter(tag => {return !(tag.split(':')[0].startsWith('_'))})
            .join(' | ').replace(/_/g, ' ');
        let downloadLink;
        let zimSize = 0;
        try {
            const downloadBookLink = book.querySelector('link[type="application/x-zim"]')
            zimSize = parseInt(downloadBookLink.getAttribute('length'));
            downloadLink = downloadBookLink.getAttribute('href').split('.meta4')[0];
        } catch {
            downloadLink = '';
        }
        humanFriendlyZimSize = humanFriendlySize(zimSize);

        const divTag = document.createElement('div');
        divTag.setAttribute('class', 'book');
        divTag.setAttribute('data-id', id);
        if (sort) {
            divTag.setAttribute('data-idx', bookOrderMap.get(id));
        }
        divTag.innerHTML = `<div class="book__wrapper"><div class='book__icon' ><img class="book__icon--image" src='${iconUrl}'></div>
            <div class='book__title' title='${title}'>
                <div id="bookTitle">${title}</div>
                ${humanFriendlyZimSize ? `<div id='bookSize'>${humanFriendlyZimSize}</div>`: ''}
            </div>
            <div class='book__description' title='${description}'>${description}</div>
            <div class='book__languageTag'>${language.substr(0, 2).toUpperCase()}</div>
            <div class='book__tags'><div class="book__tags--wrapper">${tagHtml}</div></div>
            <div class='book__links'> <a href="${link}" data-hover="Preview">Preview</a>${downloadLink ? `&nbsp;|&nbsp;<a href="${downloadLink}" data-hove="Download" download>Download</a>` : ''} </div></div>`;
        return divTag;
    }

    function toggleFooter(show=false) {
        if (show) {
            footer.style.display = 'block';
        } else {
            footer.style.display = 'none';
            fadeOutDiv.style.display = 'block';
        }
    }

    async function loadBooks() {
        loader.style.display = 'block';
        return await fetch(queryUrlBuilder()).then(async (resp) => {
            const data = new window.DOMParser().parseFromString(await resp.text(), 'application/xml');
            const books = data.querySelectorAll('entry');
            books.forEach((book, idx) => {
                bookOrderMap.set(getInnerHtml(book, 'id'), idx);
            });
            incrementalLoadingParams.start += books.length;
            const results = parseInt(data.querySelector('totalResults').innerHTML)
            if (results === bookOrderMap.size) {
                incrementalLoadingParams.count = 0;
                toggleFooter(true);
            } else {
                toggleFooter();
            }
            document.querySelector('.kiwixHomeBody__results').innerHTML = `${results} books`
            loader.style.display = 'none';
            return books;
        });
    }

    async function loadAndDisplayOptions(nodeQuery, query) {
        // currently taking an object in place of query, will replace it with query while fetching data from backend later on.
        document.querySelector(nodeQuery).innerHTML += Object.keys(query)
            .map((option) => {return `<option value='${option}'>${htmlEncode(query[option])}</option>`})
        .join('');
    }

    function checkAndInjectEmptyMessage() {
        const kiwixHomeBody = document.querySelector('.kiwixHomeBody');
        if (!bookOrderMap.size) {
            if (!noResultInjected) {
                noResultInjected = true;
                iso.remove(document.getElementsByClassName('book__list')[0].getElementsByTagName('div'));
                iso.layout();
                setTimeout(() => {
                    const divTag = document.createElement('div');
                    divTag.setAttribute('class', 'noResults');
                    divTag.innerHTML = `No result. Would you like to <a href="/?lang=">reset filter</a>?`;
                    kiwixHomeBody.append(divTag);
                    kiwixHomeBody.setAttribute('style', 'display: flex; justify-content: center; align-items: center');
                    divTag.getElementsByTagName('a')[0].onclick = (event) => {
                        event.preventDefault();
                        window.history.pushState({}, null, `${window.location.href.split('?')[0]}?lang=`);
                        setCookie(filterCookieName, 'lang=');
                        resetAndFilter();
                        filterTypes.forEach(key => {document.getElementsByName(key)[0].value = params.get(key) || ''});
                    };
                    loader.setAttribute('style', 'position: absolute; top: 50%');
                }, 300);
            }
            return true;
        } else if (noResultInjected) {
            noResultInjected = false;
            document.getElementsByClassName('noResults')[0].remove();
            kiwixHomeBody.removeAttribute('style');
        }
        loader.removeAttribute('style');
        return false;
    }

    async function loadAndDisplayBooks(sort = false) {
        if (isFetching) return;
        isFetching = true;
        await loadAndDisplayBooksUnguarded(sort);
        isFetching = false;
    }

    async function loadAndDisplayBooksUnguarded(sort) {
        let books = await loadBooks();
        if (checkAndInjectEmptyMessage()) {return}
        const booksToFilter = new Set();
        const booksToDelete = new Set();
        iso.arrange({
            filter: function (idx, elem) {
                const id = elem.getAttribute('data-id');
                const retVal = bookOrderMap.has(id);
                if (retVal) {
                    booksToFilter.add(id);
                    if (sort) {
                        elem.setAttribute('data-idx', bookOrderMap.get(id));
                        iso.updateSortData(elem);
                    }
                } else {
                    booksToDelete.add(elem);
                }
                return retVal;
            }
        });
        books = [...books].filter((book) => {return !booksToFilter.has(getInnerHtml(book, 'id'))});
        booksToDelete.forEach(book => {iso.remove(book);});
        books.forEach((book) => {iso.insert(generateBookHtml(book, sort))});
    }

    async function resetAndFilter(filterType = '', filterValue = '') {
        isFetching = false;
        incrementalLoadingParams.start = 0;
        incrementalLoadingParams.count = viewPortToCount();
        fadeOutDiv.style.display = 'none';
        bookOrderMap.clear();
        params = new URLSearchParams(window.location.search);
        if (filterType) {
            const filter = document.getElementsByName(filterType)[0];
            if (filterValue) {
                filter.style = 'background-color: #4f6478; color: #fff';
            } else {
                filter.style = 'background-color: #ffffff; color: black';
            }
            params.set(filterType, filterValue);
            window.history.pushState({}, null, `${window.location.href.split('?')[0]}?${params.toString()}`);
            setCookie(filterCookieName, params.toString());
        }
        await loadAndDisplayBooks(true);
    }

    window.addEventListener('popstate', async () => {
        await resetAndFilter();
        filterTypes.forEach(key => {document.getElementsByName(key)[0].value = params.get(key) || ''});
    });

    async function loadSubset() {
        if (window.innerHeight + window.scrollY >= document.body.offsetHeight) {
            if (incrementalLoadingParams.count) {
                loadAndDisplayBooks();
            }
            else {
                fadeOutDiv.style.display = 'none';
            }
        }
    }

    window.addEventListener('resize', (event) => {
        if (timer) {clearTimeout(timer)}
        timer = setTimeout(() => {
            incrementalLoadingParams.count = incrementalLoadingParams.count && viewPortToCount();
            loadSubset();
        }, 100, event);
    });

    window.addEventListener('scroll', loadSubset);

    window.onload = async () => {
        iso = new Isotope( '.book__list', {
            itemSelector: '.book',
            getSortData:{
                weight: function( itemElem ) {
                    const index = itemElem.getAttribute('data-idx');
                    return index ? parseInt(index) : Infinity;
                }
            },
            sortBy: 'weight',
            layoutMode: 'cellsByRow',
            cellsByRow: {
                columnWidth: '.book',
                rowHeight: '.book'
            }
        });
        footer = document.getElementById('kiwixfooter');
        fadeOutDiv = document.getElementById('fadeOut');
        loader = document.querySelector('.loader');
        await loadAndDisplayBooks();
        await loadAndDisplayOptions('#languageFilter', langList);
        await loadAndDisplayOptions('#categoryFilter', categoryList);
        filterTypes.forEach((filter) => {
            const filterTag = document.getElementsByName(filter)[0];
            filterTag.addEventListener('change', () => {resetAndFilter(filterTag.name, filterTag.value)});
        });
        if (filters) {
            window.history.pushState({}, null, `${window.location.href.split('?')[0]}?${params.toString()}`);
        }
        params.forEach((value, key) => {document.getElementsByName(key)[0].value = value});
        document.getElementById('kiwixSearchForm').onsubmit = (event) => {event.preventDefault()};
        if (!window.location.search) {
            const browserLang = navigator.language.split('-')[0];
            const langFilter = document.getElementById('languageFilter');
            langFilter.value = browserLang.length === 3 ? browserLang : iso6391To3[browserLang];
            langFilter.dispatchEvent(new Event('change'));
        }
        setCookie(filterCookieName, params.toString());
    }
})();
