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

// GreaseMonkey 4.0 support
var asyncCall = (func) => {
    if (window._qupzilla_external) {
        func();
    } else {
        document.addEventListener("_qupzilla_external_created", func);
    }
};

var decode = (val) => {
    val = String(val);
    if (!val.length) {
        return val;
    }
    var v = val.substr(1);
    if (val[0] == "b") {
        return Boolean(v == "true" ? true : false);
    } else if (val[0] == "i") {
        return Number(v);
    } else if (val[0] == "s") {
        return v;
    } else {
        return undefined;
    }
};

var encode = (val) => {
    if (typeof val == "boolean") {
        return "b" + (val ? "true" : "false");
    } else if (typeof val == "number") {
        return "i" + String(val);
    } else if (typeof val == "string") {
        return "s" + val;
    } else {
        return "";
    }
};

GM.deleteValue = function(name) {
    return new Promise((resolve, reject) => {
        asyncCall(() => {
            external.extra.greasemonkey.deleteValue("%1", name, (res) => {
                if (res) {
                    resolve();
                } else {
                    reject();
                }
            });
        });
    });
};

GM.getValue = function(name, value) {
    return new Promise((resolve) => {
        asyncCall(() => {
            external.extra.greasemonkey.getValue("%1", name, encode(value), (res) => {
                resolve(decode(res));
            });
        });
    });
};

GM.setValue = function(name, value) {
    return new Promise((resolve, reject) => {
        asyncCall(() => {
            external.extra.greasemonkey.setValue("%1", name, encode(value), (res) => {
                if (res) {
                    resolve();
                } else {
                    reject();
                }
            });
        });
    });
};

GM.listValues = function() {
    return new Promise((resolve) => {
        asyncCall(() => {
            external.extra.greasemonkey.listValues("%1", resolve);
        });
    });
};
