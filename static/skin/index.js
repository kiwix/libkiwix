const viewPortHeight = window.innerHeight;
const root = $( `link[type='root']` ).attr("href");
const count = Math.floor(viewPortHeight/100 + 1) * 3;
let isFetching = false;
let isEnd = false;
let prevStart = 0;

function htmlEncode(str) {
    return str.replace(/[\u00A0-\u9999<>\&]/gim, (i) => `&#${i.charCodeAt(0)};`);
}

async function loadAndDisplay(query, append = false) {
    isFetching = true;
    await fetch(`${root}/catalog/search${query}`)
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
                <div class='book__title' title='${htmlEncode(title)}'>${htmlEncode(title)}</div>
                <div class='book__description' title='${htmlEncode(description)}'>${htmlEncode(description)}</div>
                <div class='book__info'>${htmlEncode(getInnerHtml(book, 'articleCount'))} articles, ${htmlEncode(getInnerHtml(book, 'mediaCount'))} medias</div>
                </div>
            </div></a>`;
        });
        document.querySelector('.book__list').innerHTML = (append ? document.querySelector('.book__list').innerHTML : '') + bookHtml;
        isFetching = false;
        isEnd = !books.length;
    });
}

function getInnerHtml(node, query) {
    return node.querySelector(query).innerHTML;
}

window.addEventListener('scroll', async () => {
    if (isFetching || isEnd) return;
    if (viewPortHeight + window.scrollY >= document.body.offsetHeight) {
        start = prevStart + count;
        loadAndDisplay(`?start=${start}&count=${count}`, true);
        prevStart = start;
    }
});

window.onload = async (event) => {
    loadAndDisplay(`?start=0&count=${count}`);
}