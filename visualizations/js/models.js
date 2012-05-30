/**
 * Models for application.  Specifically for a bike animation.
 */
(function($, window, undefined) {

// Check our namespace
window.MinnPost = window.MinnPost || {};

var BikeAnimation = window.MinnPost.BikeAnimation = Backbone.Model.extend({

  options: {
    'markerCircleOptions': {
      stroke: false,
      fillColor: '#10517F',
      fillOpacity: 0.98,
      clickable: false,
      radius: 8
    },
    'timeAnimLengthCapSecs': 5 * 60 * 60
  },

  initialize: function(attr, route) {
    // Initialize some values.  Note .d is durection, and JS wants milliseconds.
    var time = (this.get('d') < this.options.timeAnimLengthCapSecs) 
      ? this.get('d') * this.get('timeRatio') * 1000 
      : this.options.timeAnimLengthCapSecs * this.get('timeRatio') * 1000;
    
    this.set('route', route.r);
    this.set('time', time);
    this.set('dist', mapExtender.lineDistance(route.r));
    this.set('marker', new L.CircleMarker(new L.LatLng(0, 0), this.options.markerCircleOptions));
    
    // Add marker to map
    
    // Create animation object
    this.animation = new Animator({
      duration: time,
      interval: this.get('animInterval'),
      transition: Animator.tx.linear,
      onComplete: function() {
        // As stop will call this as well, check for state
        if (this.state == 1) {
          //map.removeLayer(marker);
          // Remove marker
        }
      }
    });
    this.animation.addSubject(this.animateMarker);
  },
  
  animateMarker: function(interval) {
    var dist = interval * this.get('dist');
    var coord = mapExtender.coordAtDistance(this.get('route'), dist);
    this.marker.setLatLng(new L.LatLng(coord[1], coord[0]));
  }
  
});

})(jQuery, window);