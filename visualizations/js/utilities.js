/**
 * Utilities for use in the Nice Ride applicaiton.
 */

// Converts numeric degrees to radians
if (typeof Number.prototype.toRad === 'undefined') {
  Number.prototype.toRad = function() {
    return this * Math.PI / 180;
  };
}
// Converts radians to numeric (signed) degrees
if (typeof Number.prototype.toDeg === 'undefined') {
  Number.prototype.toDeg = function() {
    return this * 180 / Math.PI;
  };
}
    
/**
 * Namespace some things
 */
(function($, window, undefined) {

// Check our namespace
window.MinnPost = window.MinnPost || {};

// Object for dealing with map data
var mapExtender = window.mapExtender = window.mapExtender || {};
var timeExtender = window.timeExtender = window.timeExtender || {};

mapExtender.lineDistance = function(line) {
  // Get distance of geojson object
  var dist = 0;
  
  if (typeof line.type != 'undefined' && line.type == 'LineString') {
    for (var i = 1; i < line.coordinates.length; i++) {
      dist += mapExtender.coordDistance(line.coordinates[i], line.coordinates[i - 1]);
    }
    
    return dist;
  }
  else {
    return dist;
  }
};

mapExtender.coordDistance = function(coord1, coord2) {
  // Get Haversine distance between two Geojson coordinates
  var dLat = (coord2[1] - coord1[1]).toRad();
  var dLon = (coord2[0] - coord1[0]).toRad();
  var lat1 = coord1[1].toRad();
  var lat2 = coord2[1].toRad();
  
  var a = Math.sin(dLat/2) * Math.sin(dLat/2) + Math.sin(dLon/2) * Math.sin(dLon/2) * Math.cos(lat1) * Math.cos(lat2); 
  var c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a)); 
  return c;
};

mapExtender.coordAtDistance = function(line, dist) {
  // Gets coordinate at specific place along path
  var coord = [];
  var distCurrent = 0;
  var distOld = 0;
  
  // If 0, then just give the first coordinate back.
  if (dist === 0) {
    return line.coordinates[0];
  }

  // Got to each coordinate and check if we need to go to next;
  for (var i = 1; (i < line.coordinates.length && distCurrent < dist); i++) {
    distOld = distCurrent;
    distCurrent += mapExtender.coordDistance(line.coordinates[i], line.coordinates[i - 1]);
  }
  
  // Check if we did not make it
  if (distCurrent < dist) {
    return coord;
  }
  var p1 = line.coordinates[i - 2];
  var p2 = line.coordinates[i - 1];
  var part = (dist - distOld) / (distCurrent - distOld);

  // Lon
  coord[0] = p1[0] + (p2[0] - p1[0]) * part;
  coord[1] = p1[1] + (p2[1] - p1[1]) * part;
  return coord;
};

mapExtender.toKM = function(v) {
  // Returns KM from a lat/lon distance;
  return v * 6371;
};

mapExtender.toM = function(v) {
  // Returns M from a lat/lon distance;
  return v * 6371 * 1000;
};

// Object for dealing with time
timeExtender.secondsToTime = function(secs) {
  var hours = Math.floor(secs / (60 * 60));
  var divisorForMinutes = secs % (60 * 60);
  var minutes = Math.floor(divisorForMinutes / 60);
  var seconds = Math.ceil(divisorForMinutes % 60);
  
  return {
    'h': hours,
    'm': minutes,
    's': seconds
  };
};

})(jQuery, window);