/**
 * Views for applicaiton.  A single View to handle most of
 * the applicaiton logic as the model side is light.
 */
(function($, window, undefined) {

// Check our namespace
window.MinnPost = window.MinnPost || {};

var BikeApplication = window.MinnPost.BikeApplication = Backbone.View.extend({
  
  // Various options, easy to set and change.
  options: {
    'mapOptions': {
      'minZoom': 8,
      'maxZoom': 15
    },
    'mapID': 'map',
    'mapDefaultCenter': new L.LatLng(44.96552667056723, -93.25298309326172),
    'mapDefaultZoom': 12,
    'dataDir': 'data',
    'loadingSelector': '.loading',
    'loadingMessageSelector': '.loading .loading-message',
    'controlsContainerSelector': '.leaflet-top.leaflet-left',
    'flotOptions': {
      xaxis: { mode: 'time', timeformat: '%h:%M %p' },
      yaxis: { ticks: 2 },
      crosshair: { mode: 'x', color: '#1B8ADA' },
      legend: { backgroundColor: 'inherit', border: null },
      grid: { borderWidth: 1, borderColor: '#BBBBBB' }
    },
    'flotAverageData': {
      label: 'Avg bike density',
      data: [],
      color: '#7F3E10',
      lines: { show: true, fill: true }
    },
    'flotSelector': '#density-average-graph',
    'flotContainerSelector': '.leaflet-bottom.leaflet-left',
    'timeContainerSelector': '.leaflet-bottom.leaflet-right',
    'timeAnimDaySecs': 24 * 60 * 60,
    'timeAnimLengthSecs': 2 * 60,
    'timeAnimInterval': 100,
    'timeAnimDate': {
      'y': 2011,
      'm': 4, // Month is zero based
      'd': 18
    },
    'timeAnimTimeOffsetSecs': ((4 * 60) + 30) * 60 // 4:30
  },
  
  // Default objects for holding data.
  bikeAnimations: {},
  rentals: {},
  routes: {},
  densityAverage: {},
  timelineFlot: {},
  bikeAnimations: new MinnPost.BikeAnimations(),
  
  // Events
  events: {
    'click .control-play': 'play',
    'click .control-pause': 'pause'
  },

  // Initialize function when View is first initialized.
  initialize: function() {
    // Mark a loading; create map; load data;
    this.isLoading()
      .createMap()
      .loadDataRentals()
      .loadDataRoutes()
      .loadDataDensityAverage();
  },

  // Render function.
  render: function() {
    
  },
  
  // Called when loading all data.  We are not actually sure
  // what is done yet, so check and then fire if all is loaded.
  dataLoaded: function() {
    if (!_.isEmpty(this.rentals) && !_.isEmpty(this.routes) && !_.isEmpty(this.densityAverage)) {
      this.drawGraph()
        .makeBikeAnimations()
        .makeAnimator()
        .addControls()
        .doneLoading();
    }
    
    return this;
  },

  // Play main animation
  play: function(e) {
    e.preventDefault();
    
    // Change button to pause
    $('.control-play-pause').addClass('control-pause').removeClass('control-play').attr('title', 'Pause');
  
    // Ensure that the animation is loaded
    if (this.animation !== undefined && !_.isEmpty(this.animation)) {
      this.animation.seekTo(1);
      // Start all already started animations
      this.bikeAnimations.each(function(anim) {
        if (anim.animation.state > 0 && anim.animation.state < 1) {
          anim.animation.seekTo(1);
        }
      });
    }
    
    return this;
  },

  // Pause animation
  pause: function(e) {
    e.preventDefault();
  
    // Change button to play
    $('.control-play-pause').removeClass('control-pause').addClass('control-play').attr('title', 'Play');
    
    // Ensure that the animation is loaded
    if (this.animation !== undefined && !_.isEmpty(this.animation)) {
      this.animation.stop();
      // Pause all animations.  Do all as some are missed in the mixed
      this.bikeAnimations.each(function(anim) {
        anim.animation.stop();
      });
    }
    
    return this;
  },

  // Pause animation
  resetAnimations: function() {
    // Change button to play
    $('.control-play-pause').removeClass('control-pause').addClass('control-play').attr('title', 'Play');
    
    // Stop and change all states to 0.
    if (this.animation !== undefined && !_.isEmpty(this.animation)) {
      this.animation.stop();
      this.animation.state = 0;
      this.bikeAnimations.each(function(anim) {
        anim.resetAnimation();
      });
    }
    
    return this;
  },
  
  // Controls
  addControls: function() {
    // Play pause buttons
    var template = _.template($("#template-controls").html(), { });
    $(this.options.controlsContainerSelector).append(template);
    
    // Time
    template = _.template($("#template-time").html(), { });
    $(this.options.timeContainerSelector).append(template);
    
    return this;
  },
  
  // Mark as loading
  isLoading: function() {
    $(this.options.loadingSelector).addClass('is-loading');
    $(this.options.loadingMessageSelector).html('Loading data and animations...');
    
    return this;
  },
  
  // Done loading
  doneLoading: function() {
    $(this.options.loadingMessageSelector).html('Loaded.');
    $(this.options.loadingSelector).fadeOut('slow').removeClass('is-loading');
    
    return this;
  },
  
  // Create map.
  createMap: function() {
    this.map = new L.Map(this.options.mapID, this.options.mapOptions);
    this.addMapLayers();
    this.map.setView(this.options.mapDefaultCenter, this.options.mapDefaultZoom);
    this.map.addControl(new L.Control.Fullscreen());
    
    // Move attributioin to footnotes
    this.map.attributionControl.setPrefix('');
    $('.footnote').html($('.footnote').html() + ' ' + this.map.attributionControl._container.innerHTML);
    this.map.removeControl(this.map.attributionControl);
    
    return this;
  },
  
  // Add layers to map.
  addMapLayers: function() {
    // Minnnpost base map
    var minnpost = new L.TileLayer('http://{s}.tiles.minnpost.com/minnpost-basemaps/minnpost-minnesota-greyscale-no-labels/{z}/{x}/{y}.png', 
     { attribution: 'Map imagery from <a target="_blank" href="http://minnpost.com">MinnPost</a>; Map data from <a target="_blank" href="http://openstreetmap.org">OpenStreetMap</a>.',
      scheme: 'tms' });
    this.map.addLayer(minnpost);
    
    // Route layer
    var routes = new L.TileLayer('http://{s}.tiles.minnpost.com/minnpost-nice-ride/minnpost-nice-ride-routes/{z}/{x}/{y}.png', 
      { attribution: 'Data provided by <a target="_blank" href="https://www.niceridemn.org/">Nice Ride MN</a>; ',
      scheme: 'tms' });
    this.map.addLayer(routes);
    
    // Labels
    var minnpostLabels = new L.TileLayer('http://{s}.tiles.minnpost.com/minnpost-basemaps/minnpost-minnesota-greyscale-labels/{z}/{x}/{y}.png', 
     { scheme: 'tms' });
    this.map.addLayer(minnpostLabels);
    
    return this;
  },
  
  // Create flot graph
  drawGraph: function() {
    // We want to insert this in the leaflet map container
    var template = _.template($("#template-flot-timeline").html(), { });
    $(this.options.flotContainerSelector).append(template);
  
    this.options.flotAverageData.data = this.densityAverage;
    this.timelineFlot = $.plot($(this.options.flotSelector), [this.options.flotAverageData], this.options.flotOptions);
    this.timelineFlot.setCrosshair();
    this.timelineFlot.lockCrosshair();
    
    return this;
  },
  
  // Make bike animations
  makeBikeAnimations: function() {
    var i;
    var ba;
    var route;
    var ratio = (this.options.timeAnimLengthSecs / this.options.timeAnimDaySecs);
    
    // Go through rentals, match route, make a bike animation then add to collection
    for (i in this.rentals) {
      // Check for the route
      if (this.routes[this.rentals[i].st + '-' + this.rentals[i].et] !== undefined) {
        route = this.routes[this.rentals[i].st + '-' + this.rentals[i].et];
        
        // Add some data
        this.rentals[i].timeRatio = ratio;
        this.rentals[i].animInterval = this.options.timeAnimInterval;
        this.rentals[i].map = this.map;
        
        // Create model and add to collection.  Model initializes stuff.
        var ba = new MinnPost.BikeAnimation(this.rentals[i], route);
        this.bikeAnimations.add(ba, { silent: true });
      }
    }
    
    return this;
  },
  
  // Makes fulls animator
  makeAnimator: function() {
    var thisView = this;
  
    this.animation = new Animator({
      duration: this.options.timeAnimLengthSecs * 1000,
      interval: this.options.timeAnimInterval,
      transition: Animator.tx.linear,
      onComplete: function() {
        // As stop will call this as well, check for state
        if (this.state == 1) {
          thisView.resetAnimations();
        }
      }
    });
    
    this.animation.addSubject(function(interval) {
      // Determine the time of day it is in the animation
      var totalSecElapsed = interval * thisView.options.timeAnimDaySecs + thisView.options.timeAnimTimeOffsetSecs;
      var aD = thisView.options.timeAnimDate;
      var timeObj = timeExtender.secondsToTime(totalSecElapsed);
      var currentTime = new XDate(aD.y, aD.m, aD.d, timeObj.h, timeObj.m, timeObj.s, 0, true);

      // Show time
      thisView.showTime(currentTime);
      
      // Find any animations that have not been played yet
      // that start before now and end after now.  Animation 
      // state is between 0 and 1.
      thisView.bikeAnimations.each(function(anim) {
        if (anim.get('s') <= currentTime && anim.get('e') > currentTime && anim.animation.state === 0) {
          anim.animation.play();
        }
      });
      
      // Update graph.  TODO: Pull in bike density dat and overlay it
      // as it goes.
      //var graph_data = da_flot.getData();
      //graph_data[1].data.push([])
      thisView.timelineFlot.setCrosshair({ x: currentTime.getTime() });
    });
    
    return this;
  },
  
  showTime: function(date) {
    $('.time-day').html(date.toUTCString('h:ss tt'));
    
    return this;
  },
  
  // Load rental data.
  loadDataRentals: function() {
    var thisView = this;
    window.callback_rentals = function(rentals) {
      thisView.rentals = rentals;
      thisView.dataLoaded();
    }
    $.ajax({
      type: 'GET',
      dataType: 'jsonp',
      url: thisView.options.dataDir + '/rentals.jsonp?callback=callback_rentals',
      jsonp: 'callback'
    });
    
    return this;
  },
  
  // Load routes data.
  loadDataRoutes: function() {
    var thisView = this;
    window.callback_routes = function(routes) {
      thisView.routes = routes;
      thisView.dataLoaded();
    }
    $.ajax({
      type: 'GET',
      dataType: 'jsonp',
      url: thisView.options.dataDir + '/routes.jsonp?callback=callback_routes',
      jsonp: 'callback'
    });
    
    return this;
  },
  
  // Load density average data.
  loadDataDensityAverage: function() {
    var thisView = this;
    window.callback_density_average = function(avgs) {
      thisView.densityAverage = avgs;
      thisView.dataLoaded();
    }
    $.ajax({
      type: 'GET',
      dataType: 'jsonp',
      url: thisView.options.dataDir + '/density_average.jsonp?callback=callback_density_average',
      jsonp: 'callback'
    });
    
    return this;
  }

});

})(jQuery, window);