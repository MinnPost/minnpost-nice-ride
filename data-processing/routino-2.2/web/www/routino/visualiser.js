//
// Routino data visualiser web page Javascript
//
// Part of the Routino routing software.
//
// This file Copyright 2008-2011 Andrew M. Bishop
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//


//
// Data types
//

var data_types=[
                "junctions",
                "super",
                "oneway",
                "turns",
                "speed",
                "weight",
                "height",
                "width",
                "length"
               ];


//
// Junction styles
//

var junction_colours={
                      0: "#FFFFFF",
                      1: "#FF0000",
                      2: "#FFFF00",
                      3: "#00FF00",
                      4: "#8B4513",
                      5: "#00BFFF",
                      6: "#FF69B4",
                      7: "#000000",
                      8: "#000000",
                      9: "#000000"
                     };

var junction_styles={};


//
// Super styles
//

var super_node_style,super_segment_style;


//
// Oneway and turn restriction styles
//

var hex={0: "00", 1: "11",  2: "22",  3: "33",  4: "44",  5: "55",  6: "66",  7: "77",
         8: "88", 9: "99", 10: "AA", 11: "BB", 12: "CC", 13: "DD", 14: "EE", 15: "FF"};

var turn_restriction_style;


////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Map handling /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

var map;
var layerMap=[], layerVectors, layerBoxes;
var epsg4326, epsg900913;
var map_args;

var box;

// 
// Initialise the 'map' object
//

function map_init(lat,lon,zoom)
{
 // Map properties (North/South and East/West limits and zoom in/out limits) are now in mapprops.js
 // Map URLs are now in mapprops.js

 //
 // Create the map
 //

 epsg4326=new OpenLayers.Projection("EPSG:4326");
 epsg900913=new OpenLayers.Projection("EPSG:900913");

 map = new OpenLayers.Map ("map",
                           {
                            controls:[
                                      new OpenLayers.Control.Navigation(),
                                      new OpenLayers.Control.PanZoomBar(),
                                      new OpenLayers.Control.ScaleLine(),
                                      new OpenLayers.Control.LayerSwitcher()
                                      ],

                            projection: epsg900913,
                            displayProjection: epsg4326,

                            minZoomLevel: mapprops.zoomout,
                            numZoomLevels: mapprops.zoomin-mapprops.zoomout+1,
                            maxResolution: 156543.0339 / Math.pow(2,mapprops.zoomout),

                            maxExtent:        new OpenLayers.Bounds(-20037508.34, -20037508.34, 20037508.34, 20037508.34),
                            restrictedExtent: new OpenLayers.Bounds(mapprops.westedge,mapprops.southedge,mapprops.eastedge,mapprops.northedge).transform(epsg4326,epsg900913),

                            units: "m"
                           });

 map.events.register("moveend", map, mapMoved);

 // Add map tile layers

 for(var l=0;l < mapprops.mapdata.length;l++)
   {
    layerMap[l] = new OpenLayers.Layer.TMS(mapprops.mapdata[l].label,
                                           mapprops.mapdata[l].baseurl,
                                           {
                                            emptyUrl: mapprops.mapdata[l].errorurl,
                                            type: 'png',
                                            getURL: limitedUrl,
                                            displayOutsideMaxExtent: true,
                                            buffer: 1
                                           });
    map.addLayer(layerMap[l]);
   }

 // Get a URL for the tile; limited to map restricted extent.

 function limitedUrl(bounds)
 {
  var z = map.getZoom() + map.minZoomLevel;

  if (z>=7 && (bounds.right  < map.restrictedExtent.left ||
               bounds.left   > map.restrictedExtent.right ||
               bounds.top    < map.restrictedExtent.bottom ||
               bounds.bottom > map.restrictedExtent.top))
     return this.emptyUrl;

  var res = map.getResolution();
  var y = Math.round((this.maxExtent.top - bounds.top) / (res * this.tileSize.h));
  var limit = Math.pow(2, z);

  if (y < 0 || y >= limit)
    return this.emptyUrl;

  var x = Math.round((bounds.left - this.maxExtent.left) / (res * this.tileSize.w));

  x = ((x % limit) + limit) % limit;
  return this.url + z + "/" + x + "/" + y + "." + this.type;
 }

 // Add a vectors layer
 
 layerVectors = new OpenLayers.Layer.Vector("Markers");
 map.addLayer(layerVectors);

 for(var colour in junction_colours)
    junction_styles[colour]=new OpenLayers.Style({},{stroke: false, pointRadius: 2,fillColor: junction_colours[colour]});

 super_node_style   =new OpenLayers.Style({},{stroke: false, pointRadius: 3,fillColor  : "#FF0000"});
 super_segment_style=new OpenLayers.Style({},{fill: false  , strokeWidth: 2,strokeColor: "#FF0000"});

 turn_restriction_style=new OpenLayers.Style({},{fill: false, strokeWidth: 2,strokeColor: "#FF0000"});

 // Add a boxes layer

 layerBoxes = new OpenLayers.Layer.Boxes("Boundary");
 map.addLayer(layerBoxes);

 box=null;

 // Set the map centre to the limited range specified

 map.setCenter(map.restrictedExtent.getCenterLonLat(), map.getZoomForExtent(map.restrictedExtent,true));
 map.maxResolution = map.getResolution();

 // Move the map

 if(lon != 'lon' && lat != 'lat' && zoom != 'zoom')
   {
    if(lon<mapprops.westedge) lon=mapprops.westedge;
    if(lon>mapprops.eastedge) lon=mapprops.eastedge;

    if(lat<mapprops.southedge) lat=mapprops.southedge;
    if(lat>mapprops.northedge) lat=mapprops.northedge;

    if(zoom<mapprops.zoomout) zoom=mapprops.zoomout;
    if(zoom>mapprops.zoomin)  zoom=mapprops.zoomin;

    var lonlat = new OpenLayers.LonLat(lon,lat).transform(epsg4326,map.getProjectionObject());

    map.moveTo(lonlat,zoom-map.minZoomLevel);
   }
}


