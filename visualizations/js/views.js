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
    'mapDefaultCenter': new L.LatLng(44.9745, -93.2513),
    'mapDefaultZoom': 13,
    'dataDir': 'data'
  },
  
  // Default objects for holding data.
  bikeAnimations: {},
  rentals: {},
  routes: {},
  densityAverage: {},
  timelineFlot: {},

  // Initialize function when View is first initialized.
  initialize: function() {
    // Mark a loading
    this.isLoading();
    // Create and intialize map
    this.createMap();
    
    // Load data.  All of it is asynchronous.
    this.loadDataRentals();
    this.loadDataRoutes();
    this.loadDataDensityAverage();
  },

  // Render function.
  render: function() {
    
  },
  
  // Mark as loading
  isLoading: function() {
    $('.loading').html('Loading...');
    
    return this;
  },
  
  // Done loading
  doneLoading: function() {
    $('.loading').html('loaded.');
    
    return this;
  },
  
  // Create map.
  createMap: function() {
    this.map = new L.Map(this.options.mapID, this.options.mapOptions);
    this.addMapLayers();
    this.map.setView(this.options.mapDefaultCenter, this.options.mapDefaultZoom);
    
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
  
  // Called when loading all data.  We are not actually sure
  // what is done yet, so check adn then fire if all is loaded.
  dataLoaded: function() {
    if (!_.isEmpty(this.rentals) && !_.isEmpty(this.routes) && !_.isEmpty(this.densityAverage)) {
      
      this.doneLoading();
    }
    
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