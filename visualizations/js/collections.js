/**
 * Collections for the application.  Specifically a collection
 * of bike animations.
 */
(function($, window, undefined) {

// Check our namespace
window.MinnPost = window.MinnPost || {};

var BikeAnimations = window.MinnPost.BikeAnimations = Backbone.Collection.extend({
  
  model: window.MinnPost.BikeAnimation

});

})(jQuery, window);