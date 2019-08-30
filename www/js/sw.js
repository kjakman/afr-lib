/** An empty service worker! */
self.addEventListener('fetch', function(event) {
  /** An empty fetch handler! */
});

navigator.serviceWorker && navigator.serviceWorker.register('/afr/js/sw.js').then(function(registration) {
  console.log('Excellent, registered with scope: ', registration.scope);
});