/**
 * Routes for the application.
 */
(function($, window, undefined) {

// Check our namespace
window.MinnPost = window.MinnPost || {};

var BikeApplicationRoute = window.MinnPost.BikeApplicationRoute = Backbone.Router.extend({

  routes: {
    'slow': 'slow',
    'fast': 'fast'
  },

  slow: function() {

  },

  fast: function() {

  }

});

})(jQuery, window);