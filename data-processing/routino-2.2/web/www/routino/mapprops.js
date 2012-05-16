////////////////////////////////////////////////////////////////////////////////
/////////////////////////// Routino map properties /////////////////////////////
////////////////////////////////////////////////////////////////////////////////

var mapprops={ // contains all properties for the map to be displayed.

 // Default configuration:
 // UK coordinate range
 // West -11.0, South 49.5, East 2.0, North 61.0
 // Zoom level 4 to 15

 // EDIT THIS below to change the visible map limits

    westedge:  -11.0,          // Minimum longitude (degrees)
    eastedge:    2.0,          // Maximum longitude (degrees)

    southedge:  49.5,          // Minimum latitude (degrees)
    northedge:  61.0,          // Maximum latitude (degrees)

    zoomout:       4,          // Minimum zoom
    zoomin:       15,          // Maximum zoom

 // EDIT THIS above to change the visible map limits

 // EDIT THIS below to change the map URL(s) 

    mapdata: [
        {
            label:    "OSM map",
            baseurl:  "http://tile.openstreetmap.org/",
            errorurl: "http://openstreetmap.org/openlayers/img/404.png"
        }
    ]

 // EDIT THIS above to change the map URL(s) 

}; // end of map properties
