! function(a, b) {
  "function" == typeof define && define.amd ? define(b) : "object" == typeof exports ? module.exports = b() : a.PhotoSwipe = b()
}(this, function() {
  "use strict";
  var a = function(a, b, c, d) {
      var e = {
          features: null,
          bind: function(a, b, c, d) {
              var e = (d ? "remove" : "add") + "EventListener";
              b = b.split(" ");
              for (var f = 0; f < b.length; f++) b[f] && a[e](b[f], c, !1)
          },
          isArray: function(a) {
              return a instanceof Array
          },
          createEl: function(a, b) {
              var c = document.createElement(b || "div");
              return a && (c.className = a), c
          },
          getScrollY: function() {
              var a = window.pageYOffset;
              return void 0 !== a ? a : document.documentElement.scrollTop
          },
          unbind: function(a, b, c) {
              e.bind(a, b, c, !0)
          },
          removeClass: function(a, b) {
              var c = new RegExp("(\\s|^)" + b + "(\\s|$)");
              a.className = a.className.replace(c, " ").replace(/^\s\s*/, "").replace(/\s\s*$/, "")
          },
          addClass: function(a, b) {
              e.hasClass(a, b) || (a.className += (a.className ? " " : "") + b)
          },
          hasClass: function(a, b) {
              return a.className && new RegExp("(^|\\s)" + b + "(\\s|$)").test(a.className)
          },
          getChildByClass: function(a, b) {
              for (var c = a.firstChild; c;) {
                  if (e.hasClass(c, b)) return c;
                  c = c.nextSibling
              }
          },
          arraySearch: function(a, b, c) {
              for (var d = a.length; d--;)
                  if (a[d][c] === b) return d;
              return -1
          },
          extend: function(a, b, c) {
              for (var d in b)
                  if (b.hasOwnProperty(d)) {
                      if (c && a.hasOwnProperty(d)) continue;
                      a[d] = b[d]
                  }
          },
          easing: {
              sine: {
                  out: function(a) {
                      return Math.sin(a * (Math.PI / 2))
                  },
                  inOut: function(a) {
                      return -(Math.cos(Math.PI * a) - 1) / 2
                  }
              },
              cubic: {
                  out: function(a) {
                      return --a * a * a + 1
                  }
              }
          },
          detectFeatures: function() {
              if (e.features) return e.features;
              var a = e.createEl(),
                  b = a.style,
                  c = "",
                  d = {};
              if (d.oldIE = document.all && !document.addEventListener, d.touch = "ontouchstart" in window, window.requestAnimationFrame && (d.raf = window.requestAnimationFrame, d.caf = window.cancelAnimationFrame), d.pointerEvent = navigator.pointerEnabled || navigator.msPointerEnabled, !d.pointerEvent) {
                  var f = navigator.userAgent;
                  if (/iP(hone|od)/.test(navigator.platform)) {
                      var g = navigator.appVersion.match(/OS (\d+)_(\d+)_?(\d+)?/);
                      g && g.length > 0 && (g = parseInt(g[1], 10), g >= 1 && g < 8 && (d.isOldIOSPhone = !0))
                  }
                  var h = f.match(/Android\s([0-9\.]*)/),
                      i = h ? h[1] : 0;
                  i = parseFloat(i), i >= 1 && (i < 4.4 && (d.isOldAndroid = !0), d.androidVersion = i), d.isMobileOpera = /opera mini|opera mobi/i.test(f)
              }
              for (var j, k, l = ["transform", "perspective", "animationName"], m = ["", "webkit", "Moz", "ms", "O"], n = 0; n < 4; n++) {
                  c = m[n];
                  for (var o = 0; o < 3; o++) j = l[o], k = c + (c ? j.charAt(0).toUpperCase() + j.slice(1) : j), !d[j] && k in b && (d[j] = k);
                  c && !d.raf && (c = c.toLowerCase(), d.raf = window[c + "RequestAnimationFrame"], d.raf && (d.caf = window[c + "CancelAnimationFrame"] || window[c + "CancelRequestAnimationFrame"]))
              }
              if (!d.raf) {
                  var p = 0;
                  d.raf = function(a) {
                      var b = (new Date).getTime(),
                          c = Math.max(0, 16 - (b - p)),
                          d = window.setTimeout(function() {
                              a(b + c)
                          }, c);
                      return p = b + c, d
                  }, d.caf = function(a) {
                      clearTimeout(a)
                  }
              }
              return d.svg = !!document.createElementNS && !!document.createElementNS("http://www.w3.org/2000/svg", "svg").createSVGRect, e.features = d, d
          }
      };
      e.detectFeatures(), e.features.oldIE && (e.bind = function(a, b, c, d) {
          b = b.split(" ");
          for (var e, f = (d ? "detach" : "attach") + "Event", g = function() {
                  c.handleEvent.call(c)
              }, h = 0; h < b.length; h++)
              if (e = b[h])
                  if ("object" == typeof c && c.handleEvent) {
                      if (d) {
                          if (!c["oldIE" + e]) return !1
                      } else c["oldIE" + e] = g;
                      a[f]("on" + e, c["oldIE" + e])
                  } else a[f]("on" + e, c)
      });
      var f = this,
          g = 25,
          h = 3,
          i = {
              allowPanToNext: !0,
              spacing: .12,
              bgOpacity: 1,
              mouseUsed: !1,
              loop: !0,
              pinchToClose: !0,
              closeOnScroll: !0,
              closeOnVerticalDrag: !0,
              verticalDragRange: .75,
              hideAnimationDuration: 333,
              showAnimationDuration: 333,
              showHideOpacity: !1,
              focus: !0,
              escKey: !0,
              arrowKeys: !0,
              mainScrollEndFriction: .35,
              panEndFriction: .35,
              isClickableElement: function(a) {
                  return "A" === a.tagName
              },
              getDoubleTapZoom: function(a, b) {
                  return a ? 1 : b.initialZoomLevel < .7 ? 1 : 1.33
              },
              maxSpreadZoom: 1.33,
              modal: !0,
              scaleMode: "fit"
          };
      e.extend(i, d);
      var j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z, $, _, aa, ba, ca, da, ea, fa, ga, ha, ia, ja, ka, la = function() {
              return {
                  x: 0,
                  y: 0
              }
          },
          ma = la(),
          na = la(),
          oa = la(),
          pa = {},
          qa = 0,
          ra = {},
          sa = la(),
          ta = 0,
          ua = !0,
          va = [],
          wa = {},
          xa = !1,
          ya = function(a, b) {
              e.extend(f, b.publicMethods), va.push(a)
          },
          za = function(a) {
              var b = _b();
              return a > b - 1 ? a - b : a < 0 ? b + a : a
          },
          Aa = {},
          Ba = function(a, b) {
              return Aa[a] || (Aa[a] = []), Aa[a].push(b)
          },
          Ca = function(a) {
              var b = Aa[a];
              if (b) {
                  var c = Array.prototype.slice.call(arguments);
                  c.shift();
                  for (var d = 0; d < b.length; d++) b[d].apply(f, c)
              }
          },
          Da = function() {
              return (new Date).getTime()
          },
          Ea = function(a) {
              ia = a, f.bg.style.opacity = a * i.bgOpacity
          },
          Fa = function(a, b, c, d, e) {
              (!xa || e && e !== f.currItem) && (d /= e ? e.fitRatio : f.currItem.fitRatio), a[E] = u + b + "px, " + c + "px" + v + " scale(" + d + ")"
          },
          Ga = function(a) {
              da && (a && (s > f.currItem.fitRatio ? xa || (lc(f.currItem, !1, !0), xa = !0) : xa && (lc(f.currItem), xa = !1)), Fa(da, oa.x, oa.y, s))
          },
          Ha = function(a) {
              a.container && Fa(a.container.style, a.initialPosition.x, a.initialPosition.y, a.initialZoomLevel, a)
          },
          Ia = function(a, b) {
              b[E] = u + a + "px, 0px" + v
          },
          Ja = function(a, b) {
              if (!i.loop && b) {
                  var c = m + (sa.x * qa - a) / sa.x,
                      d = Math.round(a - sb.x);
                  (c < 0 && d > 0 || c >= _b() - 1 && d < 0) && (a = sb.x + d * i.mainScrollEndFriction)
              }
              sb.x = a, Ia(a, n)
          },
          Ka = function(a, b) {
              var c = tb[a] - ra[a];
              return na[a] + ma[a] + c - c * (b / t)
          },
          La = function(a, b) {
              a.x = b.x, a.y = b.y, b.id && (a.id = b.id)
          },
          Ma = function(a) {
              a.x = Math.round(a.x), a.y = Math.round(a.y)
          },
          Na = null,
          Oa = function() {
              Na && (e.unbind(document, "mousemove", Oa), e.addClass(a, "pswp--has_mouse"), i.mouseUsed = !0, Ca("mouseUsed")), Na = setTimeout(function() {
                  Na = null
              }, 100)
          },
          Pa = function() {
              e.bind(document, "keydown", f), N.transform && e.bind(f.scrollWrap, "click", f), i.mouseUsed || e.bind(document, "mousemove", Oa), e.bind(window, "resize scroll", f), Ca("bindEvents")
          },
          Qa = function() {
              e.unbind(window, "resize", f), e.unbind(window, "scroll", r.scroll), e.unbind(document, "keydown", f), e.unbind(document, "mousemove", Oa), N.transform && e.unbind(f.scrollWrap, "click", f), U && e.unbind(window, p, f), Ca("unbindEvents")
          },
          Ra = function(a, b) {
              var c = hc(f.currItem, pa, a);
              return b && (ca = c), c
          },
          Sa = function(a) {
              return a || (a = f.currItem), a.initialZoomLevel
          },
          Ta = function(a) {
              return a || (a = f.currItem), a.w > 0 ? i.maxSpreadZoom : 1
          },
          Ua = function(a, b, c, d) {
              return d === f.currItem.initialZoomLevel ? (c[a] = f.currItem.initialPosition[a], !0) : (c[a] = Ka(a, d), c[a] > b.min[a] ? (c[a] = b.min[a], !0) : c[a] < b.max[a] && (c[a] = b.max[a], !0))
          },
          Va = function() {
              if (E) {
                  var b = N.perspective && !G;
                  return u = "translate" + (b ? "3d(" : "("), void(v = N.perspective ? ", 0px)" : ")")
              }
              E = "left", e.addClass(a, "pswp--ie"), Ia = function(a, b) {
                  b.left = a + "px"
              }, Ha = function(a) {
                  var b = a.fitRatio > 1 ? 1 : a.fitRatio,
                      c = a.container.style,
                      d = b * a.w,
                      e = b * a.h;
                  c.width = d + "px", c.height = e + "px", c.left = a.initialPosition.x + "px", c.top = a.initialPosition.y + "px"
              }, Ga = function() {
                  if (da) {
                      var a = da,
                          b = f.currItem,
                          c = b.fitRatio > 1 ? 1 : b.fitRatio,
                          d = c * b.w,
                          e = c * b.h;
                      a.width = d + "px", a.height = e + "px", a.left = oa.x + "px", a.top = oa.y + "px"
                  }
              }
          },
          Wa = function(a) {
              var b = "";
              i.escKey && 27 === a.keyCode ? b = "close" : i.arrowKeys && (37 === a.keyCode ? b = "prev" : 39 === a.keyCode && (b = "next")), b && (a.ctrlKey || a.altKey || a.shiftKey || a.metaKey || (a.preventDefault ? a.preventDefault() : a.returnValue = !1, f[b]()))
          },
          Xa = function(a) {
              a && (X || W || ea || S) && (a.preventDefault(), a.stopPropagation())
          },
          Ya = function() {
              f.setScrollOffset(0, e.getScrollY())
          },
          Za = {},
          $a = 0,
          _a = function(a) {
              Za[a] && (Za[a].raf && I(Za[a].raf), $a--, delete Za[a])
          },
          ab = function(a) {
              Za[a] && _a(a), Za[a] || ($a++, Za[a] = {})
          },
          bb = function() {
              for (var a in Za) Za.hasOwnProperty(a) && _a(a)
          },
          cb = function(a, b, c, d, e, f, g) {
              var h, i = Da();
              ab(a);
              var j = function() {
                  if (Za[a]) {
                      if (h = Da() - i, h >= d) return _a(a), f(c), void(g && g());
                      f((c - b) * e(h / d) + b), Za[a].raf = H(j)
                  }
              };
              j()
          },
          db = {
              shout: Ca,
              listen: Ba,
              viewportSize: pa,
              options: i,
              isMainScrollAnimating: function() {
                  return ea
              },
              getZoomLevel: function() {
                  return s
              },
              getCurrentIndex: function() {
                  return m
              },
              isDragging: function() {
                  return U
              },
              isZooming: function() {
                  return _
              },
              setScrollOffset: function(a, b) {
                  ra.x = a, M = ra.y = b, Ca("updateScrollOffset", ra)
              },
              applyZoomPan: function(a, b, c, d) {
                  oa.x = b, oa.y = c, s = a, Ga(d)
              },
              init: function() {
                  if (!j && !k) {
                      var c;
                      f.framework = e, f.template = a, f.bg = e.getChildByClass(a, "pswp__bg"), J = a.className, j = !0, N = e.detectFeatures(), H = N.raf, I = N.caf, E = N.transform, L = N.oldIE, f.scrollWrap = e.getChildByClass(a, "pswp__scroll-wrap"), f.container = e.getChildByClass(f.scrollWrap, "pswp__container"), n = f.container.style, f.itemHolders = y = [{
                          el: f.container.children[0],
                          wrap: 0,
                          index: -1
                      }, {
                          el: f.container.children[1],
                          wrap: 0,
                          index: -1
                      }, {
                          el: f.container.children[2],
                          wrap: 0,
                          index: -1
                      }], y[0].el.style.display = y[2].el.style.display = "none", Va(), r = {
                          resize: f.updateSize,
                          scroll: Ya,
                          keydown: Wa,
                          click: Xa
                      };
                      var d = N.isOldIOSPhone || N.isOldAndroid || N.isMobileOpera;
                      for (N.animationName && N.transform && !d || (i.showAnimationDuration = i.hideAnimationDuration = 0), c = 0; c < va.length; c++) f["init" + va[c]]();
                      if (b) {
                          var g = f.ui = new b(f, e);
                          g.init()
                      }
                      Ca("firstUpdate"), m = m || i.index || 0, (isNaN(m) || m < 0 || m >= _b()) && (m = 0), f.currItem = $b(m), (N.isOldIOSPhone || N.isOldAndroid) && (ua = !1), a.setAttribute("aria-hidden", "false"), i.modal && (ua ? a.style.position = "fixed" : (a.style.position = "absolute", a.style.top = e.getScrollY() + "px")), void 0 === M && (Ca("initialLayout"), M = K = e.getScrollY());
                      var l = "pswp--open ";
                      for (i.mainClass && (l += i.mainClass + " "), i.showHideOpacity && (l += "pswp--animate_opacity "), l += G ? "pswp--touch" : "pswp--notouch", l += N.animationName ? " pswp--css_animation" : "", l += N.svg ? " pswp--svg" : "", e.addClass(a, l), f.updateSize(), o = -1, ta = null, c = 0; c < h; c++) Ia((c + o) * sa.x, y[c].el.style);
                      L || e.bind(f.scrollWrap, q, f), Ba("initialZoomInEnd", function() {
                          f.setContent(y[0], m - 1), f.setContent(y[2], m + 1), y[0].el.style.display = y[2].el.style.display = "block", i.focus && a.focus(), Pa()
                      }), f.setContent(y[1], m), f.updateCurrItem(), Ca("afterInit"), ua || (w = setInterval(function() {
                          $a || U || _ || s !== f.currItem.initialZoomLevel || f.updateSize()
                      }, 1e3)), e.addClass(a, "pswp--visible")
                  }
              },
              close: function() {
                  j && (j = !1, k = !0, Ca("close"), Qa(), bc(f.currItem, null, !0, f.destroy))
              },
              destroy: function() {
                  Ca("destroy"), Wb && clearTimeout(Wb), a.setAttribute("aria-hidden", "true"), a.className = J, w && clearInterval(w), e.unbind(f.scrollWrap, q, f), e.unbind(window, "scroll", f), yb(), bb(), Aa = null
              },
              panTo: function(a, b, c) {
                  c || (a > ca.min.x ? a = ca.min.x : a < ca.max.x && (a = ca.max.x), b > ca.min.y ? b = ca.min.y : b < ca.max.y && (b = ca.max.y)), oa.x = a, oa.y = b, Ga()
              },
              handleEvent: function(a) {
                  a = a || window.event, r[a.type] && r[a.type](a)
              },
              goTo: function(a) {
                  a = za(a);
                  var b = a - m;
                  ta = b, m = a, f.currItem = $b(m), qa -= b, Ja(sa.x * qa), bb(), ea = !1, f.updateCurrItem()
              },
              next: function() {
                  f.goTo(m + 1)
              },
              prev: function() {
                  f.goTo(m - 1)
              },
              updateCurrZoomItem: function(a) {
                  if (a && Ca("beforeChange", 0), y[1].el.children.length) {
                      var b = y[1].el.children[0];
                      da = e.hasClass(b, "pswp__zoom-wrap") ? b.style : null
                  } else da = null;
                  ca = f.currItem.bounds, t = s = f.currItem.initialZoomLevel, oa.x = ca.center.x, oa.y = ca.center.y, a && Ca("afterChange")
              },
              invalidateCurrItems: function() {
                  x = !0;
                  for (var a = 0; a < h; a++) y[a].item && (y[a].item.needsUpdate = !0)
              },
              updateCurrItem: function(a) {
                  if (0 !== ta) {
                      var b, c = Math.abs(ta);
                      if (!(a && c < 2)) {
                          f.currItem = $b(m), xa = !1, Ca("beforeChange", ta), c >= h && (o += ta + (ta > 0 ? -h : h), c = h);
                          for (var d = 0; d < c; d++) ta > 0 ? (b = y.shift(), y[h - 1] = b, o++, Ia((o + 2) * sa.x, b.el.style), f.setContent(b, m - c + d + 1 + 1)) : (b = y.pop(), y.unshift(b), o--, Ia(o * sa.x, b.el.style), f.setContent(b, m + c - d - 1 - 1));
                          if (da && 1 === Math.abs(ta)) {
                              var e = $b(z);
                              e.initialZoomLevel !== s && (hc(e, pa), lc(e), Ha(e))
                          }
                          ta = 0, f.updateCurrZoomItem(), z = m, Ca("afterChange")
                      }
                  }
              },
              updateSize: function(b) {
                  if (!ua && i.modal) {
                      var c = e.getScrollY();
                      if (M !== c && (a.style.top = c + "px", M = c), !b && wa.x === window.innerWidth && wa.y === window.innerHeight) return;
                      wa.x = window.innerWidth, wa.y = window.innerHeight, a.style.height = wa.y + "px"
                  }
                  if (pa.x = f.scrollWrap.clientWidth, pa.y = f.scrollWrap.clientHeight, Ya(), sa.x = pa.x + Math.round(pa.x * i.spacing), sa.y = pa.y, Ja(sa.x * qa), Ca("beforeResize"), void 0 !== o) {
                      for (var d, g, j, k = 0; k < h; k++) d = y[k], Ia((k + o) * sa.x, d.el.style), j = m + k - 1, i.loop && _b() > 2 && (j = za(j)), g = $b(j), g && (x || g.needsUpdate || !g.bounds) ? (f.cleanSlide(g), f.setContent(d, j), 1 === k && (f.currItem = g, f.updateCurrZoomItem(!0)), g.needsUpdate = !1) : d.index === -1 && j >= 0 && f.setContent(d, j), g && g.container && (hc(g, pa), lc(g), Ha(g));
                      x = !1
                  }
                  t = s = f.currItem.initialZoomLevel, ca = f.currItem.bounds, ca && (oa.x = ca.center.x, oa.y = ca.center.y, Ga(!0)), Ca("resize")
              },
              zoomTo: function(a, b, c, d, f) {
                  b && (t = s, tb.x = Math.abs(b.x) - oa.x, tb.y = Math.abs(b.y) - oa.y, La(na, oa));
                  var g = Ra(a, !1),
                      h = {};
                  Ua("x", g, h, a), Ua("y", g, h, a);
                  var i = s,
                      j = {
                          x: oa.x,
                          y: oa.y
                      };
                  Ma(h);
                  var k = function(b) {
                      1 === b ? (s = a, oa.x = h.x, oa.y = h.y) : (s = (a - i) * b + i, oa.x = (h.x - j.x) * b + j.x, oa.y = (h.y - j.y) * b + j.y), f && f(b), Ga(1 === b)
                  };
                  c ? cb("customZoomTo", 0, 1, c, d || e.easing.sine.inOut, k) : k(1)
              }
          },
          eb = 30,
          fb = 10,
          gb = {},
          hb = {},
          ib = {},
          jb = {},
          kb = {},
          lb = [],
          mb = {},
          nb = [],
          ob = {},
          pb = 0,
          qb = la(),
          rb = 0,
          sb = la(),
          tb = la(),
          ub = la(),
          vb = function(a, b) {
              return a.x === b.x && a.y === b.y
          },
          wb = function(a, b) {
              return Math.abs(a.x - b.x) < g && Math.abs(a.y - b.y) < g
          },
          xb = function(a, b) {
              return ob.x = Math.abs(a.x - b.x), ob.y = Math.abs(a.y - b.y), Math.sqrt(ob.x * ob.x + ob.y * ob.y)
          },
          yb = function() {
              Y && (I(Y), Y = null)
          },
          zb = function() {
              U && (Y = H(zb), Pb())
          },
          Ab = function() {
              return !("fit" === i.scaleMode && s === f.currItem.initialZoomLevel)
          },
          Bb = function(a, b) {
              return !(!a || a === document) && (!(a.getAttribute("class") && a.getAttribute("class").indexOf("pswp__scroll-wrap") > -1) && (b(a) ? a : Bb(a.parentNode, b)))
          },
          Cb = {},
          Db = function(a, b) {
              return Cb.prevent = !Bb(a.target, i.isClickableElement), Ca("preventDragEvent", a, b, Cb), Cb.prevent
          },
          Eb = function(a, b) {
              return b.x = a.pageX, b.y = a.pageY, b.id = a.identifier, b
          },
          Fb = function(a, b, c) {
              c.x = .5 * (a.x + b.x), c.y = .5 * (a.y + b.y)
          },
          Gb = function(a, b, c) {
              if (a - P > 50) {
                  var d = nb.length > 2 ? nb.shift() : {};
                  d.x = b, d.y = c, nb.push(d), P = a
              }
          },
          Hb = function() {
              var a = oa.y - f.currItem.initialPosition.y;
              return 1 - Math.abs(a / (pa.y / 2))
          },
          Ib = {},
          Jb = {},
          Kb = [],
          Lb = function(a) {
              for (; Kb.length > 0;) Kb.pop();
              return F ? (ka = 0, lb.forEach(function(a) {
                  0 === ka ? Kb[0] = a : 1 === ka && (Kb[1] = a), ka++
              })) : a.type.indexOf("touch") > -1 ? a.touches && a.touches.length > 0 && (Kb[0] = Eb(a.touches[0], Ib), a.touches.length > 1 && (Kb[1] = Eb(a.touches[1], Jb))) : (Ib.x = a.pageX, Ib.y = a.pageY, Ib.id = "", Kb[0] = Ib), Kb
          },
          Mb = function(a, b) {
              var c, d, e, g, h = 0,
                  j = oa[a] + b[a],
                  k = b[a] > 0,
                  l = sb.x + b.x,
                  m = sb.x - mb.x;
              return c = j > ca.min[a] || j < ca.max[a] ? i.panEndFriction : 1, j = oa[a] + b[a] * c, !i.allowPanToNext && s !== f.currItem.initialZoomLevel || (da ? "h" !== fa || "x" !== a || W || (k ? (j > ca.min[a] && (c = i.panEndFriction, h = ca.min[a] - j, d = ca.min[a] - na[a]), (d <= 0 || m < 0) && _b() > 1 ? (g = l, m < 0 && l > mb.x && (g = mb.x)) : ca.min.x !== ca.max.x && (e = j)) : (j < ca.max[a] && (c = i.panEndFriction, h = j - ca.max[a], d = na[a] - ca.max[a]), (d <= 0 || m > 0) && _b() > 1 ? (g = l, m > 0 && l < mb.x && (g = mb.x)) : ca.min.x !== ca.max.x && (e = j))) : g = l, "x" !== a) ? void(ea || Z || s > f.currItem.fitRatio && (oa[a] += b[a] * c)) : (void 0 !== g && (Ja(g, !0), Z = g !== mb.x), ca.min.x !== ca.max.x && (void 0 !== e ? oa.x = e : Z || (oa.x += b.x * c)), void 0 !== g)
          },
          Nb = function(a) {
              if (!("mousedown" === a.type && a.button > 0)) {
                  if (Zb) return void a.preventDefault();
                  if (!T || "mousedown" !== a.type) {
                      if (Db(a, !0) && a.preventDefault(), Ca("pointerDown"), F) {
                          var b = e.arraySearch(lb, a.pointerId, "id");
                          b < 0 && (b = lb.length), lb[b] = {
                              x: a.pageX,
                              y: a.pageY,
                              id: a.pointerId
                          }
                      }
                      var c = Lb(a),
                          d = c.length;
                      $ = null, bb(), U && 1 !== d || (U = ga = !0, e.bind(window, p, f), R = ja = ha = S = Z = X = V = W = !1, fa = null, Ca("firstTouchStart", c), La(na, oa), ma.x = ma.y = 0, La(jb, c[0]), La(kb, jb), mb.x = sa.x * qa, nb = [{
                          x: jb.x,
                          y: jb.y
                      }], P = O = Da(), Ra(s, !0), yb(), zb()), !_ && d > 1 && !ea && !Z && (t = s, W = !1, _ = V = !0, ma.y = ma.x = 0, La(na, oa), La(gb, c[0]), La(hb, c[1]), Fb(gb, hb, ub), tb.x = Math.abs(ub.x) - oa.x, tb.y = Math.abs(ub.y) - oa.y, aa = ba = xb(gb, hb))
                  }
              }
          },
          Ob = function(a) {
              if (a.preventDefault(), F) {
                  var b = e.arraySearch(lb, a.pointerId, "id");
                  if (b > -1) {
                      var c = lb[b];
                      c.x = a.pageX, c.y = a.pageY
                  }
              }
              if (U) {
                  var d = Lb(a);
                  if (fa || X || _) $ = d;
                  else if (sb.x !== sa.x * qa) fa = "h";
                  else {
                      var f = Math.abs(d[0].x - jb.x) - Math.abs(d[0].y - jb.y);
                      Math.abs(f) >= fb && (fa = f > 0 ? "h" : "v", $ = d)
                  }
              }
          },
          Pb = function() {
              if ($) {
                  var a = $.length;
                  if (0 !== a)
                      if (La(gb, $[0]), ib.x = gb.x - jb.x, ib.y = gb.y - jb.y, _ && a > 1) {
                          if (jb.x = gb.x, jb.y = gb.y, !ib.x && !ib.y && vb($[1], hb)) return;
                          La(hb, $[1]), W || (W = !0, Ca("zoomGestureStarted"));
                          var b = xb(gb, hb),
                              c = Ub(b);
                          c > f.currItem.initialZoomLevel + f.currItem.initialZoomLevel / 15 && (ja = !0);
                          var d = 1,
                              e = Sa(),
                              g = Ta();
                          if (c < e)
                              if (i.pinchToClose && !ja && t <= f.currItem.initialZoomLevel) {
                                  var h = e - c,
                                      j = 1 - h / (e / 1.2);
                                  Ea(j), Ca("onPinchClose", j), ha = !0
                              } else d = (e - c) / e, d > 1 && (d = 1), c = e - d * (e / 3);
                          else c > g && (d = (c - g) / (6 * e), d > 1 && (d = 1), c = g + d * e);
                          d < 0 && (d = 0), aa = b, Fb(gb, hb, qb), ma.x += qb.x - ub.x, ma.y += qb.y - ub.y, La(ub, qb), oa.x = Ka("x", c), oa.y = Ka("y", c), R = c > s, s = c, Ga()
                      } else {
                          if (!fa) return;
                          if (ga && (ga = !1, Math.abs(ib.x) >= fb && (ib.x -= $[0].x - kb.x), Math.abs(ib.y) >= fb && (ib.y -= $[0].y - kb.y)), jb.x = gb.x, jb.y = gb.y, 0 === ib.x && 0 === ib.y) return;
                          if ("v" === fa && i.closeOnVerticalDrag && !Ab()) {
                              ma.y += ib.y, oa.y += ib.y;
                              var k = Hb();
                              return S = !0, Ca("onVerticalDrag", k), Ea(k), void Ga()
                          }
                          Gb(Da(), gb.x, gb.y), X = !0, ca = f.currItem.bounds;
                          var l = Mb("x", ib);
                          l || (Mb("y", ib), Ma(oa), Ga())
                      }
              }
          },
          Qb = function(a) {
              if (N.isOldAndroid) {
                  if (T && "mouseup" === a.type) return;
                  a.type.indexOf("touch") > -1 && (clearTimeout(T), T = setTimeout(function() {
                      T = 0
                  }, 600))
              }
              Ca("pointerUp"), Db(a, !1) && a.preventDefault();
              var b;
              if (F) {
                  var c = e.arraySearch(lb, a.pointerId, "id");
                  if (c > -1)
                      if (b = lb.splice(c, 1)[0], navigator.pointerEnabled) b.type = a.pointerType || "mouse";
                      else {
                          var d = {
                              4: "mouse",
                              2: "touch",
                              3: "pen"
                          };
                          b.type = d[a.pointerType], b.type || (b.type = a.pointerType || "mouse")
                      }
              }
              var g, h = Lb(a),
                  j = h.length;
              if ("mouseup" === a.type && (j = 0), 2 === j) return $ = null, !0;
              1 === j && La(kb, h[0]), 0 !== j || fa || ea || (b || ("mouseup" === a.type ? b = {
                  x: a.pageX,
                  y: a.pageY,
                  type: "mouse"
              } : a.changedTouches && a.changedTouches[0] && (b = {
                  x: a.changedTouches[0].pageX,
                  y: a.changedTouches[0].pageY,
                  type: "touch"
              })), Ca("touchRelease", a, b));
              var k = -1;
              if (0 === j && (U = !1, e.unbind(window, p, f), yb(), _ ? k = 0 : rb !== -1 && (k = Da() - rb)), rb = 1 === j ? Da() : -1, g = k !== -1 && k < 150 ? "zoom" : "swipe", _ && j < 2 && (_ = !1, 1 === j && (g = "zoomPointerUp"), Ca("zoomGestureEnded")), $ = null, X || W || ea || S)
                  if (bb(), Q || (Q = Rb()), Q.calculateSwipeSpeed("x"), S) {
                      var l = Hb();
                      if (l < i.verticalDragRange) f.close();
                      else {
                          var m = oa.y,
                              n = ia;
                          cb("verticalDrag", 0, 1, 300, e.easing.cubic.out, function(a) {
                              oa.y = (f.currItem.initialPosition.y - m) * a + m, Ea((1 - n) * a + n), Ga()
                          }), Ca("onVerticalDrag", 1)
                      }
                  } else {
                      if ((Z || ea) && 0 === j) {
                          var o = Tb(g, Q);
                          if (o) return;
                          g = "zoomPointerUp"
                      }
                      if (!ea) return "swipe" !== g ? void Vb() : void(!Z && s > f.currItem.fitRatio && Sb(Q))
                  }
          },
          Rb = function() {
              var a, b, c = {
                  lastFlickOffset: {},
                  lastFlickDist: {},
                  lastFlickSpeed: {},
                  slowDownRatio: {},
                  slowDownRatioReverse: {},
                  speedDecelerationRatio: {},
                  speedDecelerationRatioAbs: {},
                  distanceOffset: {},
                  backAnimDestination: {},
                  backAnimStarted: {},
                  calculateSwipeSpeed: function(d) {
                      nb.length > 1 ? (a = Da() - P + 50, b = nb[nb.length - 2][d]) : (a = Da() - O, b = kb[d]), c.lastFlickOffset[d] = jb[d] - b, c.lastFlickDist[d] = Math.abs(c.lastFlickOffset[d]), c.lastFlickDist[d] > 20 ? c.lastFlickSpeed[d] = c.lastFlickOffset[d] / a : c.lastFlickSpeed[d] = 0, Math.abs(c.lastFlickSpeed[d]) < .1 && (c.lastFlickSpeed[d] = 0), c.slowDownRatio[d] = .95, c.slowDownRatioReverse[d] = 1 - c.slowDownRatio[d], c.speedDecelerationRatio[d] = 1
                  },
                  calculateOverBoundsAnimOffset: function(a, b) {
                      c.backAnimStarted[a] || (oa[a] > ca.min[a] ? c.backAnimDestination[a] = ca.min[a] : oa[a] < ca.max[a] && (c.backAnimDestination[a] = ca.max[a]), void 0 !== c.backAnimDestination[a] && (c.slowDownRatio[a] = .7, c.slowDownRatioReverse[a] = 1 - c.slowDownRatio[a], c.speedDecelerationRatioAbs[a] < .05 && (c.lastFlickSpeed[a] = 0, c.backAnimStarted[a] = !0, cb("bounceZoomPan" + a, oa[a], c.backAnimDestination[a], b || 300, e.easing.sine.out, function(b) {
                          oa[a] = b, Ga()
                      }))))
                  },
                  calculateAnimOffset: function(a) {
                      c.backAnimStarted[a] || (c.speedDecelerationRatio[a] = c.speedDecelerationRatio[a] * (c.slowDownRatio[a] + c.slowDownRatioReverse[a] - c.slowDownRatioReverse[a] * c.timeDiff / 10), c.speedDecelerationRatioAbs[a] = Math.abs(c.lastFlickSpeed[a] * c.speedDecelerationRatio[a]), c.distanceOffset[a] = c.lastFlickSpeed[a] * c.speedDecelerationRatio[a] * c.timeDiff, oa[a] += c.distanceOffset[a])
                  },
                  panAnimLoop: function() {
                      if (Za.zoomPan && (Za.zoomPan.raf = H(c.panAnimLoop), c.now = Da(), c.timeDiff = c.now - c.lastNow, c.lastNow = c.now, c.calculateAnimOffset("x"), c.calculateAnimOffset("y"), Ga(), c.calculateOverBoundsAnimOffset("x"), c.calculateOverBoundsAnimOffset("y"), c.speedDecelerationRatioAbs.x < .05 && c.speedDecelerationRatioAbs.y < .05)) return oa.x = Math.round(oa.x), oa.y = Math.round(oa.y), Ga(), void _a("zoomPan")
                  }
              };
              return c
          },
          Sb = function(a) {
              return a.calculateSwipeSpeed("y"), ca = f.currItem.bounds, a.backAnimDestination = {}, a.backAnimStarted = {}, Math.abs(a.lastFlickSpeed.x) <= .05 && Math.abs(a.lastFlickSpeed.y) <= .05 ? (a.speedDecelerationRatioAbs.x = a.speedDecelerationRatioAbs.y = 0, a.calculateOverBoundsAnimOffset("x"), a.calculateOverBoundsAnimOffset("y"), !0) : (ab("zoomPan"), a.lastNow = Da(), void a.panAnimLoop())
          },
          Tb = function(a, b) {
              var c;
              ea || (pb = m);
              var d;
              if ("swipe" === a) {
                  var g = jb.x - kb.x,
                      h = b.lastFlickDist.x < 10;
                  g > eb && (h || b.lastFlickOffset.x > 20) ? d = -1 : g < -eb && (h || b.lastFlickOffset.x < -20) && (d = 1)
              }
              var j;
              d && (m += d, m < 0 ? (m = i.loop ? _b() - 1 : 0, j = !0) : m >= _b() && (m = i.loop ? 0 : _b() - 1, j = !0), j && !i.loop || (ta += d, qa -= d, c = !0));
              var k, l = sa.x * qa,
                  n = Math.abs(l - sb.x);
              return c || l > sb.x == b.lastFlickSpeed.x > 0 ? (k = Math.abs(b.lastFlickSpeed.x) > 0 ? n / Math.abs(b.lastFlickSpeed.x) : 333, k = Math.min(k, 400), k = Math.max(k, 250)) : k = 333, pb === m && (c = !1), ea = !0, Ca("mainScrollAnimStart"), cb("mainScroll", sb.x, l, k, e.easing.cubic.out, Ja, function() {
                  bb(), ea = !1, pb = -1, (c || pb !== m) && f.updateCurrItem(), Ca("mainScrollAnimComplete")
              }), c && f.updateCurrItem(!0), c
          },
          Ub = function(a) {
              return 1 / ba * a * t
          },
          Vb = function() {
              var a = s,
                  b = Sa(),
                  c = Ta();
              s < b ? a = b : s > c && (a = c);
              var d, g = 1,
                  h = ia;
              return ha && !R && !ja && s < b ? (f.close(), !0) : (ha && (d = function(a) {
                  Ea((g - h) * a + h)
              }), f.zoomTo(a, 0, 200, e.easing.cubic.out, d), !0)
          };
      ya("Gestures", {
          publicMethods: {
              initGestures: function() {
                  var a = function(a, b, c, d, e) {
                      A = a + b, B = a + c, C = a + d, D = e ? a + e : ""
                  };
                  F = N.pointerEvent, F && N.touch && (N.touch = !1), F ? navigator.pointerEnabled ? a("pointer", "down", "move", "up", "cancel") : a("MSPointer", "Down", "Move", "Up", "Cancel") : N.touch ? (a("touch", "start", "move", "end", "cancel"), G = !0) : a("mouse", "down", "move", "up"), p = B + " " + C + " " + D, q = A, F && !G && (G = navigator.maxTouchPoints > 1 || navigator.msMaxTouchPoints > 1), f.likelyTouchDevice = G, r[A] = Nb, r[B] = Ob, r[C] = Qb, D && (r[D] = r[C]), N.touch && (q += " mousedown", p += " mousemove mouseup", r.mousedown = r[A], r.mousemove = r[B], r.mouseup = r[C]), G || (i.allowPanToNext = !1)
              }
          }
      });
      var Wb, Xb, Yb, Zb, $b, _b, ac, bc = function(b, c, d, g) {
              Wb && clearTimeout(Wb), Zb = !0, Yb = !0;
              var h;
              b.initialLayout ? (h = b.initialLayout, b.initialLayout = null) : h = i.getThumbBoundsFn && i.getThumbBoundsFn(m);
              var j = d ? i.hideAnimationDuration : i.showAnimationDuration,
                  k = function() {
                      _a("initialZoom"), d ? (f.template.removeAttribute("style"), f.bg.removeAttribute("style")) : (Ea(1), c && (c.style.display = "block"), e.addClass(a, "pswp--animated-in"), Ca("initialZoom" + (d ? "OutEnd" : "InEnd"))), g && g(), Zb = !1
                  };
              if (!j || !h || void 0 === h.x) return Ca("initialZoom" + (d ? "Out" : "In")), s = b.initialZoomLevel, La(oa, b.initialPosition), Ga(), a.style.opacity = d ? 0 : 1, Ea(1), void(j ? setTimeout(function() {
                  k()
              }, j) : k());
              var n = function() {
                  var c = l,
                      g = !f.currItem.src || f.currItem.loadError || i.showHideOpacity;
                  b.miniImg && (b.miniImg.style.webkitBackfaceVisibility = "hidden"), d || (s = h.w / b.w, oa.x = h.x, oa.y = h.y - K, f[g ? "template" : "bg"].style.opacity = .001, Ga()), ab("initialZoom"), d && !c && e.removeClass(a, "pswp--animated-in"), g && (d ? e[(c ? "remove" : "add") + "Class"](a, "pswp--animate_opacity") : setTimeout(function() {
                      e.addClass(a, "pswp--animate_opacity")
                  }, 30)), Wb = setTimeout(function() {
                      if (Ca("initialZoom" + (d ? "Out" : "In")), d) {
                          var f = h.w / b.w,
                              i = {
                                  x: oa.x,
                                  y: oa.y
                              },
                              l = s,
                              m = ia,
                              n = function(b) {
                                  1 === b ? (s = f, oa.x = h.x, oa.y = h.y - M) : (s = (f - l) * b + l, oa.x = (h.x - i.x) * b + i.x, oa.y = (h.y - M - i.y) * b + i.y), Ga(), g ? a.style.opacity = 1 - b : Ea(m - b * m)
                              };
                          c ? cb("initialZoom", 0, 1, j, e.easing.cubic.out, n, k) : (n(1), Wb = setTimeout(k, j + 20))
                      } else s = b.initialZoomLevel, La(oa, b.initialPosition), Ga(), Ea(1), g ? a.style.opacity = 1 : Ea(1), Wb = setTimeout(k, j + 20)
                  }, d ? 25 : 90)
              };
              n()
          },
          cc = {},
          dc = [],
          ec = {
              index: 0,
              errorMsg: '<div class="pswp__error-msg"><a href="%url%" target="_blank">The image</a> could not be loaded.</div>',
              forceProgressiveLoading: !1,
              preload: [1, 1],
              getNumItemsFn: function() {
                  return Xb.length
              }
          },
          fc = function() {
              return {
                  center: {
                      x: 0,
                      y: 0
                  },
                  max: {
                      x: 0,
                      y: 0
                  },
                  min: {
                      x: 0,
                      y: 0
                  }
              }
          },
          gc = function(a, b, c) {
              var d = a.bounds;
              d.center.x = Math.round((cc.x - b) / 2), d.center.y = Math.round((cc.y - c) / 2) + a.vGap.top, d.max.x = b > cc.x ? Math.round(cc.x - b) : d.center.x, d.max.y = c > cc.y ? Math.round(cc.y - c) + a.vGap.top : d.center.y, d.min.x = b > cc.x ? 0 : d.center.x, d.min.y = c > cc.y ? a.vGap.top : d.center.y
          },
          hc = function(a, b, c) {
              if (a.src && !a.loadError) {
                  var d = !c;
                  if (d && (a.vGap || (a.vGap = {
                          top: 0,
                          bottom: 0
                      }), Ca("parseVerticalMargin", a)), cc.x = b.x, cc.y = b.y - a.vGap.top - a.vGap.bottom, d) {
                      var e = cc.x / a.w,
                          f = cc.y / a.h;
                      a.fitRatio = e < f ? e : f;
                      var g = i.scaleMode;
                      "orig" === g ? c = 1 : "fit" === g && (c = a.fitRatio), c > 1 && (c = 1), a.initialZoomLevel = c, a.bounds || (a.bounds = fc())
                  }
                  if (!c) return;
                  return gc(a, a.w * c, a.h * c), d && c === a.initialZoomLevel && (a.initialPosition = a.bounds.center), a.bounds
              }
              return a.w = a.h = 0, a.initialZoomLevel = a.fitRatio = 1, a.bounds = fc(), a.initialPosition = a.bounds.center, a.bounds
          },
          ic = function(a, b, c, d, e, g) {
              b.loadError || d && (b.imageAppended = !0, lc(b, d, b === f.currItem && xa), c.appendChild(d), g && setTimeout(function() {
                  b && b.loaded && b.placeholder && (b.placeholder.style.display = "none", b.placeholder = null)
              }, 500))
          },
          jc = function(a) {
              a.loading = !0, a.loaded = !1;
              var b = a.img = e.createEl("pswp__img", "img"),
                  c = function() {
                      a.loading = !1, a.loaded = !0, a.loadComplete ? a.loadComplete(a) : a.img = null, b.onload = b.onerror = null, b = null
                  };
              return b.onload = c, b.onerror = function() {
                  a.loadError = !0, c()
              }, b.src = a.src, b
          },
          kc = function(a, b) {
              if (a.src && a.loadError && a.container) return b && (a.container.innerHTML = ""), a.container.innerHTML = i.errorMsg.replace("%url%", a.src), !0
          },
          lc = function(a, b, c) {
              if (a.src) {
                  b || (b = a.container.lastChild);
                  var d = c ? a.w : Math.round(a.w * a.fitRatio),
                      e = c ? a.h : Math.round(a.h * a.fitRatio);
                  a.placeholder && !a.loaded && (a.placeholder.style.width = d + "px", a.placeholder.style.height = e + "px"), b.style.width = d + "px", b.style.height = e + "px"
              }
          },
          mc = function() {
              if (dc.length) {
                  for (var a, b = 0; b < dc.length; b++) a = dc[b], a.holder.index === a.index && ic(a.index, a.item, a.baseDiv, a.img, !1, a.clearPlaceholder);
                  dc = []
              }
          };
      ya("Controller", {
          publicMethods: {
              lazyLoadItem: function(a) {
                  a = za(a);
                  var b = $b(a);
                  b && (!b.loaded && !b.loading || x) && (Ca("gettingData", a, b), b.src && jc(b))
              },
              
              initController: function() {
                  e.extend(i, ec, !0), f.items = Xb = c, $b = f.getItemAt, _b = i.getNumItemsFn, ac = i.loop, _b() < 3 && (i.loop = !1), Ba("beforeChange", function(a) {
                      var b, c = i.preload,
                          d = null === a || a >= 0,
                          e = Math.min(c[0], _b()),
                          g = Math.min(c[1], _b());
                      for (b = 1; b <= (d ? g : e); b++) f.lazyLoadItem(m + b);
                      for (b = 1; b <= (d ? e : g); b++) f.lazyLoadItem(m - b)
                  }), Ba("initialLayout", function() {
                      f.currItem.initialLayout = i.getThumbBoundsFn && i.getThumbBoundsFn(m)
                  }), Ba("mainScrollAnimComplete", mc), Ba("initialZoomInEnd", mc), Ba("destroy", function() {
                      for (var a, b = 0; b < Xb.length; b++) a = Xb[b], a.container && (a.container = null), a.placeholder && (a.placeholder = null), a.img && (a.img = null), a.preloader && (a.preloader = null), a.loadError && (a.loaded = a.loadError = !1);
                      dc = null
                  })
              },
              getItemAt: function(a) {
                  return a >= 0 && (void 0 !== Xb[a] && Xb[a])
              },
              allowProgressiveImg: function() {
                  return i.forceProgressiveLoading || !G || i.mouseUsed || screen.width > 1200
              },
              setContent: function(a, b) {
                  i.loop && (b = za(b));
                  var c = f.getItemAt(a.index);
                  c && (c.container = null);
                  var d, g = f.getItemAt(b);
                  if (!g) return void(a.el.innerHTML = "");
                  Ca("gettingData", b, g), a.index = b, a.item = g;
                  var h = g.container = e.createEl("pswp__zoom-wrap");
                  if (!g.src && g.html && (g.html.tagName ? h.appendChild(g.html) : h.innerHTML = g.html), kc(g), hc(g, pa), !g.src || g.loadError || g.loaded) g.src && !g.loadError && (d = e.createEl("pswp__img", "img"), d.style.opacity = 1, d.src = g.src, lc(g, d), ic(b, g, h, d, !0));
                  else {
                      if (g.loadComplete = function(c) {
                              if (j) {
                                  if (a && a.index === b) {
                                      if (kc(c, !0)) return c.loadComplete = c.img = null, hc(c, pa), Ha(c), void(a.index === m && f.updateCurrZoomItem());
                                      c.imageAppended ? !Zb && c.placeholder && (c.placeholder.style.display = "none", c.placeholder = null) : N.transform && (ea || Zb) ? dc.push({
                                          item: c,
                                          baseDiv: h,
                                          img: c.img,
                                          index: b,
                                          holder: a,
                                          clearPlaceholder: !0
                                      }) : ic(b, c, h, c.img, ea || Zb, !0)
                                  }
                                  c.loadComplete = null, c.img = null, Ca("imageLoadComplete", b, c)
                              }
                          }, e.features.transform) {
                          var k = "pswp__img pswp__img--placeholder";
                          k += g.msrc ? "" : " pswp__img--placeholder--blank";
                          var l = e.createEl(k, g.msrc ? "img" : "");
                          g.msrc && (l.src = g.msrc), lc(g, l), h.appendChild(l), g.placeholder = l
                      }
                      g.loading || jc(g), f.allowProgressiveImg() && (!Yb && N.transform ? dc.push({
                          item: g,
                          baseDiv: h,
                          img: g.img,
                          index: b,
                          holder: a
                      }) : ic(b, g, h, g.img, !0, !0))
                  }
                  Yb || b !== m ? Ha(g) : (da = h.style, bc(g, d || g.img)), a.el.innerHTML = "", a.el.appendChild(h)
              },
              cleanSlide: function(a) {
                  a.img && (a.img.onload = a.img.onerror = null), a.loaded = a.loading = a.img = a.imageAppended = !1
              }
          }
      });
      var nc, oc = {},
          pc = function(a, b, c) {
              var d = document.createEvent("CustomEvent"),
                  e = {
                      origEvent: a,
                      target: a.target,
                      releasePoint: b,
                      pointerType: c || "touch"
                  };
              d.initCustomEvent("pswpTap", !0, !0, e), a.target.dispatchEvent(d)
          };
      ya("Tap", {
          publicMethods: {
              initTap: function() {
                  Ba("firstTouchStart", f.onTapStart), Ba("touchRelease", f.onTapRelease), Ba("destroy", function() {
                      oc = {}, nc = null
                  })
              },
              onTapStart: function(a) {
                  a.length > 1 && (clearTimeout(nc), nc = null)
              },
              onTapRelease: function(a, b) {
                  if (b && !X && !V && !$a) {
                      var c = b;
                      if (nc && (clearTimeout(nc), nc = null, wb(c, oc))) return void Ca("doubleTap", c);
                      if ("mouse" === b.type) return void pc(a, b, "mouse");
                      var d = a.target.tagName.toUpperCase();
                      if ("BUTTON" === d || e.hasClass(a.target, "pswp__single-tap")) return void pc(a, b);
                      La(oc, c), nc = setTimeout(function() {
                          pc(a, b), nc = null
                      }, 300)
                  }
              }
          }
      });
      var qc;
      ya("DesktopZoom", {
          publicMethods: {
              initDesktopZoom: function() {
                  L || (G ? Ba("mouseUsed", function() {
                      f.setupDesktopZoom()
                  }) : f.setupDesktopZoom(!0))
              },
              setupDesktopZoom: function(b) {
                  qc = {};
                  var c = "wheel mousewheel DOMMouseScroll";
                  Ba("bindEvents", function() {
                      e.bind(a, c, f.handleMouseWheel)
                  }), Ba("unbindEvents", function() {
                      qc && e.unbind(a, c, f.handleMouseWheel)
                  }), f.mouseZoomedIn = !1;
                  var d, g = function() {
                          f.mouseZoomedIn && (e.removeClass(a, "pswp--zoomed-in"), f.mouseZoomedIn = !1), s < 1 ? e.addClass(a, "pswp--zoom-allowed") : e.removeClass(a, "pswp--zoom-allowed"), h()
                      },
                      h = function() {
                          d && (e.removeClass(a, "pswp--dragging"), d = !1)
                      };
                  Ba("resize", g), Ba("afterChange", g), Ba("pointerDown", function() {
                      f.mouseZoomedIn && (d = !0, e.addClass(a, "pswp--dragging"))
                  }), Ba("pointerUp", h), b || g()
              },
              handleMouseWheel: function(a) {
                  if (s <= f.currItem.fitRatio) return i.modal && (!i.closeOnScroll || $a || U ? a.preventDefault() : E && Math.abs(a.deltaY) > 2 && (l = !0, f.close())), !0;
                  if (a.stopPropagation(), qc.x = 0, "deltaX" in a) 1 === a.deltaMode ? (qc.x = 18 * a.deltaX, qc.y = 18 * a.deltaY) : (qc.x = a.deltaX, qc.y = a.deltaY);
                  else if ("wheelDelta" in a) a.wheelDeltaX && (qc.x = -.16 * a.wheelDeltaX), a.wheelDeltaY ? qc.y = -.16 * a.wheelDeltaY : qc.y = -.16 * a.wheelDelta;
                  else {
                      if (!("detail" in a)) return;
                      qc.y = a.detail
                  }
                  Ra(s, !0);
                  var b = oa.x - qc.x,
                      c = oa.y - qc.y;
                  (i.modal || b <= ca.min.x && b >= ca.max.x && c <= ca.min.y && c >= ca.max.y) && a.preventDefault(), f.panTo(b, c)
              },
              toggleDesktopZoom: function(b) {
                  b = b || {
                      x: pa.x / 2 + ra.x,
                      y: pa.y / 2 + ra.y
                  };
                  var c = i.getDoubleTapZoom(!0, f.currItem),
                      d = s === c;
                  f.mouseZoomedIn = !d, f.zoomTo(d ? f.currItem.initialZoomLevel : c, b, 333), e[(d ? "remove" : "add") + "Class"](a, "pswp--zoomed-in")
              }
          }
      });
      var rc, sc, tc, uc, vc, wc, xc, yc, zc, Ac, Bc, Cc, Dc = {
              history: !0,
              galleryUID: 1
          },
          Ec = function() {
              return Bc.hash.substring(1)
          },
          Fc = function() {
              rc && clearTimeout(rc), tc && clearTimeout(tc)
          },
          Gc = function() {
              var a = Ec(),
                  b = {};
              if (a.length < 5) return b;
              var c, d = a.split("&");
              for (c = 0; c < d.length; c++)
                  if (d[c]) {
                      var e = d[c].split("=");
                      e.length < 2 || (b[e[0]] = e[1])
                  }
              if (i.galleryPIDs) {
                  var f = b.pid;
                  for (b.pid = 0, c = 0; c < Xb.length; c++)
                      if (Xb[c].pid === f) {
                          b.pid = c;
                          break
                      }
              } else b.pid = parseInt(b.pid, 10) - 1;
              return b.pid < 0 && (b.pid = 0), b
          },
          Hc = function() {
              if (tc && clearTimeout(tc), $a || U) return void(tc = setTimeout(Hc, 500));
              uc ? clearTimeout(sc) : uc = !0;
              var a = m + 1,
                  b = $b(m);
              b.hasOwnProperty("pid") && (a = b.pid);
              var c = xc + "&gid=" + i.galleryUID + "&pid=" + a;
              yc || Bc.hash.indexOf(c) === -1 && (Ac = !0);
              var d = Bc.href.split("#")[0] + "#" + c;
              Cc ? "#" + c !== window.location.hash && history[yc ? "replaceState" : "pushState"]("", document.title, d) : yc ? Bc.replace(d) : Bc.hash = c, yc = !0, sc = setTimeout(function() {
                  uc = !1
              }, 60)
          };
      ya("History", {
          publicMethods: {
              initHistory: function() {
                  if (e.extend(i, Dc, !0), i.history) {
                      Bc = window.location, Ac = !1, zc = !1, yc = !1, xc = Ec(), Cc = "pushState" in history, xc.indexOf("gid=") > -1 && (xc = xc.split("&gid=")[0], xc = xc.split("?gid=")[0]), Ba("afterChange", f.updateURL), Ba("unbindEvents", function() {
                          e.unbind(window, "hashchange", f.onHashChange)
                      });
                      var a = function() {
                          wc = !0, zc || (Ac ? history.back() : xc ? Bc.hash = xc : Cc ? history.pushState("", document.title, Bc.pathname + Bc.search) : Bc.hash = ""), Fc()
                      };
                      Ba("unbindEvents", function() {
                          l && a()
                      }), Ba("destroy", function() {
                          wc || a()
                      }), Ba("firstUpdate", function() {
                          m = Gc().pid
                      });
                      var b = xc.indexOf("pid=");
                      b > -1 && (xc = xc.substring(0, b), "&" === xc.slice(-1) && (xc = xc.slice(0, -1))), setTimeout(function() {
                          j && e.bind(window, "hashchange", f.onHashChange)
                      }, 40)
                  }
              },
              onHashChange: function() {
                  return Ec() === xc ? (zc = !0, void f.close()) : void(uc || (vc = !0, f.goTo(Gc().pid), vc = !1))
              },
              updateURL: function() {
                  Fc(), vc || (yc ? rc = setTimeout(Hc, 800) : Hc())
              }
          }
      }), e.extend(f, db)
  };
  return a
});

