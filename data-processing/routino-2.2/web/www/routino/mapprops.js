////////////////////////////////////////////////////////////////////////////////
/////////////////////////// Routino map properties /////////////////////////////
////////////////////////////////////////////////////////////////////////////////

var mapprops={ // contains all properties for the map to be displayed.

 // Default configuration:
 // UK coordinate range
 // West -11.0, South 49.5, East 2.0, North 61.0
 // Zoom level 4 to 15

 // EDIT THIS below to change the visible map limits
 
 // EDITED For Twin Cities region

    westedge:   -94.482,          // Minimum longitude (degrees)
    eastedge:   -92.126,          // Maximum longitude (degrees)

    southedge:  44.266,          // Minimum latitude (degrees)
    northedge:  45.636,          // Maximum latitude (degrees)

    zoomout:      10,          // Minimum zoom
    zoomin:       16,          // Maximum zoom

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
