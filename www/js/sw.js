navigator.serviceWorker && navigator.serviceWorker.register('./afr/js/sw.js').then(function(registration) {
  console.log('Excellent, registered with scope: ', registration.scope);
});