// ------ ** ------ 

! function(a, b) {
  "function" == typeof define && define.amd ? define(b) : "object" == typeof exports ? module.exports = b() : a.PhotoSwipeUI_Default = b()
}(this, function() {
  "use strict";
  var a = function(a, b) {
      var c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w = this,
          x = document.getElementById("vp__my_modal"),
          y = !1,
          z = !0,
          A = !0,
          B = !0,
          C = {
              barsSize: {
                  top: 44,
                  bottom: "auto"
              },
              closeElClasses: ["item", "caption", "zoom-wrap", "ui", "top-bar"],
              timeToIdle: 4e3,
              timeToIdleOutside: 1e3,
              loadingIndicatorDelay: 1e3,
              addCaptionHTMLFn: function(a, b) {
                  return a.title ? (b.children[0].innerHTML = a.title, !0) : (b.children[0].innerHTML = "", !1)
              },
              closeEl: !0,
              captionEl: !0,
              fullscreenEl: !0,
              zoomEl: !0,
              shareEl: !0,
              counterEl: !0,
              arrowEl: !0,
              preloaderEl: !0,
              tapToClose: !1,
              tapToToggleControls: !0,
              clickToCloseNonZoomable: !0,
              shareButtons: [{
                  id: "facebook",
                  label: "Share on Facebook",
                  url: "https://www.facebook.com/sharer/sharer.php?u={{url}}"
              }, {
                  id: "twitter",
                  label: "Tweet",
                  url: "https://twitter.com/intent/tweet?text={{text}}&url={{url}}"
              }, {
                  id: "pinterest",
                  label: "Pin it",
                  url: "http://www.pinterest.com/pin/create/button/?url={{url}}&media={{image_url}}&description={{text}}"
              }, {
                  id: "download",
                  label: "Download image",
                  url: "{{raw_image_url}}",
                  download: !0
              }],
              getImageURLForShare: function() {
                  return a.currItem.src || ""
              },
              getPageURLForShare: function() {
                  return window.location.href
              },
              getTextForShare: function() {
                  return a.currItem.title || ""
              },
              indexIndicatorSep: " / ",
              fitControlsWidth: 1200
          },
          D = function(a) {
              if (s) return !0;
              a = a || window.event, r.timeToIdle && r.mouseUsed && !k && T();
              for (var c, d, e = a.target || a.srcElement, f = e.getAttribute("class") || "", g = 0; g < _.length; g++) c = _[g], c.onTap && f.indexOf("pswp__" + c.name) > -1 && (c.onTap(), d = !0);
              if (d) {
                  a.stopPropagation && a.stopPropagation(), s = !0;
                  var h = b.features.isOldAndroid ? 600 : 30;
                  t = setTimeout(function() {
                      s = !1
                  }, h)
              }
          },
          E = function() {
              return !a.likelyTouchDevice || r.mouseUsed || screen.width > r.fitControlsWidth
          },
          F = function(a, c, d) {
              b[(d ? "add" : "remove") + "Class"](a, "pswp__" + c)
          },
          G = function() {
              var a = 1 === r.getNumItemsFn();
              a !== q && (F(d, "ui--one-slide", a), q = a)
          },
          H = function() {
              F(i, "share-modal--hidden", A)
          },
          I = function() {
              return console.log("toggle my modal hidden=" + B), B = "block" != x.style.display, B ? (console.log("show my modal v2"), setTimeout(function() {
                  console.log("showing 30 ms later"), x.style.display = "block"
              }, 30)) : (console.log("hide my modal"), setTimeout(function() {
                  console.log("hiding 30 ms later"), x.style.display = "none"
              }, 30)), !1
          },
          J = function() {
              return A = !A, A ? (b.removeClass(i, "pswp__share-modal--fade-in"), setTimeout(function() {
                  if (A && (console.log("Hide modal reload=" + m), H(), m)) {
                      var b = !1,
                          c = "";
                      console.log("Reloading..."), reloadPhotoswipe(g_pswp.cid, a.getCurrentIndex(), g_pswp.galleryElement, b, c)
                  }
              }, 300)) : (H(), setTimeout(function() {
                  A || (console.log("Show modal reload=" + m), b.addClass(i, "pswp__share-modal--fade-in"))
              }, 30)), !1
          },
          K = function(b) {
              b = b || window.event;
              var c = b.target || b.srcElement;
              return a.shout("shareLinkClick", b, c), console.log("_openWindowPopup ", c), c.href ? !!c.hasAttribute("download") || (console.log("ok, opening " + c.href), window.open(c.href, "pswp_share", "scrollbars=yes,resizable=yes,toolbar=no,location=yes,width=550,height=420,top=100,left=" + (window.screen ? Math.round(screen.width / 2 - 275) : 100)), A || J(), !1) : (console.log("no href.."), !1)
          },
          L = function() {
              var a = "/login/",
                  b = document.getElementById("vp__share-modal-content");
              b.innerHTML = vp_iframe(a)
          },
          M = function() {
              var a = document.getElementById("vp__share-modal-content");
              a.innerHTML = vp_container("Chat info comes here...", "vp__chat_container")
          },
          N = function() {
              var a = document.getElementById("vp__share-modal-content");
              a.innerHTML = vp_container(vp_infocalendar(g_pswp.cid), "vp__calendar_container"), addtocalendar.load()
          },
          O = function() {
              var a = document.getElementById("vp__my-modal-content");
              var b = g_pswp.getCurrentData(g_pswp.cid) || {};
              a.innerHTML = vp_infotabs(b);

              vp_update_info(b);
              var c = document.getElementById("vp__tab1");
              c.click(), console.log("clicking tab1")
          },
          P = function() {
              var a = document.getElementById("vp__share-modal-content");
              a.innerHTML = vp_container(vp_infomap(g_pswp.vpdata), "vp__map_container");
              var b = g_pswp.getCurrentData(g_pswp.cid) || {},
                  c = b.curator || {},
                  d = c.location_lat,
                  e = c.location_long,
                  f = Math.abs(d) > 0 && Math.abs(e) > 0;
              f && initModalMap("mapCanvas")
          },
          Q = function() {
              for (var a, b, c, d, e, f, g = "", h = 0; h < r.shareButtons.length; h++) a = r.shareButtons[h], c = r.getImageURLForShare(a), d = r.getPageURLForShare(a), e = r.getTextForShare(a), f = r.getObjIdForShare(a), b = a.url.replace("{{url}}", encodeURIComponent(d)).replace("{{image_url}}", encodeURIComponent(c)).replace("{{raw_image_url}}", c).replace("{{obj_id}}", f).replace("{{text}}", encodeURIComponent(e)), g += '<a href="' + b + '" target="_blank" class="pswp__share--' + a.id + '"' + (a.download ? "download" : "") + ">" + a.label + "</a>", r.parseShareButtonOut && (g = r.parseShareButtonOut(a, g));
              i.children[0].innerHTML = '<div class="pswp__share-tooltip">' + g + "</div>", i.children[0].onclick = K
          },
          R = function(a) {
              for (var c = 0; c < r.closeElClasses.length; c++)
                  if (b.hasClass(a, "pswp__" + r.closeElClasses[c])) return !0
          },
          S = 0,
          T = function() {
              clearTimeout(v), S = 0, k && w.setIdle(!1)
          },
          U = function(a) {
              a = a ? a : window.event;
              var b = a.relatedTarget || a.toElement;
              b && "HTML" !== b.nodeName || (clearTimeout(v), v = setTimeout(function() {
                  w.setIdle(!0)
              }, r.timeToIdleOutside))
          },
          V = function() {
              r.fullscreenEl && !b.features.isOldAndroid && (c || (c = w.getFullscreenAPI()), c ? (b.bind(document, c.eventK, w.updateFullscreen), w.updateFullscreen(), b.addClass(a.template, "pswp--supports-fs")) : b.removeClass(a.template, "pswp--supports-fs"))
          },
          W = function() {
              r.preloaderEl && (X(!0), l("beforeChange", function() {
                  clearTimeout(p), p = setTimeout(function() {
                      a.currItem && a.currItem.loading ? (!a.allowProgressiveImg() || a.currItem.img && !a.currItem.img.naturalWidth) && X(!1) : X(!0)
                  }, r.loadingIndicatorDelay)
              }), l("imageLoadComplete", function(b, c) {
                  a.currItem === c && X(!0)
              }))
          },
          X = function(a) {
              o !== a && (F(n, "preloader--active", !a), o = a)
          },
          Y = function(a) {
              var c = a.vGap;
              if (E()) {
                  var g = r.barsSize;
                  if (r.captionEl && "auto" === g.bottom)
                      if (f || (f = b.createEl("pswp__caption pswp__caption--fake"), f.appendChild(b.createEl("pswp__caption__center")), d.insertBefore(f, e), b.addClass(d, "pswp__ui--fit")), r.addCaptionHTMLFn(a, f, !0)) {
                          var h = f.clientHeight;
                          c.bottom = parseInt(h, 10) || 44
                      } else c.bottom = g.top;
                  else c.bottom = "auto" === g.bottom ? 0 : g.bottom;
                  c.top = g.top
              } else c.top = c.bottom = 0
          },
          Z = function() {
              r.timeToIdle && l("mouseUsed", function() {
                  b.bind(document, "mousemove", T), b.bind(document, "mouseout", U), u = setInterval(function() {
                      S++, 2 === S && w.setIdle(!0)
                  }, r.timeToIdle / 2)
              })
          },
          $ = function() {
              l("onVerticalDrag", function(a) {
                  z && a < .95 ? w.hideControls() : !z && a >= .95 && w.showControls()
              });
              var a;
              l("onPinchClose", function(b) {
                  z && b < .9 ? (w.hideControls(), a = !0) : a && !z && b > .9 && w.showControls()
              }), l("zoomGestureEnded", function() {
                  a = !1, a && !z && w.showControls()
              })
          },
          _ = [{
              name: "caption",
              option: "captionEl",
              onInit: function(a) {
                  e = a
              }
          }, {
              name: "share-modal",
              option: "shareEl",
              onInit: function(a) {
                  i = a
              },
              onTap: function() {
                  J()
              }
          }, {
              name: "button--share",
              option: "shareEl",
              onInit: function(a) {
                  h = a
              },
              onTap: function() {
                  m = 0, J(), Q()
              }
          }, {
              name: "button--zoom",
              option: "zoomEl",
              onTap: a.toggleDesktopZoom
          }, {
              name: "counter",
              option: "counterEl",
              onInit: function(a) {
                  g = a
              }
          }, {
              name: "button--close",
              option: "closeEl",
              onTap: a.close
          }, {
              name: "button--arrow--left",
              option: "arrowEl",
              onTap: a.prev
          }, {
              name: "button--arrow--right",
              option: "arrowEl",
              onTap: a.next
          }, {
              name: "button--fs",
              option: "fullscreenEl",
              onTap: function() {
                  var a = document.getElementById("vp__infoBtn");
                  c.isFullscreen() ? (c.exit(), a.style.display = "block") : (a.style.display = "none", c.enter())
              }
          }, {
              name: "preloader",
              option: "preloaderEl",
              onInit: function(a) {
                  n = a
              }
          }, {
              name: "button--like",
              option: "likeEl",
              onTap: function() {
                  var a = g_pswp.getCurrentData(g_pswp.cid),
                      b = a.item;
                  if (m = 0, g_pswp.reload = 1, g_user_id) console.log("click on like by user " + g_user_id), g_pswp.like(g_pswp.cid, a);
                  else {
                      m = 1;
                      var c = vp_follow_link(a.curator_id, b);
                      if (console.log("click on like - no user curator:" + a.curator_id + " mid=" + b.media_id + " link=" + c), !c) return;
                      var d = document.getElementById("vp__share-modal-content");
                      d.innerHTML = vp_iframe(c), J()
                  }
              }
          }, {
              name: "button--info",
              option: "infoEl",
              onInit: function(a) {
                  h = a
              },
              onTap: function() {
                  m = 0, console.log("infoEl tap"), I(), O()
              }
          }, {
              name: "button--calendar",
              option: "calendarEl",
              onInit: function(a) {
                  h = a
              },
              onTap: function() {
                  m = 0, J(), N()
              }
          }, {
              name: "button--user",
              option: "userEl",
              onInit: function(a) {
                  h = a
              },
              onTap: function() {
                  m = 1, J(), L()
              }
          }, {
              name: "button--contact",
              option: "contactEl",
              onInit: function(a) {
                  h = a
              },
              onTap: function() {
                  m = 1;
                  var a = vp_contact_link(g_pswp.cid);
                  console.log("opening popup link=" + a + " el:", h), vp_popup(a)
              }
          }, {
              name: "button--map",
              option: "mapEl",
              onInit: function(a) {
                  h = a
              },
              onTap: function() {
                  m = 0, J(), P()
              }
          }, {
              name: "button--chat",
              option: "chatEl",
              onInit: function(a) {
                  h = a
              },
              onTap: function() {
                  m = 0, J(), M()
              }
          }],
          aa = function() {
              var a, c, e, f = function(d) {
                  if (d)
                      for (var f = d.length, g = 0; g < f; g++) {
                          a = d[g], c = a.className;
                          for (var h = 0; h < _.length; h++) e = _[h], c.indexOf("pswp__" + e.name) > -1 && (r[e.option] ? (b.removeClass(a, "pswp__element--disabled"), e.onInit && e.onInit(a)) : b.addClass(a, "pswp__element--disabled"))
                      }
              };
              f(d.children);
              var g = b.getChildByClass(d, "pswp__top-bar");
              g && f(g.children)
          };
      w.init = function() {
          b.extend(a.options, C, !0), r = a.options, d = b.getChildByClass(a.scrollWrap, "pswp__ui"), l = a.listen, $(), l("beforeChange", w.update), l("doubleTap", function(b) {
              var c = a.currItem.initialZoomLevel;
              a.getZoomLevel() !== c ? a.zoomTo(c, b, 333) : a.zoomTo(r.getDoubleTapZoom(!1, a.currItem), b, 333)
          }), l("preventDragEvent", function(a, b, c) {
              var d = a.target || a.srcElement;
              d && d.getAttribute("class") && a.type.indexOf("mouse") > -1 && (d.getAttribute("class").indexOf("__caption") > 0 || /(SMALL|STRONG|EM)/i.test(d.tagName)) && (c.prevent = !1)
          }), l("bindEvents", function() {
              b.bind(d, "pswpTap click", D), b.bind(a.scrollWrap, "pswpTap", w.onGlobalTap), a.likelyTouchDevice || b.bind(a.scrollWrap, "mouseover", w.onMouseOver)
          }), l("unbindEvents", function() {
              A || J(), u && clearInterval(u), b.unbind(document, "mouseout", U), b.unbind(document, "mousemove", T), b.unbind(d, "pswpTap click", D), b.unbind(a.scrollWrap, "pswpTap", w.onGlobalTap), b.unbind(a.scrollWrap, "mouseover", w.onMouseOver), c && (b.unbind(document, c.eventK, w.updateFullscreen), c.isFullscreen() && (r.hideAnimationDuration = 0, c.exit()), c = null)
          }), l("destroy", function() {
              r.captionEl && (f && d.removeChild(f), b.removeClass(e, "pswp__caption--empty")), i && (i.children[0].onclick = null), b.removeClass(d, "pswp__ui--over-close"), b.addClass(d, "pswp__ui--hidden"), w.setIdle(!1)
          }), r.showAnimationDuration || b.removeClass(d, "pswp__ui--hidden"), l("initialZoomIn", function() {
              r.showAnimationDuration && b.removeClass(d, "pswp__ui--hidden")
          }), l("initialZoomOut", function() {
              b.addClass(d, "pswp__ui--hidden")
          }), l("parseVerticalMargin", Y), aa(), r.shareEl && h && i && (A = !0), G(), Z(), V(), W()
      }, w.setIdle = function(a) {
          k = a, F(d, "ui--idle", a)
      }, w.update = function() {
          z && a.currItem ? (w.updateIndexIndicator(), r.captionEl && (r.addCaptionHTMLFn(a.currItem, e), F(e, "caption--empty", !a.currItem.title)), y = !0) : y = !1, A || J(), G()
      }, w.updateFullscreen = function(d) {
          d && setTimeout(function() {
              a.setScrollOffset(0, b.getScrollY())
          }, 50), b[(c.isFullscreen() ? "add" : "remove") + "Class"](a.template, "pswp--fs")
      }, w.updateIndexIndicator = function() {
          r.counterEl && (g.innerHTML = a.getCurrentIndex() + 1 + r.indexIndicatorSep + r.getNumItemsFn())
      }, w.onGlobalTap = function(c) {
          c = c || window.event;
          var d = c.target || c.srcElement;
          if (console.log("tap target=", d), b.hasClass(d, "pswp__top-bar")) return void console.log("tap on top bar");
          if (!s)
              if (c.detail && "mouse" === c.detail.pointerType) {
                  if (R(d)) return void a.close();
                  b.hasClass(d, "pswp__img") && (1 === a.getZoomLevel() && a.getZoomLevel() <= a.currItem.fitRatio ? r.clickToCloseNonZoomable && a.close() : a.toggleDesktopZoom(c.detail.releasePoint))
              } else if (r.tapToToggleControls && (z ? w.hideControls() : w.showControls()), r.tapToClose && (b.hasClass(d, "pswp__img") || R(d))) return void a.close()
      }, w.onMouseOver = function(a) {
          a = a || window.event;
          var b = a.target || a.srcElement;
          F(d, "ui--over-close", R(b))
      }, w.hideControls = function() {
          b.addClass(d, "pswp__ui--hidden"), z = !1
      }, w.showControls = function() {
          z = !0, y || w.update(), b.removeClass(d, "pswp__ui--hidden")
      }, w.supportsFullscreen = function() {
          var a = document;
          return !!(a.exitFullscreen || a.mozCancelFullScreen || a.webkitExitFullscreen || a.msExitFullscreen)
      }, w.getFullscreenAPI = function() {
          var b, c = document.documentElement,
              d = "fullscreenchange";
          return c.requestFullscreen ? b = {
              enterK: "requestFullscreen",
              exitK: "exitFullscreen",
              elementK: "fullscreenElement",
              eventK: d
          } : c.mozRequestFullScreen ? b = {
              enterK: "mozRequestFullScreen",
              exitK: "mozCancelFullScreen",
              elementK: "mozFullScreenElement",
              eventK: "moz" + d
          } : c.webkitRequestFullscreen ? b = {
              enterK: "webkitRequestFullscreen",
              exitK: "webkitExitFullscreen",
              elementK: "webkitFullscreenElement",
              eventK: "webkit" + d
          } : c.msRequestFullscreen && (b = {
              enterK: "msRequestFullscreen",
              exitK: "msExitFullscreen",
              elementK: "msFullscreenElement",
              eventK: "MSFullscreenChange"
          }), b && (b.enter = function() {
              return j = r.closeOnScroll, r.closeOnScroll = !1, "webkitRequestFullscreen" !== this.enterK ? a.template[this.enterK]() : void a.template[this.enterK](Element.ALLOW_KEYBOARD_INPUT)
          }, b.exit = function() {
              return r.closeOnScroll = j, document[this.exitK]()
          }, b.isFullscreen = function() {
              return document[this.elementK]
          }), b
      }
  };
  return a
});

