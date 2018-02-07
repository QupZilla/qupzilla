var GM = {
    info: {
        script: {
            description: "",
            excludes: [],
            includes: [],
            matches: [],
            name: "",
            namespace: "",
            resources: {},
            'run-at': "document-end",
            version: ""
        },
        scriptMetaStr: "",
        scriptHandler: "QupZilla GreaseMonkey",
        version: "4.0"
    }
};
window.GM = GM;

function GM_info() {
    return GM.info;
}

function GM_xmlhttpRequest(/* object */ details) {
    details.method = details.method.toUpperCase() || "GET";

    if (!details.url) {
        throw("GM_xmlhttpRequest requires an URL.");
    }

    // build XMLHttpRequest object
    var oXhr = new XMLHttpRequest;
    // run it
    if("onreadystatechange" in details)
        oXhr.onreadystatechange = function() { details.onreadystatechange(oXhr) };
    if("onload" in details)
        oXhr.onload = function() { details.onload(oXhr) };
    if("onerror" in details)
        oXhr.onerror = function() { details.onerror(oXhr) };

    oXhr.open(details.method, details.url, true);

    if("headers" in details)
        for(var header in details.headers)
            oXhr.setRequestHeader(header, details.headers[header]);

    if("data" in details)
        oXhr.send(details.data);
    else
        oXhr.send();
}

function GM_addStyle(/* string */ styles) {
    var head = document.getElementsByTagName("head")[0];
    if (head === undefined) {
        document.onreadystatechange = function() {
            if (document.readyState == "interactive") {
                var oStyle = document.createElement("style");
                oStyle.setAttribute("type", "text/css");
                oStyle.appendChild(document.createTextNode(styles));
                document.getElementsByTagName("head")[0].appendChild(oStyle);
            }
        }
    }
    else {
        var oStyle = document.createElement("style");
        oStyle.setAttribute("type", "text/css");
        oStyle.appendChild(document.createTextNode(styles));
        head.appendChild(oStyle);
    }
}

function GM_log(log) {
    if(console)
        console.log(log);
}

function GM_openInTab(url) {
    return window.open(url);
}

function GM_setClipboard(text) {
    external.extra.greasemonkey.setClipboard(text);
}

// GM_registerMenuCommand not supported
function GM_registerMenuCommand(caption, commandFunc, accessKey) { }

// GM_getResourceUrl not supported
function GM_getResourceUrl(resourceName) { }

// GreaseMonkey 4.0 support
GM.openInTab = GM_openInTab;
GM.setClipboard = GM_setClipboard;
GM.xmlhttpRequest = GM_xmlhttpRequest;

// GM_getResourceUrl not supported
GM.getResourceUrl = function(resourceName) {
    return new Promise((resolve, reject) => {
        reject();
    });
};
