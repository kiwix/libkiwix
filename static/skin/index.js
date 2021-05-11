const root = $( `link[type='root']` ).attr("href");
let viewPortHeight = window.innerHeight;
let isFetching = false;
let isEnd = false;
const params = new URLSearchParams(window.location.search);
const basicConfig = {
    'start': 0,
    'count': Math.floor(viewPortHeight/100 + 1) * 3
};

function queryUrlBuilder() {
    let url = `${root}/catalog/search?`
    Object.keys(basicConfig).forEach((key, idx) => {
        url += `${key}=${basicConfig[key]}${idx !== Object.keys(basicConfig).length - 1 ? '&' : ''}`;
    });
    return url + (params.toString() ? `&${params.toString()}` : '');
}

async function loadAndDisplayBooks(append = false) {
    isFetching = true;
    await fetch(queryUrlBuilder())
    .then(async (resp) => {
        const data = new window.DOMParser().parseFromString(await resp.text(), 'application/xml');
        const books = data.querySelectorAll("entry");
        let bookHtml = '';
        books.forEach((book) => {
            const link = book.querySelector('link').getAttribute('href');
            const title =  getInnerHtml(book, 'title');
            const description = getInnerHtml(book, 'summary');
            
            bookHtml += `<a href='${link}'><div class='book'>
                <div class='book__background' style="background-image: url('${getInnerHtml(book, 'icon')}');">
                <div class='book__title' title='${title}'>${title}</div>
                <div class='book__description' title='${description}'>${description}</div>
                <div class='book__info'>${getInnerHtml(book, 'articleCount')} articles, ${getInnerHtml(book, 'mediaCount')} medias</div>
                </div>
            </div></a>`;
        });
        document.querySelector('.book__list').innerHTML = (append ? document.querySelector('.book__list').innerHTML : '') + bookHtml;
        isFetching = false;
        isEnd = !books.length;
    });
}

async function loadAndDisplayOptions(nodeQuery, query) {
    // currently taking an array in place of query, will replace it with query while fetching data from backend later on.
    query.forEach((option) => {
        let value = Object.keys(option)[0];
        let label = option[value];
        document.querySelector(nodeQuery).innerHTML += `<option value='${value}'>${label}</option>`;
    })
}

async function filterBooks(filterType, filterValue) {
    isEnd = false;
    isFetching = false;
    basicConfig['start'] = 0;
    const params = new URLSearchParams(window.location.search);
    if (!filterValue) {
       params.delete(filterType); 
    } else {
        params.set(filterType, filterValue);
    }
    window.location.search = params.toString();
    loadAndDisplayBooks();
}

function getInnerHtml(node, query) {
    return node.querySelector(query).innerHTML;
}

window.addEventListener('resize', async () => {
    if (isFetching || isEnd) return;
    viewPortHeight = window.innerHeight;
    basicConfig['count'] = Math.floor(viewPortHeight/100 + 1) * 3;
    basicConfig['start'] = basicConfig['start'] + basicConfig['count'];
    loadAndDisplayBooks(true);
});

window.addEventListener('scroll', async () => {
    if (isFetching || isEnd) return;
    if (viewPortHeight + window.scrollY >= document.body.offsetHeight) {
        basicConfig['start'] = basicConfig['start'] + basicConfig['count']; 
        loadAndDisplayBooks(true);
    }
});

window.onload = async (event) => {
    loadAndDisplayBooks();
    loadAndDisplayOptions('#languageFilter', [{'eng': 'English'}, {'fra': 'french'}, {'ara': 'arab'}, {'hin': 'Hindi'}]);
    loadAndDisplayOptions('#categoryFilter', [{'stack_exchange': 'stack exchange'}, {'wikipedia': 'wikipedia'}, {'phet': 'phet'}, {'youtube': 'youtube'}]);
    for (const key of params.keys()) {
        document.getElementsByName(key)[0].value = params.get(key);
    }
}