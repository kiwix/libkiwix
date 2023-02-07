import mustache from '../skin/mustache.min.js?KIWIXCACHEID'

const Translations = {
  defaultLanguage: null,
  currentLanguage: null,
  promises: {},
  data: {},

  load: function(lang, asDefault=false) {
    if ( asDefault ) {
      this.defaultLanguage = lang;
      this.loadTranslationsJSON(lang);
    } else {
      this.currentLanguage = lang;
      if ( lang != this.defaultLanguage ) {
        this.loadTranslationsJSON(lang);
      }
    }
  },

  loadTranslationsJSON: function(lang) {
    if ( this.promises[lang] )
      return;

    const errorMsg = `Error loading translations for language '${lang}': `;
    this.promises[lang] = fetch(`./skin/i18n/${lang}.json`).then(async (resp) => {
      if ( resp.ok ) {
        this.data[lang] = JSON.parse(await resp.text());
      } else {
        console.log(errorMsg + resp.statusText);
      }
    }).catch((err) => {
      console.log(errorMsg + err);
    });
  },

  whenReady: function(callback) {
    const defaultLangPromise = this.promises[this.defaultLanguage];
    const currentLangPromise = this.promises[this.currentLanguage];
    Promise.all([defaultLangPromise, currentLangPromise]).then(callback);
  },

  get: function(msgId) {
    const activeTranslation = this.data[this.currentLanguage];

    const r = activeTranslation && activeTranslation[msgId];
    if ( r )
      return r;

    const defaultMsgs = this.data[this.defaultLanguage];
    if ( defaultMsgs )
      return defaultMsgs[msgId];

    throw "Translations are not loaded";
  }
}


function $t(msgId, params={}) {
  try {
    const msgTemplate = Translations.get(msgId);
    if ( ! msgTemplate ) {
      return "Invalid message id: " + msgId;
    }

    return mustache.render(msgTemplate, params);
  } catch (err) {
    return "ERROR: " + err;
  }
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


const DEFAULT_UI_LANGUAGE = 'en';

Translations.load(DEFAULT_UI_LANGUAGE, /*asDefault=*/true);

function getUserLanguage() {
  return new URLSearchParams(window.location.search).get('userlang')
      || getCookie('userlang')
      || DEFAULT_UI_LANGUAGE;
}

function setUserLanguage(lang, callback) {
  setPermanentGlobalCookie('userlang', lang);
  Translations.load(lang);
  Translations.whenReady(callback);
}

window.$t = $t;
window.getUserLanguage = getUserLanguage;
window.setUserLanguage = setUserLanguage;