//
// Map has moved
//

function mapMoved()
{
 var centre = map.getCenter().clone();

 var lonlat = centre.transform(map.getProjectionObject(),epsg4326);

 var zoom = this.getZoom() + map.minZoomLevel;

 map_args="lat=" + lonlat.lat + ";lon=" + lonlat.lon + ";zoom=" + zoom;

 updateCustomURL();
}


//
// Update custom URL
//

function updateCustomURL()
{
 var router_url=document.getElementById("router_url");
 var link_url  =document.getElementById("link_url");
 var edit_url  =document.getElementById("edit_url");

 router_url.href="customrouter.cgi?" + map_args;
 link_url.href="customvisualiser.cgi?" + map_args;
 edit_url.href="http://www.openstreetmap.org/edit?" + map_args;
}


////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Server handling ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//
// Display the status
//

function displayStatus(type,subtype,content)
{
 var div_status=document.getElementById("result_status");

 var child=div_status.firstChild;

 do
   {
    if(child.id != undefined)
       child.style.display="none";

    child=child.nextSibling;
   }
 while(child != undefined);

 var chosen_status=document.getElementById("result_status_" + type);

 chosen_status.style.display="";

 if(subtype != null)
   {
    var format_status=document.getElementById("result_status_" + subtype).innerHTML;

    chosen_status.innerHTML=format_status.replace('#',String(content));
   }
}


//
// Display data statistics
//

function displayStatistics()
{
 // Use AJAX to get the statistics

 OpenLayers.loadURL("statistics.cgi",null,null,runStatisticsSuccess);
}


//
// Success in running data statistics generation.
//

function runStatisticsSuccess(response)
{
 var statistics_data=document.getElementById("statistics_data");
 var statistics_link=document.getElementById("statistics_link");

 statistics_data.innerHTML="<pre>" + response.responseText + "</pre>";

 statistics_link.style.display="none";
}


//
// Get the requested data
//

function displayData(datatype)
{
 for(var data in data_types)
    hideshow_hide(data_types[data]);

 if(datatype != "")
    hideshow_show(datatype);

 // Delete the old data

 layerVectors.destroyFeatures();

 if(box != null)
    layerBoxes.removeMarker(box);
 box=null;

 // Print the status

 displayStatus("no_data");

 // Return if just here to clear the data

 if(datatype == "")
    return;

 // Get the new data

 var mapbounds=map.getExtent().clone();
 mapbounds.transform(epsg900913,epsg4326);

 var url="visualiser.cgi";

 url=url + "?lonmin=" + mapbounds.left;
 url=url + ";latmin=" + mapbounds.bottom;
 url=url + ";lonmax=" + mapbounds.right;
 url=url + ";latmax=" + mapbounds.top;
 url=url + ";data=" + datatype;

 // Use AJAX to get the data

 switch(datatype)
   {
   case 'junctions':
    OpenLayers.loadURL(url,null,null,runJunctionsSuccess,runFailure);
    break;
   case 'super':
    OpenLayers.loadURL(url,null,null,runSuperSuccess,runFailure);
    break;
   case 'oneway':
    OpenLayers.loadURL(url,null,null,runOnewaySuccess,runFailure);
    break;
   case 'turns':
    OpenLayers.loadURL(url,null,null,runTurnsSuccess,runFailure);
    break;
   case 'speed':
   case 'weight':
   case 'height':
   case 'width':
   case 'length':
    OpenLayers.loadURL(url,null,null,runLimitSuccess,runFailure);
    break;
   }
}


