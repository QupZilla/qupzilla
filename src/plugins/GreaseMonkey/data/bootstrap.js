if(typeof GM_xmlhttpRequest === "undefined") {
    GM_xmlhttpRequest = function(/* object */ details) {
        details.method = details.method.toUpperCase() || "GET";

        if(!details.url) {
            throw("GM_xmlhttpRequest requires an URL.");
        }

        // build XMLHttpRequest object
        var oXhr = new XMLHttpRequest;
        // run it
        if(oXhr) {
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
        } else
            throw ("This Browser is not supported, please upgrade.")
    }
}

if(typeof GM_addStyle === "undefined") {
    function GM_addStyle(/* String */ styles) {
        var head = document.getElementsByTagName("head")[0];
        if (head === undefined) {
            document.onreadystatechange = function() {
                if (document.readyState == "interactive") {
                    var oStyle = document.createElement("style");
                    oStyle.setAttribute("type", "text\/css");
                    oStyle.appendChild(document.createTextNode(styles));
                    document.getElementsByTagName("head")[0].appendChild(oStyle);
                }
            }
        }
        else {
            var oStyle = document.createElement("style");
            oStyle.setAttribute("type", "text\/css");
            oStyle.appendChild(document.createTextNode(styles));
            head.appendChild(oStyle);
        }
    }
}

if(typeof GM_log === "undefined") {
    function GM_log(log) {
        if(console)
            console.log(log);
    }
}

if(typeof GM_openInTab === "undefined") {
    function GM_openInTab(url) {
        return window.open(url);
    }
}

if(typeof GM_setClipboard === "undefined") {
    function GM_setClipboard(text) {
        window._qz_greasemonkey.setClipboard(text);
    }
}

// GM Settings Impl
if(typeof GM_getValueImpl === "undefined") {
    function GM_getValueImpl(namespace, name, value) {
        return window._qz_greasemonkey.getValue(namespace, name, value);
    }
}

if(typeof GM_setValueImpl === "undefined") {
    function GM_setValueImpl(namespace, name, value) {
        window._qz_greasemonkey.setValue(namespace, name, value);
    }
}

if(typeof GM_deleteValueImpl === "undefined") {
    function GM_deleteValueImpl(namespace, name) {
        window._qz_greasemonkey.deleteValue(namespace, name);
    }
}

if(typeof GM_listValuesImpl === "undefined") {
    function GM_listValuesImpl(namespace) {
        return window._qz_greasemonkey.listValues(namespace);
    }
}

// Define unsafe window
var unsafeWindow = window;
window.wrappedJSObject = unsafeWindow;

// GM_registerMenuCommand not supported
if(typeof GM_registerMenuCommand === "undefined") {
    function GM_registerMenuCommand(caption, commandFunc, accessKey) { }
}

// GM Resource not supported
if(typeof GM_getResourceText === "undefined") {
    function GM_getResourceText(resourceName) {
        throw ("QupZilla: GM Resource is not supported!");
    }
}

if(typeof GM_getResourceURL === "undefined") {
    function GM_getResourceURL(resourceName) {
        throw ("QupZilla: GM Resource is not supported!");
    }
}