// ------ ** ------ 

/* JSONP Pure js. Thanks: http://stackoverflow.com/questions/6132796/how-to-make-a-jsonp-request-from-javascript-without-jquery */
var $jsonp = (function() {
  var that = {};

  that.send = function(src, options) {
      var callback_name = options.callbackName || 'callback',
          on_success = options.onSuccess || function() {},
          on_timeout = options.onTimeout || function() {},
          timeout = options.timeout || 10; // sec

      var timeout_trigger = window.setTimeout(function() {
          window[callback_name] = function() {};
          on_timeout();
      }, timeout * 1000);

      window[callback_name] = function(data) {
          window.clearTimeout(timeout_trigger);
          on_success(data);
      }

      var script = document.createElement('script');
      script.type = 'text/javascript';
      script.async = true;
      script.src = src;

      document.getElementsByTagName('head')[0].appendChild(script);
  }

  return that;
})();

function inArray(needle, haystack) {
  var length = haystack.length;
  for (var i = 0; i < length; i++) {
      if (haystack[i] == needle) return true;
  }
  return false;
}

// find nearest parent element
var closest = function closest(el, fn) {
  return el && (fn(el) ? el : closest(el.parentNode, fn));
};

// Thanks http://stackoverflow.com/questions/5223/length-of-a-javascript-object-that-is-associative-array
Object.size = function(obj) {
  var size = 0,
      key;
  for (key in obj) {
      if (obj.hasOwnProperty(key)) size++;
  }
  return size;
};