//
// Success in getting the junctions.
//

function runJunctionsSuccess(response)
{
 var lines=response.responseText.split('\n');

 var features=[];

 for(var line=0;line<lines.length;line++)
   {
    var words=lines[line].split(' ');

    if(line == 0)
      {
       var lat1=words[0];
       var lon1=words[1];
       var lat2=words[2];
       var lon2=words[3];

       var bounds = new OpenLayers.Bounds(lon1,lat1,lon2,lat2).transform(epsg4326,map.getProjectionObject());

       box = new OpenLayers.Marker.Box(bounds);

       layerBoxes.addMarker(box);
      }
    else if(words[0] != "")
      {
       var lat=words[0];
       var lon=words[1];
       var count=words[2];

       var lonlat= new OpenLayers.LonLat(lon,lat).transform(epsg4326,epsg900913);

       var point = new OpenLayers.Geometry.Point(lonlat.lon,lonlat.lat);

       features.push(new OpenLayers.Feature.Vector(point,{},junction_styles[count]));
      }
   }

 layerVectors.addFeatures(features);

 displayStatus("data","junctions",lines.length-2);
}


//
// Success in getting the super-node and super-segments
//

function runSuperSuccess(response)
{
 var lines=response.responseText.split('\n');

 var features=[];

 var nodepoint;

 for(var line=0;line<lines.length;line++)
   {
    var words=lines[line].split(' ');

    if(line == 0)
      {
       var lat1=words[0];
       var lon1=words[1];
       var lat2=words[2];
       var lon2=words[3];

       var bounds = new OpenLayers.Bounds(lon1,lat1,lon2,lat2).transform(epsg4326,map.getProjectionObject());

       box = new OpenLayers.Marker.Box(bounds);

       layerBoxes.addMarker(box);
      }
    else if(words[0] != "")
      {
       var lat=words[0];
       var lon=words[1];
       var type=words[2];

       var lonlat= new OpenLayers.LonLat(lon,lat).transform(epsg4326,epsg900913);

       var point = new OpenLayers.Geometry.Point(lonlat.lon,lonlat.lat);

       if(type == "n")
         {
          nodepoint=point;

          features.push(new OpenLayers.Feature.Vector(point,{},super_node_style));
         }
       else
         {
          var segment = new OpenLayers.Geometry.LineString([nodepoint,point]);

          features.push(new OpenLayers.Feature.Vector(segment,{},super_segment_style));
         }
      }
   }

 layerVectors.addFeatures(features);

 displayStatus("data","super",lines.length-2);
}


//
// Success in getting the oneway data
//

function runOnewaySuccess(response)
{
 var lines=response.responseText.split('\n');

 var features=[];

 for(var line=0;line<lines.length;line++)
   {
    var words=lines[line].split(' ');

    if(line == 0)
      {
       var lat1=words[0];
       var lon1=words[1];
       var lat2=words[2];
       var lon2=words[3];

       var bounds = new OpenLayers.Bounds(lon1,lat1,lon2,lat2).transform(epsg4326,map.getProjectionObject());

       box = new OpenLayers.Marker.Box(bounds);

       layerBoxes.addMarker(box);
      }
    else if(words[0] != "")
      {
       var lat1=words[0];
       var lon1=words[1];
       var lat2=words[2];
       var lon2=words[3];

       var lonlat1= new OpenLayers.LonLat(lon1,lat1).transform(epsg4326,epsg900913);
       var lonlat2= new OpenLayers.LonLat(lon2,lat2).transform(epsg4326,epsg900913);

     //var point1 = new OpenLayers.Geometry.Point(lonlat1.lon,lonlat1.lat);
       var point2 = new OpenLayers.Geometry.Point(lonlat2.lon,lonlat2.lat);

       var dlat = lonlat2.lat-lonlat1.lat;
       var dlon = lonlat2.lon-lonlat1.lon;
       var dist = Math.sqrt(dlat*dlat+dlon*dlon)/10;
       var ang  = Math.atan2(dlat,dlon);

       var point3 = new OpenLayers.Geometry.Point(lonlat1.lon+dlat/dist,lonlat1.lat-dlon/dist);
       var point4 = new OpenLayers.Geometry.Point(lonlat1.lon-dlat/dist,lonlat1.lat+dlon/dist);

       var segment = new OpenLayers.Geometry.LineString([point2,point3,point4,point2]);

       var r=Math.round(7.5+7.9*Math.cos(ang));
       var g=Math.round(7.5+7.9*Math.cos(ang+2.0943951));
       var b=Math.round(7.5+7.9*Math.cos(ang-2.0943951));
       var colour = "#" + hex[r] + hex[g] + hex[b];

       var style=new OpenLayers.Style({},{strokeWidth: 2,strokeColor: colour});

       features.push(new OpenLayers.Feature.Vector(segment,{},style));
      }
   }

 layerVectors.addFeatures(features);

 displayStatus("data","oneway",lines.length-2);
}


