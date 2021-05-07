function htmlEncode(str) {
    return str.replace(/[\u00A0-\u9999<>\&]/gim, (i) => `&#${i.charCodeAt(0)};`);
}

window.onload = async (event) => {
    const root = $( `link[type='root']` ).attr("href");
    await fetch(`${root}/catalog/search`)
    .then(async (resp) => {
        const data = new window.DOMParser().parseFromString(await resp.text(), 'application/xml');
        const entries = data.querySelectorAll("entry");
        displayBooks(entries);
    });
};

function getInnerHtml(node, query) {
    return node.querySelector(query).innerHTML;
}

function displayBooks(books) {
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
    document.querySelector('.book__list').innerHTML = bookHtml;
}