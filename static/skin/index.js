(function() {
    const root = $(`link[type='root']`).attr('href');
    const incrementalLoadingParams = {
        start: 0,
        count: viewPortToCount()
    };
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
    let languages = {};

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

    const humanFriendlyTitle = (title) => {
        if (typeof title === 'string' && title.length > 0) {
            title = title.replace(/_/g, ' ');
            if (title.length > 0) {
                return htmlEncode(title[0].toUpperCase() + title.slice(1));
            }
        }
        return '';
    }

    function htmlEncode(str) {
        return str.replace(/[\u00A0-\u9999<>\&]/gim, (i) => `&#${i.charCodeAt(0)};`);
    }

    function viewPortToCount(){
        const zoom = Math.floor((( window.outerWidth - 10 ) / window.innerWidth) * 100);
        return Math.floor(window.innerHeight/(3*zoom) + 1)*(window.innerWidth/(2.5*zoom) + 1);
    }

    function getInnerHtml(node, query) {
        const queryNode = node.querySelector(query);
        return queryNode != null ? queryNode.innerHTML : "";
    }

    function generateBookHtml(book, sort = false) {
        const link = book.querySelector('link[type="text/html"]').getAttribute('href');
        let iconUrl;
        book.querySelectorAll('link[rel="http://opds-spec.org/image/thumbnail"]').forEach(link => {
            if (link.getAttribute('type').split(';')[1] == 'width=48' && !iconUrl) {
                iconUrl = link.getAttribute('href');
            }
        });
        const title =  getInnerHtml(book, 'title');
        const description = getInnerHtml(book, 'summary');
        const id = getInnerHtml(book, 'id');
        const langCode = getInnerHtml(book, 'language');
        const language = languages[langCode];
        const tags = getInnerHtml(book, 'tags');
        let tagHtml = tags.split(';').filter(tag => {return !(tag.split(':')[0].startsWith('_'))})
            .map((tag) => {return tag.charAt(0).toUpperCase() + tag.slice(1)})
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
        const humanFriendlyZimSize = humanFriendlySize(zimSize);

        const divTag = document.createElement('div');
        divTag.setAttribute('class', 'book');
        divTag.setAttribute('data-id', id);
        if (sort) {
            divTag.setAttribute('data-idx', bookOrderMap.get(id));
        }
        const faviconAttr = iconUrl != undefined ? `style="background-image: url('${iconUrl}')"` : '';
        const languageAttr = langCode != '' ? `title="${language}" aria-label="${language}"` : 'style="background-color: transparent"';
        divTag.innerHTML = `<a class="book__link" href="${link}" data-hover="Preview">
            <div class="book__wrapper">
            <div class="book__icon" ${faviconAttr}></div>
            <div class="book__header">
                <div id="book__title">${title}</div>
                ${downloadLink ? `<div class="book__download"><span data-link="${downloadLink}">Download ${humanFriendlyZimSize ? ` - ${humanFriendlyZimSize}</span></div>`: ''}` : ''}
            </div>
            <div class="book__description" title="${description}">${description}</div>
            <div class="book__languageTag" ${languageAttr}>${getLanguageCodeToDisplay(langCode)}</div>
            <div class="book__tags"><div class="book__tags--wrapper">${tagHtml}</div></div>
            </div></div></a>`;
        return divTag;
    }

    function getLanguageCodeToDisplay(langCode3Letter) {
        const langCode2Letter = (Object.keys(iso6391To3).find(key => iso6391To3[key] === langCode3Letter));
        const res = (langCode2Letter != undefined) ? langCode2Letter : langCode3Letter;
        return res.toUpperCase();
    }

    function toggleFooter(show=false) {
        if (show) {
            footer.style.display = 'block';
        } else {
            footer.style.display = 'none';
            fadeOutDiv.style.display = 'block';
        }
    }

    function insertModal(button) {
        const downloadLink = button.getAttribute('data-link');
        button.addEventListener('click', (event) => {
            event.preventDefault();
            document.body.insertAdjacentHTML('beforeend', `<div class="modal-wrapper">
                <div class="modal">
                    <div class="modal-heading">
                        <div class="modal-title">
                            <div>
                                Download
                            </div>
                        </div>
                        <div onclick="closeModal()" class="modal-close-button">
                            <div>
                                <svg xmlns="http://www.w3.org/2000/svg" width="14" height="14" viewBox="0 0 14 14" fill="none">
                                    <path fill-rule="evenodd" clip-rule="evenodd" d="M13.7071 1.70711C14.0976 1.31658 14.0976 
                                    0.683417 13.7071 0.292893C13.3166 -0.0976311 12.6834 -0.0976311 12.2929 0.292893L7 5.58579L1.70711
                                    0.292893C1.31658 -0.0976311 0.683417 -0.0976311 0.292893 0.292893C-0.0976311 0.683417
                                    -0.0976311 1.31658 0.292893 1.70711L5.58579 7L0.292893 12.2929C-0.0976311 12.6834
                                    -0.0976311 13.3166 0.292893 13.7071C0.683417 14.0976 1.31658 14.0976 1.70711 13.7071L7
                                    8.41421L12.2929 13.7071C12.6834 14.0976 13.3166 14.0976 13.7071 13.7071C14.0976 13.3166 
                                    14.0976 12.6834 13.7071 12.2929L8.41421 7L13.7071 1.70711Z" fill="black" />
                                </svg>
                            </div>
                        </div>
                    </div>
                    <div class="modal-content">
                        <div class="modal-regular-download">
                            <a href="${downloadLink}" download>
                                <img src="../skin/download.png" alt="direct download" />
                                <div>Direct</div>
                            </a>
                        </div>
                        <div class="modal-regular-download">
                            <a href="${downloadLink}.sha256" download>
                                <img src="../skin/hash.png" alt="download hash" />
                                <div>Sha256 hash</div>
                            </a>
                        </div>
                        <div class="modal-regular-download">
                            <a href="${downloadLink}.magnet" target="_blank">
                                <img src="../skin/magnet.png" alt="download magnet" />
                                <div>Magnet link</div>
                            </a>
                        </div>
                        <div class="modal-regular-download">
                            <a href="${downloadLink}.torrent" download>
                                <img src="../skin/bittorrent.png" alt="download torrent" />
                                <div>Torrent file</div>
                            </a>
                        </div>
                    </div>
                </div>
            </div>`);
        })
    }

    async function getBookCount(query) {
        const url = `${root}/catalog/search?${query}`;
        return await fetch(url).then(async (resp) => {
            const data = new window.DOMParser().parseFromString(await resp.text(), 'application/xml');
            return parseInt(data.querySelector('totalResults').innerHTML);
        });
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
            const kiwixResultText = document.querySelector('.kiwixHomeBody__results')
            if (results) {
                let resultText = `${results} books`;
                if (results === 1) {
                    resultText = `${results} book`;
                }
                kiwixResultText.innerHTML = resultText;
            } else {
                kiwixResultText.innerHTML = ``;
            }
            loader.style.display = 'none';
            return books;
        });
    }

    async function loadAndDisplayOptions(nodeQuery, query, valueEntryNode) {
        await fetch(query).then(async (resp) => {
            const data = new window.DOMParser().parseFromString(await resp.text(), 'application/xml');
            let optionStr = '';
            data.querySelectorAll('entry').forEach(entry => {
                const title = getInnerHtml(entry, 'title');
                const value = getInnerHtml(entry, valueEntryNode);
                const hfTitle = humanFriendlyTitle(title);
                if (valueEntryNode == 'language') {
                    languages[value] = hfTitle;
                }
                optionStr += (hfTitle != '') ? `<option value="${value}">${hfTitle}</option>` : '';
            });
            document.querySelector(nodeQuery).innerHTML += optionStr;
        });
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
                        document.querySelectorAll('.filter').forEach(filter => {
                            filter.value = params.get(filter.name) || '';
                            if (filter.value) {
                                filter.style = 'background-color: #858585; color: #fff';
                            } else {
                                filter.style = 'background-color: #ffffff; color: black';
                            }
                        })
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
        books.forEach((book) => {
            iso.insert(generateBookHtml(book, sort))
            const downloadButton = document.querySelector(`[data-id="${getInnerHtml(book, 'id')}"] .book__download span`);
            if (downloadButton) {
                insertModal(downloadButton);
            }
        });
    }

    async function resetAndFilter(filterType = '', filterValue = '') {
        isFetching = false;
        incrementalLoadingParams.start = 0;
        incrementalLoadingParams.count = viewPortToCount();
        fadeOutDiv.style.display = 'none';
        bookOrderMap.clear();
        params = new URLSearchParams(window.location.search);
        if (filterType) {
            params.set(filterType, filterValue);
            window.history.pushState({}, null, `${window.location.href.split('?')[0]}?${params.toString()}`);
            setCookie(filterCookieName, params.toString());
        }
        document.querySelectorAll('.filter').forEach(filter => {
            if (filter.value) {
                filter.style = 'background-color: #858585; color: #fff';
            } else {
                filter.style = 'background-color: #ffffff; color: black';
            }
        });
        await loadAndDisplayBooks(true);
    }

    window.addEventListener('popstate', async () => {
        await resetAndFilter();
        document.querySelectorAll('.filter').forEach(filter => {filter.value = params.get(filter.name) || ''});
    });

    async function loadSubset() {
        if (window.innerHeight + window.scrollY >= (document.body.offsetHeight * 0.98)) {
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
        await loadAndDisplayOptions('#languageFilter', `${root}/catalog/v2/languages`, 'language');
        await loadAndDisplayOptions('#categoryFilter', `${root}/catalog/v2/categories`, 'title');
        await loadAndDisplayBooks();
        document.querySelectorAll('.filter').forEach(filter => {
            filter.addEventListener('change', () => {resetAndFilter(filter.name, filter.value)});
        });
        if (filters) {
            window.history.pushState({}, null, `${window.location.href.split('?')[0]}?${params.toString()}`);
        }
        params.forEach((value, key) => {
            const selectBox = document.getElementsByName(key)[0];
            if (selectBox) {
                selectBox.value = value
            }
        });
        document.getElementById('kiwixSearchForm').onsubmit = (event) => {event.preventDefault()};
        if (!window.location.search) {
            const browserLang = navigator.language.split('-')[0];
            const langFilter = document.getElementById('languageFilter');
            const lang = browserLang.length === 3 ? browserLang : iso6391To3[browserLang];
            if (await getBookCount(`lang=${lang}`)) {
                langFilter.value = lang;
                langFilter.dispatchEvent(new Event('change'));
            }
        }
        document.querySelectorAll('.filter').forEach(filter => {
            if (filter.value) {
                filter.style = 'background-color: #858585; color: #fff';
            } else {
                filter.style = 'background-color: #ffffff; color: black';
            }
        });
        setCookie(filterCookieName, params.toString());
    }
})();