// Thanks: http://stackoverflow.com/questions/9838812/how-can-i-open-a-json-file-in-javascript-without-jquery
function loadJSON(path, success, error) {
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
      if (xhr.readyState === XMLHttpRequest.DONE) {
          if (xhr.status === 200) {
              if (success)
                  success(JSON.parse(xhr.responseText));
          } else {
              if (error)
                  error(xhr);
          }
      }
  };
  xhr.open("GET", path, true);
  xhr.send();
}

function currency2symbol(currency) {
  var currency_symbols = {
      'USD': '$', // US Dollar
      'EUR': '€', // Euro
      'CRC': '₡', // Costa Rican Colón
      'GBP': '£', // British Pound Sterling
      'ILS': '₪', // Israeli New Sheqel
      'INR': '₹', // Indian Rupee
      'JPY': '¥', // Japanese Yen
      'KRW': '₩', // South Korean Won
      'NGN': '₦', // Nigerian Naira
      'PHP': '₱', // Philippine Peso
      'PLN': 'zł', // Polish Zloty
      'PYG': '₲', // Paraguayan Guarani
      'THB': '฿', // Thai Baht
      'UAH': '₴', // Ukrainian Hryvnia
      'VND': '₫', // Vietnamese Dong
      'BTC': '฿', // Vietnamese Dong
  };
  var symbol = currency_symbols[currency];
  if (!symbol) return currency;
  return symbol;
}

