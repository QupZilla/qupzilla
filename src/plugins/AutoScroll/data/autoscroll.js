/*
 * AutoScroll
 *
 * modified from https://chrome.google.com/webstore/detail/autoscroll/occjjkgifpmdgodlplnacmkejpdionan
 */

(function () {
  "use strict";

  function hypot(x, y) {
    return Math.sqrt(x * x + y * y)
  }

  var startAnimation = function (f) {
    (function anon() {
      f(requestAnimationFrame(anon))
    })()
  }
  var stopAnimation = cancelAnimationFrame

  // The timer that does the actual scrolling; must be very fast so the scrolling is smooth
  var cycle = {
    dirX: 0,
    dirY: 0,
    start: function (elem) {
      var self = this

      startAnimation(function (id) {
        self.timeout = id

        var x = self.dirX
          , y = self.dirY

        if (x !== 0) {
          elem.scrollLeft += x
        }
        if (y !== 0) {
          elem.scrollTop += y
        }
      })
    },
    stop: function () {
      this.dirX = 0
      this.dirY = 0
      stopAnimation(this.timeout)
    }
  }

  var options = {
    dragThreshold: 10,
    moveThreshold: 20,
    moveSpeed: %MOVE_SPEED%,
    stickyScroll: true,
    ctrlClick: %CTRL_CLICK%,
    middleClick: %MIDDLE_CLICK%
  }

  function make(parent, f) {
    var state = {}

    function image(o) {
      if (o.width && o.height) {
        return "data:image/png;base64,%IMG_ALL%"
      } else if (o.width) {
        return "data:image/png;base64,%IMG_HORIZONTAL%"
      } else {
        return "data:image/png;base64,%IMG_VERTICAL%"
      }
    }

    function shouldSticky(x, y) {
      return options["stickyScroll"] && hypot(x, y) < options["dragThreshold"]
    }

    function scale(value) {
      return value / options["moveSpeed"]
    }

    var e = document.createElement("iframe")
    e.style.setProperty("display", "none", "important")
    e.style.setProperty("position", "fixed", "important")
    e.style.setProperty("padding", "0px", "important")
    e.style.setProperty("margin", "0px", "important")
    e.style.setProperty("left", "0px", "important")
    e.style.setProperty("top", "0px", "important")
    e.style.setProperty("width", "100%", "important")
    e.style.setProperty("height", "100%", "important")
    e.style.setProperty("background-color", "transparent", "important")
    e.style.setProperty("z-index", "2147483647", "important") // 32-bit signed int
    e.style.setProperty("border", "none", "important")
    parent.appendChild(e)

    var inner = e.contentDocument.body

    function stopScroll(e) {
      if (e.stopImmediatePropagation) {
        e.stopImmediatePropagation()
      }
      e.stopPropagation()
    }

    function unclick() {
      cycle.stop()
      inner.style.removeProperty("cursor")
      inner.style.setProperty("display", "none", "important")
      e.style.setProperty("display", "none", "important")
      delete state.oldX
      delete state.oldY
      delete state.click

      // Force relayout
      getComputedStyle(inner).left;
      getComputedStyle(e).left;
      getComputedStyle(eCursor).left;

      removeEventListener("scroll", stopScroll, true)
    }

    inner.addEventListener("mousewheel", function (event) {
      event.preventDefault()
    }, true)

    inner.addEventListener("mousemove", function (event) {
      var x = event.clientX - state.oldX,
          y = event.clientY - state.oldY

      if (hypot(x, y) > options["moveThreshold"]) {
        x = scale(x)
        y = scale(y)

        cycle.dirX = Math.round(x) | 0
        cycle.dirY = Math.round(y) | 0
      } else {
        cycle.dirX = 0
        cycle.dirY = 0
      }
    }, true)

    inner.addEventListener("mouseup", function (event) {
      var x = event.clientX - state.oldX,
          y = event.clientY - state.oldY

      if (state.click || !shouldSticky(x, y)) {
        unclick()
      } else {
        state.click = true
      }
    }, true)

    var eCursor = document.createElement("img")
    eCursor.style.setProperty("position", "absolute")
    inner.appendChild(eCursor)

    function show(o, x, y) {
      state.oldX = x
      state.oldY = y

      addEventListener("scroll", stopScroll, true)

      eCursor.setAttribute("src", image(o))
      eCursor.style.setProperty("left", (x - 13) + "px")
      eCursor.style.setProperty("top",  (y - 13) + "px")
      inner.style.setProperty("display", "block", "important")
      e.style.setProperty("display", "block", "important")
      cycle.start(o.element)
    }

    f(show)
  }

  var htmlNode = document.documentElement

  function isInvalid(elem) {
    if (elem.localName === "input") {
      return !(elem.type === "button"   ||
               elem.type === "checkbox" ||
               elem.type === "file"     ||
               elem.type === "hidden"   ||
               elem.type === "image"    ||
               elem.type === "radio"    ||
               elem.type === "reset"    ||
               elem.type === "submit")
    } else {
      while (true) {
        if (elem === document.body || elem === htmlNode) {
          return false
        } else if (elem.localName === "a" && elem.href || elem.localName === "textarea") {
          return true
        } else {
          elem = elem.parentNode
        }
      }
    }
  }

  function canScroll(elem, style) {
    return style === "auto" || style === "scroll"
  }

  function canScrollTop(elem, style) {
    return style === "auto" || style === "scroll" || style === "visible"
  }

  function hasWidth(elem, can) {
    var style = getComputedStyle(elem)
    return can(elem, style.overflowX) && elem.scrollWidth > elem.clientWidth
  }

  function hasHeight(elem, can) {
    var style = getComputedStyle(elem)
    return can(elem, style.overflowY) && elem.scrollHeight > elem.clientHeight
  }

  function findScroll(elem) {
      while (elem !== document &&
             elem !== document.body &&
             elem !== htmlNode) {

        var width  = hasWidth(elem, canScroll)
          , height = hasHeight(elem, canScroll)

        if (width || height) {
          return {
            element: elem,
            width:   width,
            height:  height
          }

        } else {
          elem = elem.parentNode
        }
      }

    var body_width  = hasWidth(document.body, canScrollTop);
    var body_height = hasHeight(document.body, canScrollTop);

    var html_width  = hasWidth(htmlNode, canScrollTop);
    var html_height = hasHeight(htmlNode, canScrollTop);

    var width  = (body_width  || html_width);
    var height = (body_height || html_height);

    if (width || height) {
      return {
        element: document.body,
        width:   width,
        height:  height
      };
    } else {
      return null;
    }
  }

  function getBody(x) {
    if (x === null) {
      return null;

    } else {
      return {
        element: (x.element === htmlNode ? document.body : x.element),
        width:   x.width,
        height:  x.height
      }
    }
  }

  function ready(f) {
    if (document.body) {
      f()
    } else {
      var observer = new MutationObserver(function () {
        if (document.body) {
          observer.disconnect()
          f()
        }
      })
      observer.observe(htmlNode, { childList: true })
    }
  }

  ready(function () {
    make(document.body, function (show) {
      addEventListener("mousedown", function (e) {
        if (((e.button === 1 && options.middleClick) ||
             (e.button === 0 && (e.ctrlKey || e.metaKey) && options.ctrlClick)) &&
            e.clientX < htmlNode.clientWidth &&
            !isInvalid(e.target)) {
          var elem = getBody(findScroll(e.target))
          if (elem !== null) {
            if (e.stopImmediatePropagation) {
              e.stopImmediatePropagation()
            }
            e.stopPropagation()
            e.preventDefault()
            show(elem, e.clientX, e.clientY)
          }
        }
      }, true)
    })
  })
})()
