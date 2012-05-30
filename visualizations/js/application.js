/**
 * Main application for Nice Ride app.
 */
      
// Namespace some things
(function($, window, undefined) {
  $(document).ready(function() {
    $('.loading').html('Loading map..');
    
    // Start map
    var map = new L.Map('map', { maxZoom: 15, minZoom: 8 });
    
    // Minnnpost base map
    minnpost = new L.TileLayer('http://{s}.tiles.minnpost.com/minnpost-basemaps/minnpost-minnesota-greyscale-no-labels/{z}/{x}/{y}.png', 
     { attribution: 'Map imagery from <a target="_blank" href="http://minnpost.com">MinnPost</a>; Map data from <a target="_blank" href="http://openstreetmap.org">OpenStreetMap</a>.',
      scheme: 'tms' });
    map.addLayer(minnpost);
    
    // Route layer
    routes = new L.TileLayer('http://{s}.tiles.minnpost.com/minnpost-nice-ride/minnpost-nice-ride-routes/{z}/{x}/{y}.png', 
      { attribution: 'Data provided by <a target="_blank" href="https://www.niceridemn.org/">Nice Ride MN</a>; ',
      scheme: 'tms' });
    map.addLayer(routes);
    
    // Labels
    minnpost_l = new L.TileLayer('http://{s}.tiles.minnpost.com/minnpost-basemaps/minnpost-minnesota-greyscale-labels/{z}/{x}/{y}.png', 
     { scheme: 'tms' });
    map.addLayer(minnpost_l);
    
    // Set view
    map.setView(new L.LatLng(44.9745, -93.2513), 13);
    
    // A place for animations, rentals, and routes.  (seekFromTo(0.25, 0.75))
    var animations = {};
    var rentals_loaded = {};
    var routes_loaded = {};
    
    var da_flot = {};
    
    // Callback once animations and routes have been loaded.
    var loadAnimations = function() {
      // Time in seconds
      var time_day = 24 * 60 * 60;
      var time_anim = 2 * 60; // 20 minute animation
      var time_ratio = time_anim / time_day;
      var anim_date = { // 2011-05-18
        'y': 2011,
        'm': 4,
        'd': 18
      };
      var day_offset = ((4 * 60) + 30) * 60; // 4:30
      
      // Animation options
      var anim_interval = 100;
      var circleOptions = {
        stroke: false,
        fillColor: '#10517F',
        fillOpacity: 0.98,
        clickable: false,
        radius: 8
      };
    
      // Create each animation up front.
      var i;
      for (i in rentals_loaded) {
        // TODO: Not taking into account X to X routes
        if (typeof routes_loaded[rentals_loaded[i].st + '-' + rentals_loaded[i].et] != 'undefined') {
          var closureAnimation = (function(rent, rout) {
            return function() {
              // Time should be in milliseconds.  If time is above
              // 10 hours, then cap it as it will take so long.
              var cap = 5 * 60 * 60;
              var time = (rent.d < cap) ? rent.d * time_ratio * 1000 : cap * time_ratio * 1000;
              var d = mapExtender.lineDistance(rout.r);
              var marker = new L.CircleMarker(new L.LatLng(0, 0), circleOptions);

              map.addLayer(marker);
              animation = new Animator({
                duration: time,
                interval: anim_interval,
                transition: Animator.tx.linear,
                onComplete: function() {
                  // As stop will call this as well, check for state
                  if (this.state == 1) {
                    map.removeLayer(marker);
                  }
                }
              });
              function animateMarker(interval) {
                var dist = interval * d;
                var coord = mapExtender.coordAtDistance(rout.r, dist);
                marker.setLatLng(new L.LatLng(coord[1], coord[0]));
              }
              animation.addSubject(animateMarker);

              // Add some meta data
              animation.st = rent.s;
              animation.en = rent.e;
              animation.dur = rent.d;
              
              return animation;
            };
          })(rentals_loaded[i], routes_loaded[rentals_loaded[i].st + '-' + rentals_loaded[i].et]);
          
          animations[i] = closureAnimation();
        }
      }
      $('.loading').html('loaded rentals.  loaded routes.  loaded animations.');
      
      // Create day animation to fire off different events
      var day = new Animator({
        duration: time_anim * 1000,
        interval: anim_interval,
        transition: Animator.tx.linear,
        onComplete: function() {
          // Shown when stopped as well as complete.
        }
      });
      var handleDay = function(interval) {
        // Determine the time of day it is in the animation
        var totalSecElapsed = interval * time_day + day_offset;
        var timeObj = timeExtender.secondsToTime(totalSecElapsed);
        var currentTime = new XDate(anim_date.y, anim_date.m, anim_date.d, timeObj.h, timeObj.m, timeObj.s, 0, true);
        var a;

        // Show time
        $('.time_day').html(currentTime.getUTCHours() + ':' + currentTime.getUTCMinutes());
        
        // Find any animations that have not been played yet
        // that start before now and end after now.  Animation 
        // state is between 0 and 1.
        for (a in animations) {
          var animateStart = new XDate(animations[a].st, true);
          var animateEnd = new XDate(animations[a].en, true);
          if (animateStart <= currentTime && animateEnd > currentTime && animations[a].state === 0) {
            animations[a].play();
          }
        }
        
        // Update graph.  TODO: Pull in bike density dat and overlay it
        // as it goes.
        //var graph_data = da_flot.getData();
        //graph_data[1].data.push([])
        da_flot.setCrosshair({ x: currentTime.getTime() });
      };
      day.addSubject(handleDay);
      
      // Animation controls
      $('.play').click(function(e) {
        var a;
        e.preventDefault();
        day.seekTo(1);
        // Start all already started animations
        for (a in animations) {
          if (animations[a].state > 0 && animations[a].state < 1) {
            animations[a].seekTo(1);
          }
        }
      });
      $('.pause').click(function(e) {
        var a;
        e.preventDefault();
        day.stop();
        // Pause all animations.  Sometimes this misses??
        for (a in animations) {
          if (animations[a].state > 0 && animations[a].state < 1) {
            animations[a].stop();
          }
        }
      });
      
      
      // Graphing
      callback_density_average = function(graph) {
        var options = {
          xaxis: { mode: 'time' },
          crosshair: { mode: 'x', color: '#1B8ADA' }
        };
        var data = [{
          label: 'Average',
          data: graph,
          bars: { show: true },
          color: '#7F3E10'
        }];
        
        da_flot = $.plot($('#density-average-graph'), data, options);
        //da_flot.setCrosshair({ x: 1305711072000  });
        da_flot.setCrosshair();
        da_flot.lockCrosshair();
      };
      $.ajax({
        type: 'GET',
        dataType: 'jsonp',
        url: 'data/density_average.jsonp?callback=callback_density_average',
        jsonp: 'callback'
      });
    };
    
    // Call back for routes
    callback_routes = function(routes) {
      $('.loading').html('loaded rentals.  loaded routes.  loading animations.');
      routes_loaded = routes;
      loadAnimations();
    };
    
    // Call back for rentals
    callback_rentals = function(rentals) {
      $('.loading').html('loaded rentals.  loading routes..');
      rentals_loaded = rentals;
      $.ajax({
        type: 'GET',
        dataType: 'jsonp',
        url: 'data/routes.jsonp?callback=callback_routes',
        jsonp: 'callback'
      });
    };
    
    // Load data
    $.ajax({
      type: 'GET',
      dataType: 'jsonp',
      url: 'data/rentals.jsonp?callback=callback_rentals',
      jsonp: 'callback'
    });
  });
})(jQuery, window);