function numberWithCommas(x) {
  return x.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ",");
}

// https://stackoverflow.com/questions/4671031/print-function-log-stack-trace-for-entire-program-using-firebug
function logStackTrace(levels) {
  var callstack = [];
  var isCallstackPopulated = false;
  try {
      i.dont.exist += 0; //doesn't exist- that's the point
  } catch (e) {
      if (e.stack) { //Firefox / chrome
          var lines = e.stack.split('\n');
          for (var i = 0, len = lines.length; i < len; i++) {
              callstack.push(lines[i]);
          }
          //Remove call to logStackTrace()
          callstack.shift();
          isCallstackPopulated = true;
      } else if (window.opera && e.message) { //Opera
          var lines = e.message.split('\n');
          for (var i = 0, len = lines.length; i < len; i++) {
              if (lines[i].match(/^\s*[A-Za-z0-9\-_\$]+\(/)) {
                  var entry = lines[i];
                  //Append next line also since it has the file info
                  if (lines[i + 1]) {
                      entry += " at " + lines[i + 1];
                      i++;
                  }
                  callstack.push(entry);
              }
          }
          //Remove call to logStackTrace()
          callstack.shift();
          isCallstackPopulated = true;
      }
  }
  if (!isCallstackPopulated) { //IE and Safari
      var currentFunction = arguments.callee.caller;
      while (currentFunction) {
          var fn = currentFunction.toString();
          var fname = fn.substring(fn.indexOf("function") + 8, fn.indexOf("(")) || "anonymous";
          callstack.push(fname);
          currentFunction = currentFunction.caller;
      }
  }
  if (levels) {
      console.log(callstack.slice(0, levels).join('\n'));
  } else {
      console.log(callstack.join('\n'));
  }
};

// ------ ** ------ 

/*! vPatina JS: 0.1.3 2017-03-19
* https://www.vpatina.com
* Copyright (c) 2016 vPatina BV; */
var getUrl = window.location;
var baseUrl = getUrl.protocol + "//" + getUrl.host;
var g_pswp = false;
var g_pswp_index = -1;
var g_site_url = baseUrl;
var g_host_url = baseUrl;
var g_aid = 10;
var g_site_id = '247';
var g_user_id = '';
var g_user_name = '';
var g_sharer_id = '';
var g_host_protocol = 'https';
var g_host_tld = 'www';
var g_host_domain = getUrl.host;
var g_client_protocol = document.location.protocol;
var g_follow = [];
var g_branch = '9d08da5';

var g_list;
var g_cid = '';
var g_follow = [];
var g_artists = {};
var g_json_data;
var g_local_file = '';
var g_local_data = '';
var g_plugin_options = '';
var g_popup_options = '';
var g_popup_buttons = '';
var g_popup_tabs = '';
var g_cids = {};
var g_json_data = {};
var g_list = {};
var g_keys = {};

// add-ons: move to separate file
// ATC Calendar
(function(w, d) {
  var atc_url = "//addtocalendar.com/atc/",
      atc_version = "1.5",
      b = d.documentElement;
  if (!Array.indexOf) {
      Array.prototype.indexOf = function(e) {
          for (var t = 0, n = this.length; t < n; t++) {
              if (this[t] == e) {
                  return t
              }
          }
          return -1
      }
  }
  if (!Array.prototype.map) {
      Array.prototype.map = function(e) {
          var t = [];
          for (var n = 0, r = this.length; n < r; n++) {
              t.push(e(this[n]))
          }
          return t
      }
  }
  var isArray = function(e) {
      return Object.prototype.toString.call(e) === "[object Array]"
  };
  var isFunc = function(e) {
      return Object.prototype.toString.call(e) === "[object Function]"
  };
  var ready = function(e, t) {
      function u() {
          if (!n) {
              if (!t.body) return setTimeout(u, 13);
              n = true;
              if (i) {
                  var e, r = 0;
                  while (e = i[r++]) e.call(null);
                  i = null
              }
          }
      }

      function a() {
          if (r) return;
          r = true;
          if (t.readyState === "complete") return u();
          if (t.addEventListener) {
              t.addEventListener("DOMContentLoaded", s, false);
              e.addEventListener("load", u, false)
          } else {
              if (t.attachEvent) {
                  t.attachEvent("onreadystatechange", s);
                  e.attachEvent("onload", u);
                  var n = false;
                  try {
                      n = e.frameElement == null
                  } catch (i) {}
                  if (b.doScroll && n) f()
              } else {
                  o = e.onload;
                  e.onload = function(e) {
                      o(e);
                      u()
                  }
              }
          }
      }

      function f() {
          if (n) return;
          try {
              b.doScroll("left")
          } catch (e) {
              setTimeout(f, 1);
              return
          }
          u()
      }
      var n = false,
          r = false,
          i = [],
          s, o;
      if (t.addEventListener) {
          s = function() {
              t.removeEventListener("DOMContentLoaded", s, false);
              u()
          }
      } else {
          if (t.attachEvent) {
              s = function() {
                  if (t.readyState === "complete") {
                      t.detachEvent("onreadystatechange", s);
                      u()
                  }
              }
          }
      }
      return function(e) {
          a();
          if (n) {
              e.call(null)
          } else {
              i.push(e)
          }
      }
  }(w, d);
  if (w.addtocalendar && typeof w.addtocalendar.start == "function") return;
  if (!w.addtocalendar) w.addtocalendar = {};
  addtocalendar.languages = {
      de: "In den Kalender",
      en: "Add to Calendar",
      es: "Añadir al Calendario",
      fr: "Ajouter au calendrier",
      hi: "कैलेंडर में जोड़ें",
      "in": "Tambahkan ke Kalender",
      ja: "カレンダーに追加",
      ko: "캘린더에 추가",
      pt: "Adicionar ao calendário",
      ru: "Добавить в календарь",
      uk: "Додати в календар",
      zh: "添加到日历"
  };
  addtocalendar.calendar_urls = {};
  addtocalendar.loadSettings = function(element) {
      var settings = {
          language: "auto",
          "show-list-on": "click",
          calendars: ["iCalendar", "Google Calendar", "Outlook", "Outlook Online", "Yahoo! Calendar"],
          secure: "auto",
          "on-button-click": function() {},
          "on-calendar-click": function() {}
      };
      for (var option in settings) {
          var pname = "data-" + option;
          var eattr = element.getAttribute(pname);
          if (eattr != null) {
              if (isArray(settings[option])) {
                  settings[option] = eattr.replace(/\s*,\s*/g, ",").replace(/^\s+|\s+$/g, "").split(",");
                  continue
              }
              if (isFunc(settings[option])) {
                  var fn = window[eattr];
                  if (isFunc(fn)) {
                      settings[option] = fn
                  } else {
                      settings[option] = eval("(function(mouseEvent){" + eattr + "})")
                  }
                  continue
              }
              settings[option] = element.getAttribute(pname)
          }
      }
      return settings
  };
  addtocalendar.load = function() {
      ready(function() {
          var e = {
              iCalendar: "ical",
              "Google Calendar": "google",
              Outlook: "outlook",
              "Outlook Online": "outlookonline",
              "Yahoo! Calendar": "yahoo"
          };
          var t = -(new Date).getTimezoneOffset().toString();
          var n = addtocalendar.languages;
          var r = document.getElementsByTagName("*");
          for (var i = 0; i < r.length; i++) {
              var s = r[i].className;
              if (s.length && s.split(" ").indexOf("addtocalendar") != -1) {
                  var o = addtocalendar.loadSettings(r[i]);
                  var u = o["calendars"].length == 1;
                  var a = "http:";
                  if (o["secure"] == "auto") {
                      a = location.protocol == "https:" ? "https:" : "http:"
                  } else if (o["secure"] == "true") {
                      a = "https:"
                  }
                  var f = a + atc_url;
                  var l = r[i].id;
                  var c = n["en"];
                  if (o["language"] == "auto") {
                      var h = "no_lang";
                      if (typeof navigator.language === "string") {
                          h = navigator.language.substr(0, 2)
                      } else if (typeof navigator.browserLanguage === "string") {
                          h = navigator.browserLanguage.substr(0, 2)
                      }
                      if (n.hasOwnProperty(h)) {
                          c = n[h]
                      }
                  } else if (n.hasOwnProperty(o["language"])) {
                      c = n[o["language"]]
                  }
                  var p = ["utz=" + t, "uln=" + navigator.language, "vjs=" + atc_version];
                  var d = r[i].getElementsByTagName("var");
                  var v = -1;
                  for (var m = 0; m < d.length; m++) {
                      var g = d[m].className.replace("atc_", "");
                      var y = d[m].innerHTML;
                      if (g == "event") {
                          v++;
                          continue
                      }
                      if (g == d[m].className) {
                          if (g == "atc-body") {
                              c = y
                          }
                          continue
                      }
                      if (v == -1) {
                          continue
                      }
                      p.push("e[" + v + "][" + g + "]" + "=" + encodeURIComponent(y))
                  }
                  var b = l == "" ? "" : l + "_link";
                  var w = document.createElement("ul");
                  w.className = "atcb-list";
                  var E = "";
                  var S = "";
                  for (var x in o["calendars"]) {
                      if (!e.hasOwnProperty(o["calendars"][x])) {
                          continue
                      }
                      var T = e[o["calendars"][x]];
                      var N = l == "" ? "" : 'id="' + l + "_" + T + '_link"';
                      var C = f + T + "?" + p.join("&");
                      if (u) {
                          S = C
                      } else {
                          E += '<li class="atcb-item"><a ' + N + ' class="atcb-item-link" href="' + C + '" target="_blank">' + o["calendars"][x] + "</a></li>"
                      }
                  }
                  w.innerHTML = E;
                  if (r[i].getElementsByClassName("atcb-link")[0] == undefined) {
                      var k = document.createElement("a");
                      k.className = "atcb-link";
                      k.innerHTML = c;
                      k.id = b;
                      k.tabIndex = 1;
                      if (u) {
                          k.href = S;
                          k.target = "_blank"
                      }
                      r[i].appendChild(k);
                      if (!u) {
                          r[i].appendChild(w)
                      }
                  } else {
                      var k = r[i].getElementsByClassName("atcb-link")[0];
                      if (!u) {
                          k.parentNode.appendChild(w)
                      }
                      k.tabIndex = 1;
                      if (k.id == "") {
                          k.id = b
                      }
                      if (u) {
                          k.href = S;
                          k.target = "_blank"
                      }
                  }
                  r[i].getElementsByClassName("atcb-link")[0].addEventListener("click", o["on-button-click"], false);
                  var L = r[i].getElementsByClassName("atcb-item-link");
                  for (var m = 0; m < L.length; m++) {
                      L[m].addEventListener("click", o["on-calendar-click"], false)
                  }
              }
          }
      })
  };
  addtocalendar.load()
})(window, document)

var g_local = false;
var g_start = new Date().getTime();
var g_timer = {};

/** Tab content */
function vp_update_info(vpdata) {
  // the content
  var item = vpdata.item;

  var artist_id = item.artist_id;
  var artist = vpdata.artists && artist_id ? vpdata.artists[artist_id] : {};
  var curator = vpdata.curator || {};
  var collection = vpdata.collection;
  var content = '';

  if (document.getElementById("vp__tab1")) {
      //document.getElementById("vp__tab1").click(); // open first tab 

      if (g_popup_buttons && g_popup_tabs) {

          //console.log("show/hide tabs according to popup tabs:", g_popup_tabs);
          //console.log("vp_update_info: buttons",g_popup_buttons);
          //console.log("vp_update_info: tabs",g_popup_tabs);
          if (!g_plugin_options.show_info_button) {
              //console.log("vp_update_info: no info, should return - showing anyways");
              //return;
          }

          document.getElementById('vp__tab1-content').innerHTML = vp_artwork_info(vpdata);
          document.getElementById('vp__tab2-content').innerHTML = content = vp_tab_content(artist, 'vp__tab2-inner-content');
          document.getElementById('vp__tab3-content').innerHTML = content = vp_tab_content(collection, 'vp__tab3-inner-content');
          document.getElementById('vp__tab4-content').innerHTML = content = vp_tab_content(curator, 'vp__tab4-inner-content');
          document.getElementById('vp__tab5-content').innerHTML = content = vp_artwork_description(vpdata, 'vp__tab5-inner-content');

          var tab1 = document.getElementById('vp__tab1');
          var tab2 = document.getElementById('vp__tab2');
          var tab3 = document.getElementById('vp__tab3');
          var tab4 = document.getElementById('vp__tab4');

          tab1.style.display = inArray('artwork', g_popup_tabs) ? "block" : "none";
          tab2.style.display = inArray('artist', g_popup_tabs) ? "block" : "none";
          tab3.style.display = inArray('exhibition', g_popup_tabs) ? "block" : "none";
          tab4.style.display = inArray('gallery', g_popup_tabs) ? "block" : "none";

          return;
      }
  }
}

function vp_tab_content(object, div_id) {
  console.log("object-------------");

  console.log(object);
  var title = object.display_name || object.title || '';
  var desc = object.description;
  var photo = object.image ? "<br><img src='" + object.image + "' width='150px'><br>" : "";
  var content = '';
  if (title) content = content + "<h4>" + title + "</h4>";
  if (photo) content = content + photo;
  if (desc) content = content + "<div class='vp__tab-inner-content' id='" + div_id + "'><p>" + desc + "</p></div>";
  content = content + "<p>" + object.bio_description + "</p>";
  
  return content;
}

function vp_artwork_info(vpdata) {
  var item = vpdata.item;
  var content = '';

  var artist_id = item.artist_id;
  var artist = vpdata.artists && artist_id ? vpdata.artists[artist_id] : null;

  var caption = vp_caption(vpdata, item, 1);

  var content = "<h3 class='popup_main_title'>" + caption + "</h3><br>";

  /**                       
  if(artist && artist.display_name) {
    var artist_str = artist.display_name;
    var loc_str = '';
    if(artist.city || artist.country) {
      var ar = [artist.city, artist.country];
      loc_str = ar.join(', ');
      artist_str = artist_str + "<br><small>" + loc_str + "</small>";
    }
    content = content + "<h4>by " + artist_str + "</h4>";
  }
  */

  if (item.original_technique) content = content + "<h5 class='popup_sub_title'><i>" + item.original_technique + "</i></h5>";

  var size = vp_size(item);
  if (size) content = content + "<h5 class='popup_sub_title'><i>" + size + "</i></h5><br>";

  

  
  //if(item.original_price && item.original_currency && item.original_price > 0) {
  //  content = content + "<i>" + currency2symbol(item.original_currency) +  numberWithCommas(Math.round(item.original_price)) + "</i><br>";
  //} else {
  //  content = content + "Price on inquiry<br>";
  //}
  if (item.original_count > 0) {
      if (item.original_count == 1) {
          // content = content + "<hr><p>Original of one</p>";
          content = content + "<hr><p class='popup_desc'>Original of one</p>";
      } else {
          // content = content + "<hr><p>Series of " + item.original_count + "</p>";
          content = content + "<hr><p class='popup_desc'>Series of " + item.original_count + "</p>";
      }
  }else{
      content = content + "<hr><p class='popup_desc'>Unique" + "</p>";

  }
  var price = vp_price(item);

  if (price) content = content + "<p class='popup_desc'>" + price + "</p>";

  // if (item.comment) content = content + "<hr><p>" + item.comment + "</p><br>";

  //<div class='vp__tab-content'><p>" + comment + "</p></div>";
  return content;
}

function vp_artwork_description(vpdata) {
  var item = vpdata.item;
  var content = '';


  var artist_id = item.artist_id;
  var artist = vpdata.artists && artist_id ? vpdata.artists[artist_id] : null;

  var caption = vp_caption(vpdata, item, 1);

  // var content = "<h4>Description</h4><br>";

  content = content + "<i>" + item.description + "</i><br><br>";

  return content;
}

// context: 
// 0 (default) = all 
// 1: tab 
// 2: mail

function vp_artist_name(vpdata, artist_id) {
  var artists = vpdata.artists;
  var artist = artists && artist_id ? artists[artist_id] : null;
  var artist_name = artist ? artist.display_name : "Unknown arist";
  return artist_name;
}

function vp_caption(vpdata, item, context) {
  if (typeof context == "undefined") var context = 0;

  var artist_id = item.artist_id;
  var artists = vpdata.artists;
  var artist = artists && artist_id ? artists[artist_id] : null;
  var artist_name = artist ? artist.display_name : "Unknown arist";
  //var artists = g_artists || {};
  //var artist_id = item.artist_id || 0;

  var gallery_name = vpdata.curator.display_name || '';

  var available = parseInt(item.original_available);
  var color = available > 0 ? '#0f0' : '#f00';
  var dot = available ? '<span style="color:' + color + '">&#x25cf;</span> ' : '';

  //console.log("vp_caption: vpdata:",vpdata);
  //console.log("vp_caption: artist:",artist);
  //console.log("vp_caption: artist_id:" + artist_id + " name=" + artist_name + " gartist:" + Object.size(g_artists));

  if (context > 0) {
      if (context == 2) dot = ''
      var year = item.original_year ? " (" + item.original_year + ")" : "";
      return dot + item.title; // only used for sharing

      // return dot + item.title + year + " by " + artist_name; // only used for sharing
  }

  // (AVAILABILITY DOT) GALLERY/ CURATOR NAME presents ARTWORK NAME(YEAR) TECHNQUE (COMMA)by ARTIST NAME (COMMA) SIZE (COMMA) SERIES OF X, (COMMA)PRICE
  // line 1: dot, gallery presents title technique

  //list of comma separated elements
  var elAr = []

  // element 1
  var element1 = '';

  // dot  
  if (inArray('dot', g_popup_title)) {
      element1 = element1 + dot + ' ';
  }

  // gallery
  if (inArray('gallery', g_popup_title) && gallery_name) {
      element1 = element1 + gallery_name + ' presents:';
  }

  // title
  if (inArray('artwork', g_popup_title) && item.title) {
      element1 = element1 + ' ' + item.title;
  }

  // year
  if (inArray('year', g_popup_title) && item.original_year) {
      //element1 = element1 + ' (' +  item.original_year + ')';
  }

  // technique
  if (inArray('technique', g_popup_title) && item.original_technique) {
      //element1 = element1 + ' ' + item.original_technique;
  }

  if (element1) elAr.push(element1);

  // artist name
  if (inArray('artist', g_popup_title) && artist_name) {
      //elAr.push('by ' + artist_name); hidden for tym being
  }

  //console.log("vp_caption item:", item);

  var series = '';
  if (item.original_count > 0) {
      if (item.original_count == 1) {
          series = "Original of one";
      } else {
          series = "Series of " + item.original_count;
      }
  }

  var size = vp_size(item);
  var price = vp_price(item);

  var show_size = size && inArray('size', g_popup_title);
  var show_series = series && inArray('series', g_popup_title);
  var show_price = price && inArray('price', g_popup_title);

  //console.log("vp_caption " + item.title + " (" + item.media_id + "): available:" + available + " org_price:" + item.original_price + " price:" + price + " show_price:" + show_price + " g_popup_title:", g_popup_title);

  if (show_size || show_price || show_series) {
      if (show_size) elAr.push(size);
      if (show_series) elAr.push(series);
      if (show_price) elAr.push(price);
  }

  return elAr.join(', ');
}

function vp_size(item) {
  var h = item.original_height;
  var w = item.original_width;
  var size = h > 0 && w > 0 ? h + "x" + w + " cm" : '';
  return size;
}

function vp_price(item) {
  var available = parseInt(item.original_available);
  var price = '';
  if (item.original_price > 0 && item.original_currency) { // price is set
      price = currency2symbol(item.original_currency) + ' ' + numberWithCommas(Math.round(item.original_price));
  } else { // no price - only show if available
      price = available > 0 ? 'Price on inquiry' : '';
  }
  //price = "<i>" + price + "</i>";
  return price;
}

/** Load gallery data for PhotoSwipe from vPatina server */
function vp_load(element, data, reload) {
  var cid = element.getAttribute('id');
  var plugin_id = data.plugin_id;

  var ajax_url = g_site_url + '/ajax.php?oper=get-gallery&jsoncallback=handleStuff';
  var data_str = "&obj_type=" + data.obj_type + "&obj_id=" + data.obj_id + "&plugin_id=" + data.plugin_id + "&target=" + cid + "&artwork=" + window.location.href;
  //var key = data.obj_type + "-id_" + data.obj_id + '-share_' + data.plugin_id;
  var key = data.obj_type + '-' + data.obj_id;
  console.log("vpload on el: " + cid + " plugin_id=" + data.plugin_id + " data=", data);

  if (g_local_data) {
      g_local = 1;
      //console.log("We have local (test) data: size=" + Object.size(g_local_data));
      console.log("Local Data:", g_local_data);
      vp_process_json(g_local_data, cid, reload, data);
  } else if (g_local_file) {
      g_local = 1;
      //console.log("Reading from local file:" + g_local_file);
      loadJSON(g_local_file,
          function(json) {
              //console.log("OK: read the following locally"); 
              //console.log(json); 
              vp_process_json(json, cid, reload, data);
          },
          function(xhr) {
              console.error(xhr);
          }
      );

  } else if (g_list[key]) { // prevent reloading if already loaded - doesn't work as they process in parallel

      var this_cid = '__vp_plugin-' + plugin_id;
      g_cids[this_cid] = 3; // success - using preloaded data
      var json = g_list[key];
      json.plugin_id = data.plugin_id;
      json.container_id = data.container_id;

      console.log("Done - key is already loaded:" + key + "cid=" + this_cid + " data:", data, " json:", json);
      vp_process_json(json, this_cid, reload, data);

  } else {

      console.log("Not loaded. Key=" + key + " Loading from ajax_url:" + ajax_url + data_str);
      //console.log("Calling ajax_url:" + ajax_url + data_str);
      var this_start = g_timer[plugin_id] = new Date().getTime();
      $jsonp.send(ajax_url + data_str, {
          callbackName: 'handleStuff',
          onSuccess: function(json) {
              var plugin_id = json.plugin_id;
              var cid = json.container_id;
              var data = {
                  'plugin_id': plugin_id,
                  'container_id': cid,
                  'obj_type': json.obj_type,
                  'obj_id': json.obj_id
              };
              var key = vp_element_key(data);

              var elapsed = new Date().getTime() - g_start;
              var this_time = new Date().getTime() - g_timer[plugin_id];

              g_cids[cid] = 2; // success

              var status = g_cids[cid];
              var plugin_ids = g_keys[key] || [];
              var len = plugin_ids.length;

              //console.log("Done. Loaded plugin " + plugin_id + " ('" + cid + "') in " + elapsed + "ms (" + this_time + "ms) status=" + status + " data:", data, " g_cids", g_cids, " key=" + key + " plugin ids:", plugin_ids);        
              //console.log("Done. Loaded plugin " + plugin_id + " len=" + len + " ids:", plugin_ids);
              console.log("Done. Data:", json);

              if (len > 1) {
                  // alert("More than one plugin for this data");
                  console.log("More than one plugin for this data", plugin_ids);
              }

              if (1) { /** old way */
                  //g_json_data[plugin_id] = json;

                  vp_process_json(json, cid, reload, data);

              } else { /** new way */
                  var this_id;
                  for (var i = 0; i < len; i++) {
                      this_id = plugin_ids[i];
                      var this_cid = '__vp_plugin-' + plugin_id;
                      var this_data = {
                          'plugin_id': plugin_id,
                          'container_id': this_cid,
                          'obj_type': json.obj_type,
                          'obj_id': json.obj_id
                      };
                      console.log("This ID:" + this_id + " this cid:" + this_cid + " data:", this_data);
                      json.plugin_id = this_id;
                      //g_json_data[plugin_id] = json;
                      vp_process_json(json, this_cid, reload, this_data);

                  }
              }

          },
          //onError: function(json) {
          //},
          onTimeout: function() {
              //console.log('load timeout!');
              g_cids[cid] = -2; // timeout

          },
          timeout: 5
      });
  }
}

function vp_process_json(json, cid, reload, data) {

  var plugin_id = data.plugin_id;
  var obj_type = data.obj_type;
  var obj_id = data.obj_id;

  //var key = obj_type + '-' + obj_id;
  var key = vp_element_key(data);
  console.log("loaded: vp_process on plugin id=" + plugin_id + " cid=" + cid + " data=", data);
  //console.log("vp_process json=", json);

  if (!g_local) cid = json.container_id;

  // update class of frame on client screen
  var element = document.getElementById(cid);
  if (!element) {
      console.log("Could not find element " + cid + " local=" + g_local);
      return false;
  }
  var button_id = "__vp-view-button-" + plugin_id;
  var view_button = document.getElementById(button_id);
  if (view_button) {
      console.log("Done loading set to view on button: " + button_id + " cid=" + cid);
      view_button.innerHTML = "View";
  } else {
      console.log("Could not find button " + button_id + " cid=" + cid);
  }

  var cn = element.className;
  var ncn = cn.replace(/transparent/, '');
  element.className = ncn;

  ////console.log("success Id: " + cid + " json: ", json);
  ////console.log("success Id: " + cid + " Changing class from " + cn + " => " + ncn);

  g_artists = json.artist_names || {};

  var i, item, items, psitem;
  var psitems = [];
  g_user_id = json.user_id;
  g_user_name = json.user_name;
  if (g_user_id) {
      g_follow = [];
      var f = json.follow;
      for (i = 0; i < f.length; i++) {
          var mid = f[i];
          g_follow[mid] = 1;
      }
  }

  items = json.items;
  for (i in items) {
      item = items[i];
      psitem = {};
      psitem.src = item.href;
      psitem.msrc = item.href_small ? item.href_small : item.href;
      psitem.w = item.w;
      psitem.h = item.h;
      psitem.title = item.title;
      psitem.data = item;
      psitems.push(psitem)
  }

  // define options (if needed)
  var options = {
      index: 0 // start at first slide

  };

  // Initializes and opens PhotoSwipe
  var pswpElement = document.querySelectorAll('.pswp')[0];
  // var gallery = new PhotoSwipe(element, PhotoSwipeUI_Default, psitems, options);

  element.setAttribute('data-pswp-uid', cid);
  element.onclick = onThumbnailsClick;

  console.log("register onclick on " + cid);
  // Initializes and opens PhotoSwipe
  // var gallery = new PhotoSwipe( pswpElement, PhotoSwipeUI_Default, items, options);
  json.psitems = psitems;
  json.element = element;
  json.data = data;

  //console.log("g_list for cid=" + cid + " and user:" + g_user_id + " = ", json);

  g_list[key] = json;

  if (reload && g_pswp) {
      g_cid = cid;

      g_pswp.items = psitems;

      // sets a flag that slides should be updated
      g_pswp.invalidateCurrItems();

      // updates content of slides
      g_pswp.updateSize(true);

      if (!g_user_id) {
          userBtn.style.display = "none";
      } else {
          userBtn.style.display = "block";
      }

  } else {
      //console.log("Not Reloading");
  }
}

/** Open PhotoSwipe Gallery */
var openPhotoSwipe = function(cid, index, galleryElement, disableAnimation, fromURL) {
  var pswpElement = document.querySelectorAll('.pswp')[0],
      gallery,
      options,
      items;

  var data = vp_element_data(cid);
  console.log("pradeepa " + data);
  var plugin_id = data.plugin_id;
  var key = vp_element_key(data);

  var json = g_list[key];
  if (!json) {
      //alert("no json in g_list for cid=" + cid);
      alert("Ooops, no data found");
      console.log("g_list:", g_list);
      return;
  }

  console.log("openPhotoSwipe data=", json);
  var events = json.events || [];
  var event_count = events.length;

  var curator = json.curator || {};
  var lat = curator.location_lat;
  var lng = curator.location_long;

  /** these determine if we should show respective buttons - if turned on in options */
  var has_info = true; /** todo */
  var has_event = event_count > 0;
  var has_location = Math.abs(lat) > 0 && Math.abs(lng) > 0;
  var has_contact = curator.email_address && isEmail(curator.email_address);

  console.log("open PhotoSwipe cid=" + cid);

  //console.log("open PhotoSwipe cid=" + cid + " plugin_id" + plugin_id + " key=" + key + " index=" + index + " data:", data);
  //console.log("g_list cid=" + cid + ":",g_list);
  //console.log("calendar event count for plugin_id: " + plugin_id + "=" + event_count + " collection_id=" + json.collection_id);
  //console.log("data:",json);

  g_cid = cid;
  items = json.psitems;
  g_sharer_id = json.curator_id;
  console.log('g_sharer_id=' + g_sharer_id);

  //fconsole.log("show/hide buttons according to popup buttons:", g_popup_buttons);

  if (typeof g_plugin_options == "undefined") var g_plugin_options = null;

  // define options (if needed)
  options = {
      barsSize: {
          top: 44,
          bottom: 'auto'
      },

      // Adds class pswp__ui--idle to pswp__ui element when mouse isn't moving for 4000ms
      timeToIdle: 4000,

      // Same as above, but this timer applies when mouse leaves the window
      timeToIdleOutside: 1000,

      // Delay until loading indicator is displayed
      loadingIndicatorDelay: 1000,

      // Function builds caption markup 
      addCaptionHTMLFn: function(item, captionEl, isFake) {
          // item      - slide object
          // captionEl - caption DOM element
          // isFake    - true when content is added to fake caption container
          //             (used to get size of next or previous caption)

          // test 12345
          /**
          if(!item.title) {
              captionEl.children[0].innerHTML = '';
              return false;
          }
          */

          /** todo: use -1 for no, 1 for yes, 0 for unknown */
          //console.log("calling vp_caption for title: data", item.data);
        var caption = vp_caption(json, item.data, 0);
        //   captionEl.children[0].innerHTML = caption;
       //   captionEl.children[0].innerHTML = 'fofoo2'; // caption;
       //    //captionEl.children[0].innerHTML = ''; // caption;
            captionEl.children[0].innerHTML = caption;

            return true;
        },

      // Buttons/elements
      closeEl: true,
      captionEl: true,

      zoomEl: true,
      counterEl: true,
      arrowEl: true,
      preloaderEl: true,

      // standard buttons - on by default
      fullscreenEl: g_popup_buttons ? inArray('fullscreen', g_popup_buttons) : true,
      shareEl: g_popup_buttons ? inArray('share', g_popup_buttons) : true,

      // new buttons - off by default
      infoEl: g_popup_buttons ? inArray('info', g_popup_buttons) && has_info : false,
      mapEl: g_popup_buttons ? inArray('map', g_popup_buttons) && has_location : false,
      likeEl: g_popup_buttons ? inArray('follow', g_popup_buttons) : false,
      chatEl: g_popup_buttons ? inArray('chat', g_popup_buttons) : false,
      userEl: g_popup_buttons ? inArray('user', g_popup_buttons) : false,
      contactEl: g_popup_buttons ? inArray('contact', g_popup_buttons) && has_contact : false,
      calendarEl: g_popup_buttons ? inArray('calendar', g_popup_buttons) && has_event : false,

      //contactEl: g_plugin_options ? g_plugin_options.show_contact_button : true,

      // Tap on sliding area should close gallery
      tapToClose: false,

      // Tap should toggle visibility of controls
      tapToToggleControls: true,

      // Mouse click on image should close the gallery,
      // only when image is smaller than size of the viewport
      clickToCloseNonZoomable: true,

      // Element classes click on which should close the PhotoSwipe.
      // In HTML markup, class should always start with "pswp__", e.g.: "pswp__item", "pswp__caption".
      // 
      // "pswp__ui--over-close" class will be added to root element of UI when mouse is over one of these elements
      // By default it's used to highlight the close button.
      closeElClasses: ['item', 'caption', 'zoom-wrap', 'ui', 'top-bar'],

      // Separator for "1 of X" counter
      indexIndicatorSep: ' / ',

      // Share buttons
      // 
      // Available variables for URL:
      // {{url}}             - url to current page
      // {{text}}            - title
      // {{image_url}}       - encoded image url
      // {{raw_image_url}}   - raw image url
      // {{obj_id}}          - media_collection_map ID (vpatina)
      shareButtons: [{
              id: 'facebook',
              label: 'Facebook',
              url: 'https://www.facebook.com/sharer/sharer.php?u={{url}}'
          }, {
              id: 'twitter',
              label: 'Tweet',
              url: 'https://twitter.com/intent/tweet?text={{text}}&url={{url}}'
          }, {
              id: 'pinterest',
              label: 'Pin it',
              url: 'http://www.pinterest.com/pin/create/button/?url={{url}}&media={{image_url}}&description={{text}}'
          }, {
              id: 'email',
              label: 'E-mail',
              url: g_host_url + '/share/?url={{image_url}}&obj_type=media_collection_map&obj_id={{obj_id}}&_aid=10&template=mail/mail-artwork.html&controller=gallery_artworks'
          }
          //{id:'download', label:'Download image', url:'{{raw_image_url}}', download:true}
      ],

      // Next 3 functions return data for share links
      // 
      // functions are triggered after click on button that opens share modal,
      // which means that data should be about current (active) slide
      getImageURLForShare: function(shareButtonData) {
          // `shareButtonData` - object from shareButtons array
          // 
          // `pswp` is the gallery instance object,
          // you should define it by yourself
          // 
          var vpdata = g_pswp.getCurrentData(cid) || {};
          var item = vpdata.item;
          //console.log("sharing using global g_pswp link=" + item.href + " item=",item);

          return item.href;

          //return g_pswp.currItem.src || '';
      },
      getPageURLForShare: function(shareButtonData) {
          var vpdata = g_pswp.getCurrentData(cid) || {};
          var item = vpdata.item;
          var link = item.link;

          var sharer_id = g_sharer_id ? g_sharer_id : 1;
          var tail = "_aid=" + sharer_id;
          link = link.indexOf('?') > -1 ? link + "&" + tail : link + "?" + tail;
          //console.log("sharing using global g_pswp v2 link=" + link + " item=",item);
          return link;
          //return window.location.href;
      },
      getTextForShare: function(shareButtonData) {
          var vpdata = g_pswp.getCurrentData(cid) || {};
          var item = vpdata.item;
          var caption = vp_caption(json, item, 2);
          //console.log("getTextForShare: item=",item);
          //logStackTrace(5);
          return caption;
      },
      getObjIdForShare: function(shareButtonData) {
          var vpdata = g_pswp.getCurrentData(cid) || {};
          var item = vpdata.item;
          console.log("getObjId for share: item=", item, " id=", item.map_id);
          return item.map_id;
      },

      // Parse output of share links
      parseShareButtonOut: function(shareButtonData, shareButtonOut) {
          // `shareButtonData` - object from shareButtons array
          // `shareButtonOut` - raw string of share link element
          return shareButtonOut;
      }
  };

  // PhotoSwipe opened from URL
  if (fromURL) {
      if (options.galleryPIDs) {
          // parse real index when custom PIDs are used
          // http://photoswipe.com/documentation/faq.html#custom-pid-in-url
          for (var j = 0; j < items.length; j++) {
              if (items[j].pid == index) {
                  options.index = j;
                  break;
              }
          }
      } else {
          // in URL indexes start from 1
          options.index = parseInt(index, 10) - 1;
      }
  } else {
      options.index = parseInt(index, 10);
  }

  // exit if index not found
  if (isNaN(options.index)) {
      return;
  }

  if (disableAnimation) {
      options.showAnimationDuration = 0;
  }

  /** added vPatina */
  options.escKey = true;
  options.closeOnScroll = false;
  options.closeOnVerticalDrag = false;
  options.pinchToClose = false;

  options.clickToCloseNonZoomable = false;
  options.tapToClose = false;
  options.showAnimationDuration = 0;

  //console.log("Start PS with options:",options);
  // Pass data to PhotoSwipe and initialize it

  //console.log("Start PS with items:",items);

  gallery = new PhotoSwipe(pswpElement, PhotoSwipeUI_Default, items, options);
  gallery.init();

  g_pswp = gallery;
  g_pswp.cid = cid;
  g_pswp.vpdata = json;
  g_pswp.galleryElement = galleryElement;

  var content = ' [empty] ';

  g_pswp.listen('close', function() {
      console.log("closing gallery");
      vp__close_modal();
  });

  //console.log("\n\n\naddress:"  + address_formatted + "\n\n\n lat=" + lat + " lng=" + lng );
  //console.log("gh: curator=", curator);

  //modalEl.data = {};                                                                     

  //var chatBtn = document.getElementById("chatBtn");  
  //var closeBtn = document.getElementById("closeBtn");  
  //var infoBtn = document.getElementById("infoBtn");
  //var contactBtn = document.getElementById("contactBtn");
  //var calendarBtn = document.getElementById("calendarBtn");
  //var mapBtn = document.getElementById("mapBtn");
  //
  //// When the user clicks the button, open the modal
  //chatBtn.onclick = function() {
  //  openModal();
  //  modalEl.data.owner = "chat";
  //  modal_content.innerHTML = infoiframe;
  //}

  // When the user clicks the button, open the modal
  //mapBtn.onclick = function() {
  //  openModal();
  //  modalEl.data.owner = "map";
  //  modal_content.innerHTML = vp_infomap(json);    
  //}

  /**
  infoBtn.onclick = function() {

    //modal_content.innerHTML = infotabs;
    // openModal();
    console.log("info Btn click - do nothing...");
    return;

    share_modal_content.innerHTML = vp_infotabs();
    _toggleShareModal();
    modalEl.data.owner = "info";                                                                                           

    var vpdata = g_pswp.getCurrentData(cid) || {};
    vp_update_info(vpdata);          

  }

  calendarBtn.onclick = function() {
    modalEl.data.owner = "calendar";
    //openModal();                                                                         
  }

  contactBtn.onclick = function() {
  }

  //closeBtn.onclick = function() {
  //  closeModal();
  //  //console.log("click on chatBtn owner=" + modalEl.data.owner);
  //  if(modalEl.data.owner == 'chat') reloadPhotoswipe(cid, index, galleryElement, disableAnimation, fromURL);
  //}

  */

  /** move to photoswipe-ui-default.js */
  /**
var likeBtn = document.getElementById("likeBtn");
// When the user clicks the button, open the modal
likeBtn.onclick = function() {
  console.log("click on like");
  var data = g_pswp.getCurrentData(cid);
  console.log("data=", data);
  if(g_user_id) {
    console.log("click on like by user");
    g_pswp.like(cid, data);

  } else {
    var link = "/login/"; 
    var share_modal_content = document.getElementById('vp__share-modal-content');
    share_modal_content.innerHTML = vp_iframe(link);      
    //_toggleShareModal();
    console.log("click on like - no user");
  }
}
*/

  var userBtn = document.getElementById("userBtn");
  if (!g_user_id) {
      userBtn.style.display = "none";
  } else {
      userBtn.style.display = "block";
      userBtn.setAttribute('title', "Logged in as " + g_user_name);
  }

  g_pswp.listen('afterChange', function(data) {

      var vpdata = g_pswp.getCurrentData(cid);
      //console.log("gcd for cid=" + cid + " returned ",vpdata);
      var item = vpdata.item;

      var p = vp_price(item);
      var h = item.original_height;
      var w = item.original_width;
      var t = item.original_technique;
      var c = item.original_currency;
      var ti = item.title;
      var mid = item.media_id;
      var str = ti + " " + t + " (" + h + "x" + w + ") " + p + " " + c;
      //console.log("Slide changed: index=" + g_pswp_index + " str=" + str + " mid=" + mid);
      ////console.log(psitem);
      //alert(str);

      var vpdata = g_pswp.getCurrentData(cid) || {};

      var item = vpdata.item;
      var comment = item.comment;

      vp_update_info(vpdata);

      var div = document.createElement("div");
      div.innerHTML = comment;
      var text = div.textContent || div.innerText || "";

      var infoBtn = document.getElementById("vp__infoBtn");
      //console.log('comment=' + comment);
      //console.log('text="' + text + '" len=' + text.length);
      if (1 || text.length > 2) { // comment stripped of tags
          infoBtn.style.display = 'block';
          //console.log("showing infoBtn");
      } else {
          infoBtn.style.display = 'none';
          //console.log("hiding infoBtn");
      }

      var follow = g_user_id && g_follow.length && g_follow[mid]; // inArray(mid, g_follow);

      //console.log("Reload is true mid=" + mid + " user_id=" + g_user_id + " follow len=" + g_follow.length + " follow=" + follow);

      var likeBtn = document.getElementById("likeBtn");
      if (follow) {
          if (!likeBtn.classList.contains('follow')) likeBtn.classList.add('follow');
          likeBtn.setAttribute('title', "You follow " + item.title);
      } else {
          if (likeBtn.classList.contains('follow')) likeBtn.classList.remove('follow');
          likeBtn.setAttribute('title', "Click to follow " + item.title);
      }
      //var star = follow ? '<i class="fa fa-star"></i>' : '<i class="fa fa-star-o"></i>';
      //likeBtn.innerHTML = star;

  });

  g_pswp.like = function(cid, vpdata) {
      var ajax_url = g_site_url + '/ajax.php?oper=gallery-favorite&format=json&jsoncallback=likeStuff';

      //$('#galleria .galleria-stage').addClass("loading-big");

      //console.log("like: data=", vpdata);

      var item = vpdata.item;
      var obj_type = 'media';
      var mid = item.media_id;
      var data_str = "&site_id=" + vpdata.site_id + "&user_id=" + vpdata.user_id + "&obj_type=" + vpdata.obj_type + "&obj_id=" + mid + "&plugin_id=" + vpdata.plugin_id + "&target=" + cid;

      //console.log("like: url=", ajax_url);

      var likeBtn = document.getElementById("likeBtn");
      var spin = '<i class="fa fa-spinner fa-spin"></i>';
      likeBtn.innerHTML = spin;

      $jsonp.send(ajax_url + data_str, {
          callbackName: 'likeStuff',
          onSuccess: function(json) {
              ////console.log("like success: url=" + ajax_url + " result:", json);
              var id = json.id;

              if (id) {
                  if (!likeBtn.classList.contains('follow')) likeBtn.classList.add('follow');
                  likeBtn.setAttribute('title', "You follow " + item.title);

              } else {
                  if (likeBtn.classList.contains('follow')) likeBtn.classList.remove('follow');
                  likeBtn.setAttribute('title', "Click to follow " + item.title);
              }
              //var star = id ? '<i class="fa fa-star"></i>' : '<i class="fa fa-star-o"></i>';
              ////console.log("follow was:", json);
              g_follow[mid] = id;

              likeBtn.innerHTML = '';
          },
          onTimeout: function() {
              //console.log('like timeout!');
          },
          timeout: 5
      });
  };

  g_pswp.getCurrentData = function(cid) {
      var index = this.getCurrentIndex();
      var data = vp_element_data(cid);
      var key = vp_element_key(data);
      var json = g_list[key];

      var vpdata = json;

      var psitems = json.psitems;
      var psitem = psitems[index];

      vpdata.index = index;
      vpdata.item = psitem.data
      console.log('vpdata')
      console.log(vpdata)

          //console.log("get current data for cid=" + cid + " index=" + index + " data:", vpdata);
      return vpdata;
  };

};

function vp_element_data(cid) {
  var vpElement = document.getElementById(cid);
  var data = vpElement && vpElement.getAttribute('data-data');
  return JSON.parse(data);
}

function vp_element_key(data) {
  if(!data) return;
  var plugin_id = data.plugin_id;
  var obj_type = data.obj_type;
  var obj_id = data.obj_id;
  var key = obj_type + '-' + obj_id;
  return key;
}

/** Reload gallery data and redraw PhotoSwipe */
function reloadPhotoswipe(cid, index, galleryElement, disableAnimation, fromURL) {
  var data = vp_element_data(cid);
  var key = vp_element_key(data);
  var json = g_list[key];
  g_cid = cid;
  items = json.psitems;
  console.log("Reloading ps cid=" + cid + " index=" + index + " on element:", galleryElement);

  vp_load(json.element, json.data, true);

}

// triggers when user clicks on thumbnail
var onThumbnailsClick = function(e) {

  e = e || window.event;
  e.preventDefault ? e.preventDefault() : e.returnValue = false;

  var eTarget = e.target || e.srcElement;

  // find root element of slide
  var clickedListItem = closest(eTarget, function(el) {
      return (el.tagName && (el.tagName.toUpperCase() === 'FIGURE' || el.tagName.toUpperCase() === 'OBJECT'));
  });

  var cid = clickedListItem.getAttribute('data-pswp-uid');
  var index = clickedListItem.getAttribute('data-index') || 0;
  var plugin_options = clickedListItem.getAttribute('data-plugin_options');
  var popup_options = clickedListItem.getAttribute('data-popup_options');
  g_plugin_options = plugin_options ? JSON.parse(plugin_options) : {};
  g_popup_options = popup_options ? JSON.parse(popup_options) : {};

  g_popup_buttons = g_popup_options.buttons || ["info", "contact", "map", "zoom", "share", "fullscreen"];
  g_popup_tabs = g_popup_options.tabs || ["artwork", "artist", "exhibition", "gallery"];
  g_popup_title = g_popup_options.title || ["dot", "artwork", "artist", "year"];

  console.log("buttons for " + cid + " = ", g_popup_buttons);
  console.log("tabs for " + cid + " = ", g_popup_tabs);
  zoomOutMobile();

  openPhotoSwipe(cid, index, clickedListItem);

  return;
};

function vp_event_button(event, curator) {
  var timezone = event.timezone || 'Europe/Amsterdam';
  return '' +
      '<span class="addtocalendar atc-style-blue">' +
      '<a class="atcb-link" tabindex="1">' +
      '<i class="fa fa-calendar"></i> ' + 'Add to Calendar' +
      '<button class="pswp__button pswp__button--calendar" title="Calendar"></button>' +
      '</a>' +
      '<var class="atc_event">' +
      '<var class="atc_date_start">' + event.start_time + '</var>' +
      '<var class="atc_date_end">' + event.end_time + '</var>' +
      '<var class="atc_timezone">' + timezone + '</var>' +
      '<var class="atc_title">' + event.title + '</var>' +
      '<var class="atc_description">' + event.description + '</var>' +
      '<var class="atc_location">' + event.start_address + '</var>' +
      '<var class="atc_organizer">' + curator.display_name + '</var>' +
      '<var class="atc_organizer_email">' + curator.email_address + '</var>' +
      '</var>' +
      '</span>';
}

function vp_events(vpdata) {
  var events = vpdata.events || [];
  var event_count = events.length;
  var output = '';
  var evtString, event, button;
  var curator = vpdata.curator || {};
  output = '';

  for (var i = 0; i < event_count; i++) {
      event = events[i];
      output = output + vp_event(event, curator) + '<br><br>';

      //button = vp_event_button(event, curator);     
      //evtString = '';
      //evtString = evtString + '<h4>' + event.title + '</h4>';
      //evtString = evtString + event.start_time + ' ' + button + '<br>';
      //evtString = evtString + event.start_address + '<br>';
      //output = output + '<div class="vp__event">' + evtString + "</div>";
  }
  return output;
}

function vp_get_date(datetime, time, timezone) {
  if (typeof time == "undefined") var time = 0;
  if (typeof timezone == "undefined") var timezone = '';

  var t = datetime.split(/[- :]/); // Split timestamp into [ Y, M, D, h, m, s ]
  console.log("dt:", datetime);
  console.log("t:", t);

  var time_str = t[3] + ':' + t[4];
  if (time) return time_str;

  if (time && !timezone) return time_str;

  var date = new Date(Date.UTC(t[0], t[1] - 1, t[2], t[3], t[4], t[5])); // Apply each element to the Date function
  var monthNames = [
      "January", "February", "March",
      "April", "May", "June", "July",
      "August", "September", "October",
      "November", "December"
  ];

  var day = date.getDate();
  var monthIndex = date.getMonth();
  var year = date.getFullYear();
  var hour = date.getHours();
  var minute = date.getMinutes();

  console.log(day, monthNames[monthIndex], year, hour, minute);
  if (time) {
      //return time_str;
      return hour + ':' + minute;
  } else {
      return day + ' ' + monthNames[monthIndex] + ' ' + year;
  }
}

function vp_event(event, curator) {
  var address_formatted = event.address_formatted || event.start_address;
  var address_anchor = '';
  if (address_formatted) {
      var address_link = "https://maps.google.com/?q=" + encodeURIComponent(address_formatted);
      address_anchor = '<a href="' + address_link + '" target="new">' + 'View in Google Maps' + '</a>';
  }

  var start_date = vp_get_date(event.start_time);
  var end_date = vp_get_date(event.end_time);
  var start_time = vp_get_date(event.start_time, 1, event.timezone);
  var end_time = vp_get_date(event.end_time, 1, event.timezone);

  console.log("start date=" + start_date);
  console.log("end date=" + end_date);
  console.log("start time=" + start_time);
  console.log("end time=" + end_time);

  var datetime = start_date + " at " + start_time + "-" + end_time;

  var calendar_link = vp_event_button(event, curator);
  var output = '' +
      '<table cellpadding="4" cellspacing="4" class="vp__event">' +
      //  '<tr>'+            
      //  '  <td align="left"><br>'+ canvas +
      //  '  </td>'+
      //  '</tr>'+
      '  <tr>' +
      '    <td align="left">' + '<br>' +
      '      <h4 style="font-size:20px;margin-bottom:4px">' + event.title + '</h4>' +
      '      <p style="font-size:16px">' + datetime + ' ' + calendar_link + '<br>' + event.address_street + ' ' + (event.address_number && event.address_number > 0 ? event.address_number : '') + '<br>' +
      '         ' + event.address_city + '<br><br>' +
      '         ' + address_anchor + '<br><br></p>' +
      '    </td>' +
      '  </tr>' +
      '</table>';
  return output;
}

function vp_follow_link(user_id, item) {
  if (!user_id || !item || !item.media_id) return '';
  return '/login/?ref=%2Fmy-collection%2F%3F_aif%3D' + item.media_id + '%26_aid%3D' + user_id;
}

function vp_contact_link(cid) {
  //modalEl.data.owner = "contact";

  var get_obj = cid.split('_')[0];
  var vpdata = g_pswp.getCurrentData(cid) || {};
  var item = vpdata.item;

  var curator = vpdata.curator || {};
  var email = curator.email_address || '';
  var curator_name = curator.display_name || 'gallery';
  var artist_name = vp_artist_name(vpdata, item.artist_id);

  console.log("contact item: ", item);

  // subject: Curators Name : Collector Inquiry on “Title”
  var title = item.title || 'Untitled Image'
  var subject = curator_name + ': Collector Inquiry on &quot;' + title + "&quot; by " + artist_name;

  var recipient = email ? curator_name + " <" + email + ">" : '';
  if (get_obj == 'artworkmedia')
      var obj_type = 'artwork_media';
  else
      var obj_type = 'media_collection_map';
  var obj_id = item.map_id ? item.map_id : cid.split('_')[1];

  var link = g_host_url + "/share/?recipient=" + encodeURIComponent(recipient) + "&subject=" + encodeURIComponent(subject);
  link = link + "&obj_type=" + obj_type + "&obj_id=" + obj_id;

  if (g_user_id && vpdata.user) {
      var sender = "";
      var user = vpdata.user;
      var user_name = user.display_name;
      var user_email = user.email_address;
      var sender = user_name + " <" + user_email + ">";
      link = link + "&sender=" + encodeURIComponent(sender);
  }

  return link;
}

function vp_container(str, container_class) {
  container_class = container_class + ' vp__container'
  return '<div class="' + container_class + '">' + str + '</div>';
}

function vp_infocalendar(cid) {
  var vpdata = g_pswp.getCurrentData(cid) || {};
  var events = vpdata.events;
  return vp_events(vpdata);
}

function vp_infomap(json) {

  var curator = json.curator || {};

  var details = '';
  var hasmap = false;

  if (address_formatted = curator.address_formatted) {
      var address_link = "https://maps.google.com/?q=" + encodeURIComponent(address_formatted);
      var address_anchor = '<a href="' + address_link + '" target="new">' + 'View in Google Maps' + '</a>';
      //details = details + ' Address:' + address_anchor + '<br>';
  }

  var lat = curator.location_lat;
  var lng = curator.location_long;

  if (Math.abs(lat) > 0 && Math.abs(lng) > 0) {
      hasmap = true;
      var canvas = '<div id="mapCanvas" class="" style="width:286px;height:200px" data-lat="' + lat + '" data-lng="' + lng + '"></div>';
  }

  var output = '' +
      '<table cellpadding="4" cellspacing="4">' +
      '<tr>' +
      '  <td align="left"><br>' + canvas +
      '  </td>' +
      '</tr>' +
      '<tr>' +
      '  <td align="left">' + '<br>' +
      '    <h4 style="font-size:20px;margin-bottom:4px">' + curator.display_name + '</h4>' +
      '    <p style="font-size:16px">' + curator.address1 + '<br>' +
      '       ' + curator.city + '<br><br>' +
      '       ' + address_anchor + '<br><br></p>' +
      '  </td>' +
      '</tr>' +
      '</table>';
  return output;
}

function vp_infotabs(a) {
    console.log('Ashii----------')
    console.log(a.item.description)
    console.log('Ashii----------')

    var output = '' +
      '  <ul class="w3-pagination w3-white w3-border-bottom vp__tabs" style="width:100%;" id="infoTabs">' +
      '   <li><a id="vp__tab1" class="vp__tab-links active" onclick="return vp_openTab(event, \'vp__tab1-content\')">Art</a></li>' +
      '   <li><a id="vp__tab2" class="vp__tab-links" onclick="return vp_openTab(event, \'vp__tab2-content\')">Artist</a></li>';


    if(a.item.description!="") 
        output=  output + '   <li><a id="vp__tab5" class="vp__tab-links" onclick="return vp_openTab(event, \'vp__tab5-content\')">About</a></li>';

    output = output +  '   <li><a id="vp__tab3" class="vp__tab-links" onclick="return vp_openTab(event, \'vp__tab3-content\')">Exhibition</a></li>' +
      '   <li><a id="vp__tab4" class="vp__tab-links" onclick="return vp_openTab(event, \'vp__tab4-content\')">Gallery</a></li>' +
      '  </ul>' +
      '  <div id="vp__tab1-content" class="w3-container vp__tab-content">' +
      '  </div>' +
      '  <div id="vp__tab2-content" class="w3-container vp__tab-content">' +
      '  </div>' +
      '  <div id="vp__tab5-content" class="w3-container vp__tab-content">' +
      '  </div>' +
      '  <div id="vp__tab3-content" class="w3-container vp__tab-content">' +
      '  </div>' +
      '  <div id="vp__tab4-content" class="w3-container vp__tab-content">' +
      '  </div>';
  return output;
}

function vp_popup(link) {
  var full_link = vp_full_link(link);
  console.log("vp_popup: link=" + full_link);
  window.open(full_link, 'pswp_share', 'scrollbars=yes,resizable=yes,toolbar=no,' +
      'location=no,width=640,height=620,top=100,left=' +
      (window.screen ? Math.round(screen.width / 2 - 275) : 100));
}

function vp_full_link(link) {
  var sharer_id = g_sharer_id ? g_sharer_id : 1;
  var tail = "_aid=" + sharer_id;
  var full_link = link.indexOf('?') > -1 ? link + "&" + tail : link + "?" + tail;
  var domain_link = full_link.indexOf('http') > -1 ? full_link : g_site_url + full_link;
  return domain_link;
}

function vp_iframe(link) {
  var sharer_id = g_sharer_id ? g_sharer_id : 1;
  var tail = "_aid=" + sharer_id;
  link = link.indexOf('?') > -1 ? link + "&" + tail : link + "?" + tail;
  return '<iframe width="100%" height="100%" style="min-height:800px" src="' + g_site_url + link + '"></iframe>';
}

/** find all vPatina photoswipe elements on page - load data */
function vp_search() {
  var vpElements = document.querySelectorAll('.vp-loader-photoswipe')
  var len = vpElements.length;
  //console.log("Load: Found " + len + " ps items gcids:", g_cids);
  console.log("Load: Found " + len + " ps items g_keys:", g_keys);
  
  for (var i = 0; i < len; i++) {
      var vpElement = vpElements[i];
      var data = vpElement && vpElement.getAttribute('data-data');
      data = JSON.parse(data);
      console.log("Pradeepa Plugin Id " + data);
      var key = vp_element_key(data);

      var cid = vpElement.getAttribute('id');
      var plugin_id = data.plugin_id;

      var status = g_cids[cid];
      var st2 = g_keys[key];
      if (!g_keys[key]) g_keys[key] = [];

      var plugin_ids = g_keys[key];

      console.log("Found plugin id:" + plugin_id + " cid:" + cid + " key: " + key + " status:" + status + " g_keys:", g_keys);
      if (0 && plugin_ids.length) {
          console.log("Already loading data for plugin id:" + plugin_id + " cid:" + cid + " status:" + status + " key:" + key + " plugin ids=", plugin_ids);
          if (!inArray(plugin_id, g_keys[key])) g_keys[key].push(plugin_id);

      } else if (!status) {
          g_cids[cid] = 1; // loading
          g_keys[key].push(plugin_id);
          console.log("Loading data for plugin id:" + plugin_id + " cid:" + cid + " status:" + status + " data:", data);
          vp_load(vpElement, data, false);
      } else {
          console.log("Not loading data for plugin id:" + plugin_id + " cid:" + cid + " status:" + status);
      }
  }
}

function initModalMap(map_id) {
  //var lat = $("#" + map_id).data('lat'); 
  //var lng = $("#" + map_id).data('lng');
  var mapEl = document.getElementById(map_id);
  var lat = mapEl && mapEl.getAttribute('data-lat');
  var lng = mapEl && mapEl.getAttribute('data-lng');
  //console.log("initModalMap at " + lat + "/" + lng);

  var mapOptions = {
      center: new google.maps.LatLng(lat, lng),
      zoom: 12,
      mapTypeId: google.maps.MapTypeId.ROADMAP
  };
  var map = new google.maps.Map(document.getElementById(map_id),
      mapOptions);
  var marker = new google.maps.Marker({
      position: new google.maps.LatLng(lat, lng)
  });
  marker.setMap(map);
}

//console.log("Adding event listener...");

/** Begin runtime */
//document.addEventListener("DOMContentLoaded", function(event) {
////console.log("DOM Loaded");
vp_search();
//});

var test = 'kj-v0.0.4';

function vp_openTab(evt, tabName) {
  console.log("open tab " + tabName);
  var i, tabcontent, tablinks;
  tabcontent = document.getElementsByClassName("vp__tab-content");
  for (i = 0; i < tabcontent.length; i++) {
      tabcontent[i].style.display = "none";
  }

  // Get all elements with class="tablinks" and remove the class "active"
  tablinks = document.getElementsByClassName("vp__tab-links");
  for (i = 0; i < tablinks.length; i++) {
      tablinks[i].className = tablinks[i].className.replace(" active", "");
  }

  // Show the current tab, and add an "active" class to the link that opened the tab
  document.getElementById(tabName).style.display = "block";
  evt.currentTarget.className += " active";
}

function openTab(evt, cityName) {
  var i, x, tablinks;
  x = document.getElementsByClassName("city");
  for (i = 0; i < x.length; i++) {
      x[i].style.display = "none";
  }
  tablinks = document.getElementsByClassName("tablink");
  for (i = 0; i < x.length; i++) {
      tablinks[i].classList.remove("w3-light-grey");
  }
  document.getElementById(cityName).style.display = "block";
  evt.currentTarget.classList.add("w3-light-grey");
}

function openModal() {
  var modalEl = document.getElementById('vp__w3modal');
  //console.log("Open modal");
  modalEl.style.display = 'block';
}

function closeModal() {
  var modalEl = document.getElementById('vp__w3modal');
  //console.log("Close modal");
  modalEl.style.display = 'none';
}

function zoomOutMobile() {
  var viewport = document.querySelector("meta[name=viewport]");
  var scale = 'width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=0';
  //var scale = 'width=device-width, initial-scale=1';
  //var scale = 'width=device-width, initial-scale=1, maximum-scale=1.0';

  if (viewport) {
      viewport.setAttribute('content', scale);
      //viewport.content = "initial-scale=0.1";
      //viewport.content = "width=1200";
  } else {
      var metaTag = document.createElement('meta');
      metaTag.name = "viewport"
      metaTag.content = scale;
      document.getElementsByTagName('head')[0].appendChild(metaTag);
  }

  // after updating viewport tag, force the page to pick up changes           
  document.body.style.opacity = .9999;
  setTimeout(function() {
      document.body.style.opacity = 1;
  }, 1);

  //alert("zoomed...");
}

function inArray(needle, haystack) {
  var length = haystack.length;
  for (var i = 0; i < length; i++) {
      if (haystack[i] == needle) return true;
  }
  return false;
}

function appendHtml(el, str) {
  var div = document.createElement('div');
  div.innerHTML = str;
  while (div.children.length > 0) {
      el.appendChild(div.children[0]);
  }
}

/** simplified version: for full RFC 2822, see http://stackoverflow.com/questions/46155/validate-email-address-in-javascript */
function isEmail(em) {
  var re = /^(([^<>()\[\]\\.,;:\s@"]+(\.[^<>()\[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
  return re.test(em);
}

// When the user clicks anywhere outside of the modal, close it
window.onclick = function(event) {
  var modalEl = document.getElementById("vp__my_modal");
  if (event.target == modalEl) vp__close_modal();
}

function vp__close_modal() {
  var modalEl = document.getElementById("vp__my_modal");
  modalEl.style.display = "none";
}

/**
var modal = '' +
'<div id="vp__w3modal" class="w3-modal">' +
' <div class="w3-modal-content w3-card-4 w3-animate-zoom">' +
'  <header class="w3-container"> ' +
'   <span onclick="document.getElementById(\'vp__w3modal\').style.display=\'none\'" class="w3-closebtn">&times;</span>' +
'  </header>' +
'  <div  id="vp__w3-modal-content" class="vp__modal-content"><br><br>Loading...<br><br></div>' +
'  <div class="w3-container w3-light-grey w3-padding">' +
'   <button class="w3-btn w3-right w3-white w3-border" ' +
'   onclick="document.getElementById(\'vp__w3modal\').style.display=\'none\'">Close</button>' +
'  </div>';   
' </div>' +     
'</div>';                                                            

appendHtml(document.body, modal);
*/

//console.log("\n\n\nLoaded...\n\n\n");
//zoomOutMobile();