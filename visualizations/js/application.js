/**
 * Main application for Nice Ride app.
 */
      
// Namespace some things
(function($, window, undefined) {
  $(document).ready(function() {

    // Create and kick off the application.    
    var app = new MinnPost.BikeApplication({ el: $('#minnpost-nice-ride-application') });
    
  });
})(jQuery, window);