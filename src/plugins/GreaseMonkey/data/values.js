// Modified from https://gist.githubusercontent.com/arantius/3123124/raw/grant-none-shim.js
//
// %1 - unique script id

function GM_deleteValue(aKey) {
    localStorage.removeItem("%1" + aKey);
}

function GM_getValue(aKey, aDefault) {
    var val = localStorage.getItem("%1" + aKey)
    if (null === val && 'undefined' != typeof aDefault) return aDefault;
    return val;
}

function GM_listValues() {
    var prefixLen = "%1".length;
    var values = [];
    var i = 0;
    for (var i = 0; i < localStorage.length; i++) {
        var k = localStorage.key(i);
        if (k.substr(0, prefixLen) === "%1") {
            values.push(k.substr(prefixLen));
        }
    }
    return values;
}

function GM_setValue(aKey, aVal) {
    localStorage.setItem("%1" + aKey, aVal);
}
