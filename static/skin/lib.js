function getRoot() {
  const rootlink = document.querySelector("link[type='root']");
  if (rootlink) {
    return rootlink.getAttribute("href");
  }
  if (typeof getRootLocation === "function") {
    return getRootLocation();
  }
  return "";
}

function closeModal() {
  for (modal of document.getElementsByClassName("modal-wrapper")) {
    if (modal.id == "uiLanguageSelector") {
      window.modalUILanguageSelector.close();
    } else {
      modal.remove();
    }
  }
}

function makeURLSearchString(params, keysToURIEncode) {
  let output = "";
  for (const [key, value] of params.entries()) {
    let finalValue =
      keysToURIEncode.indexOf(key) >= 0 ? encodeURIComponent(value) : value;
    output += `&${key}=${finalValue}`;
  }
  // exclude first char so the first params are not prefixed with &
  return output.substring(1);
}

async function getMagnetLink(downloadLink) {
  const magnetUrl = downloadLink + ".magnet";
  const controller = new AbortController();
  setTimeout(() => controller.abort(), 5000);
  const magnetLink = await fetch(magnetUrl, { signal: controller.signal })
    .then((response) => {
      return response.ok ? response.text() : "";
    })
    .catch((_error) => "");
  if (magnetLink) {
    return await getFixedMirrorbrainMagnet(magnetLink);
  }
  return magnetLink;
}

function downloadButtonText(zimSize) {
  return $t("download") + (zimSize ? ` - ${zimSize}` : "");
}

function insertModal(button) {
  const root = getRoot();
  const downloadSize = button.getAttribute("data-size");
  const downloadLink = button.getAttribute("data-link");
  button.addEventListener("click", async (event) => {
    event.preventDefault();
    const magnetLink = await getMagnetLink(downloadLink);
    document.body.insertAdjacentHTML(
      "beforeend",
      `<div class="modal-wrapper">
                <div class="modal">
                    <div class="modal-heading">
                        <div class="modal-title">
                            <div>
                                ${downloadButtonText(downloadSize)}
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
                                <img src="${root}/skin/download.png?KIWIXCACHEID" alt="${$t("direct-download-alt-text")}" />
                                <div>${$t("direct-download-link-text")}</div>
                            </a>
                        </div>
                        <div class="modal-regular-download">
                            <a href="${downloadLink}.sha256" download>
                                <img src="${root}/skin/hash.png?KIWIXCACHEID" alt="${$t("hash-download-alt-text")}" />
                                <div>${$t("hash-download-link-text")}</div>
                            </a>
                        </div>
                        ${
                          magnetLink
                            ? `<div class="modal-regular-download">
                            <a href="${magnetLink}" target="_blank">
                                <img src="${root}/skin/magnet.png?KIWIXCACHEID" alt="${$t("magnet-alt-text")}" />
                                <div>${$t("magnet-link-text")}</div>
                            </a>
                        </div>`
                            : ``
                        }
                        <div class="modal-regular-download">
                            <a href="${downloadLink}.torrent" download>
                                <img src="${root}/skin/bittorrent.png?KIWIXCACHEID" alt="${$t("torrent-download-alt-text")}" />
                                <div>${$t("torrent-download-link-text")}</div>
                            </a>
                        </div>
                    </div>
                </div>
            </div>`,
    );
  });
}

async function getFixedMirrorbrainMagnet(magnetLink) {
  const params = new URLSearchParams(
    magnetLink.replaceAll("&amp;", "&").replace(/^magnet:/, ""),
  );
  const zimUrl = params.get("as");
  let mirrorUrls = [];
  const metalink = await fetch(`${zimUrl}.meta4`)
    .then((response) => {
      return response.ok ? response.text() : "";
    })
    .catch((_error) => "");
  if (metalink) {
    try {
      const parser = new DOMParser();
      const doc = parser.parseFromString(metalink, "application/xml");
      doc.querySelectorAll("url").forEach((node) => {
        if (node.hasAttribute("priority")) {
          mirrorUrls.push(node.innerHTML);
        }
      });
    } catch (err) {
      console.debug(`Failed to parse mirror links for ${zimUrl}`);
    }
  }
  params.set("ws", zimUrl);
  if (mirrorUrls) {
    mirrorUrls.forEach((url) => {
      params.append("ws", url);
    });
  }
  params.set("xs", `${zimUrl}.torrent`);
  return (
    "magnet:?" + makeURLSearchString(params, ["ws", "as", "dn", "xs", "tr"])
  );
}
