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
    var values = [];
    for (var i = 0; i < localStorage.length; i++) {
        var k = localStorage.key(i);
        if (k.indexOf("%1") === 0) {
            values.push(k.replace("%1", ""));
        }
    }
    return values;
}

function GM_setValue(aKey, aVal) {
    localStorage.setItem("%1" + aKey, aVal);
}
