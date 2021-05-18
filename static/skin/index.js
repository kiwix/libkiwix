(function() {
    const root = $(`link[type='root']`).attr('href');
    let isFetching = false;
    const incrementalLoadingParams = {
        start: 0,
        count: viewPortToCount()
    };

    function queryUrlBuilder() {
        let url = `${root}/catalog/search?`;
        url += Object.keys(incrementalLoadingParams).map(key => `${key}=${incrementalLoadingParams[key]}`).join("&");
        return url;
    }

    function viewPortToCount(){
        return Math.floor(window.innerHeight/100 + 1)*(window.innerWidth>1000 ? 3 : 2);
    }

    function getInnerHtml(node, query) {
        return node.querySelector(query).innerHTML;
    }

    function generateBookHtml(book) {
        const link = book.querySelector('link').getAttribute('href');
        const title =  getInnerHtml(book, 'title');
        const description = getInnerHtml(book, 'summary');
        const id = getInnerHtml(book, 'id');
        const iconUrl = getInnerHtml(book, 'icon');
        const articleCount = getInnerHtml(book, 'articleCount');
        const mediaCount = getInnerHtml(book, 'mediaCount');

        return `<a href='${link}' data-id='${id}'><div class='book'>
            <div class='book__background' style="background-image: url('${iconUrl}');">
            <div class='book__title' title='${title}'>${title}</div>
            <div class='book__description' title='${description}'>${description}</div>
            <div class='book__info'>${articleCount} articles, ${mediaCount} medias</div>
            </div>
        </div></a>`;
    }

    async function loadAndDisplayBooks() {
        isFetching = true;
        fetch(queryUrlBuilder()).then(async (resp) => {
            const data = new window.DOMParser().parseFromString(await resp.text(), 'application/xml');
            const books = data.querySelectorAll('entry');
            let bookHtml = '';
            books.forEach((book) => {bookHtml += generateBookHtml(book)});
            document.querySelector('.book__list').innerHTML += bookHtml;
            incrementalLoadingParams.start += books.length;
            if (books.length < incrementalLoadingParams.count) {
                incrementalLoadingParams.count = 0;
            }
            isFetching = false;
        });
    }

    async function loadSubset() {
        if (!isFetching &&
            incrementalLoadingParams.count &&
            window.innerHeight + window.scrollY >= document.body.offsetHeight
        ) {
            loadAndDisplayBooks();
        }
    }

    window.addEventListener('resize', async () => {
        incrementalLoadingParams.count = incrementalLoadingParams.count && viewPortToCount();
        loadSubset();
    });

    window.addEventListener('scroll', loadSubset);

    window.onload = async () => {
        loadAndDisplayBooks();
    }
})();