//
// Success in getting the turn restrictions data
//

function runTurnsSuccess(response)
{
 var lines=response.responseText.split('\n');

 var features=[];

 for(var line=0;line<lines.length;line++)
   {
    var words=lines[line].split(' ');

    if(line == 0)
      {
       var lat1=words[0];
       var lon1=words[1];
       var lat2=words[2];
       var lon2=words[3];

       var bounds = new OpenLayers.Bounds(lon1,lat1,lon2,lat2).transform(epsg4326,map.getProjectionObject());

       box = new OpenLayers.Marker.Box(bounds);

       layerBoxes.addMarker(box);
      }
    else if(words[0] != "")
      {
       var lat1=words[0];
       var lon1=words[1];
       var lat2=words[2];
       var lon2=words[3];
       var lat3=words[4];
       var lon3=words[5];

       var lonlat1= new OpenLayers.LonLat(lon1,lat1).transform(epsg4326,epsg900913);
       var lonlat2= new OpenLayers.LonLat(lon2,lat2).transform(epsg4326,epsg900913);
       var lonlat3= new OpenLayers.LonLat(lon3,lat3).transform(epsg4326,epsg900913);

       var point1 = new OpenLayers.Geometry.Point(lonlat1.lon,lonlat1.lat);
       var point2 = new OpenLayers.Geometry.Point(lonlat2.lon,lonlat2.lat);
       var point3 = new OpenLayers.Geometry.Point(lonlat3.lon,lonlat3.lat);

       var segments = new OpenLayers.Geometry.LineString([point1,point2,point3]);

       features.push(new OpenLayers.Feature.Vector(segments,{},turn_restriction_style));
      }
   }

 layerVectors.addFeatures(features);

 displayStatus("data","turns",lines.length-2);
}


//
// Success in getting the speed/weight/height/width/length limits
//

function runLimitSuccess(response)
{
 var lines=response.responseText.split('\n');

 var features=[];

 var nodelonlat;

 for(var line=0;line<lines.length;line++)
   {
    var words=lines[line].split(' ');

    if(line == 0)
      {
       var lat1=words[0];
       var lon1=words[1];
       var lat2=words[2];
       var lon2=words[3];

       var bounds = new OpenLayers.Bounds(lon1,lat1,lon2,lat2).transform(epsg4326,map.getProjectionObject());

       box = new OpenLayers.Marker.Box(bounds);

       layerBoxes.addMarker(box);
      }
    else if(words[0] != "")
      {
       var lat=words[0];
       var lon=words[1];
       var number=words[2];

       var lonlat= new OpenLayers.LonLat(lon,lat).transform(epsg4326,epsg900913);

       if(number == undefined)
         {
          var point = new OpenLayers.Geometry.Point(lonlat.lon,lonlat.lat);

          nodelonlat=lonlat;

          features.push(new OpenLayers.Feature.Vector(point,{},junction_styles[1]));
         }
       else
         {
          var dlat = lonlat.lat-nodelonlat.lat;
          var dlon = lonlat.lon-nodelonlat.lon;
          var dist = Math.sqrt(dlat*dlat+dlon*dlon)/60;

          var point = new OpenLayers.Geometry.Point(nodelonlat.lon+dlon/dist,nodelonlat.lat+dlat/dist);

          features.push(new OpenLayers.Feature.Vector(point,{},
                                                      new OpenLayers.Style({},{externalGraphic: 'icons/limit-' + number + '.png',
                                                                               graphicYOffset: -9,
                                                                               graphicWidth: 19,
                                                                               graphicHeight: 19})));
         }
      }
   }

 layerVectors.addFeatures(features);

 displayStatus("data","limit",lines.length-2);
}


//
// Failure in getting data.
//

function runFailure(response)
{
 displayStatus("error");
}
