<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<HTML>

<!--
 Routino router web page.

 Part of the Routino routing software.

 This file Copyright 2008-2011 Andrew M. Bishop

 Dutch translation by Jan Jansen (August 2010).

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see http://www.gnu.org/licenses/.
-->

<HEAD>
<TITLE>Routino : Route Planner for OpenStreetMap Data</TITLE>
<META name="keywords" content="openstreetmap routing route planner">
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">

<!-- OpenLayers Javascript library -->
<script src="../openlayers/OpenLayers.js" type="text/javascript"></script>

<!-- Page elements -->
<script src="page-elements.js" type="text/javascript"></script>
<link href="page-elements.css" type="text/css" rel="stylesheet">

<!-- Router and visualiser shared features -->
<link href="maplayout.css" type="text/css" rel="stylesheet">
<!--[if IE 6]>
  <link href="maplayout-ie6-bugfixes.css" type="text/css" rel="stylesheet">
<![endif]-->
<!--[if IE 7]>
  <link href="maplayout-ie7-bugfixes.css" type="text/css" rel="stylesheet">
<![endif]-->

<!-- Router specific features -->
<script src="profiles.js" type="text/javascript"></script>
<script src="mapprops.js" type="text/javascript"></script>
<script src="router.js" type="text/javascript"></script>
<link href="router.css" type="text/css" rel="stylesheet">

</HEAD>
<BODY onload="map_init('lat','lon','zoom');form_init();block_return_key();">

<!-- Left hand side of window - data panel -->

<div class="left_panel">

  <div class="tab_box">
    <span id="tab_options" onclick="tab_select('options');" class="tab_selected"   title="Set routing options">Opties</span>
    <span id="tab_results" onclick="tab_select('results');" class="tab_unselected" title="See routing results">Resultaten</span>
    <span id="tab_data"    onclick="tab_select('data');"    class="tab_unselected" title="View database information">Data</span>
  </div>

  <div class="tab_content" id="tab_options_div">

    <form name="form" id="form" action="router.cgi" method="get">
      <div class="hideshow_box">
        <span class="hideshow_title">Routino OpenStreetMap Router</span>
        Zoom naar straatniveau.
        Selecteer start- and eindpunten onder Coordinaten. (click op het marker
        icoon links, schuif het op map naar gewenste positie).
        <div align="center">
          <a target="other" href="http://www.routino.org/">Routino Website</a>
          |
          <a target="other" href="documentation/">Documentation</a>
        </div>
      </div>

      <div class="hideshow_box">
        <span id="hideshow_language_show" onclick="hideshow_show('language');" class="hideshow_show">+</span>
        <span id="hideshow_language_hide" onclick="hideshow_hide('language');" class="hideshow_hide">-</span>
        <span class="hideshow_title">Taal (Language)</span>

        <!-- Note for translations: Only this HTML file needs to be translated, the Javascript has
             no language specific information in it.  Only the body text and title attributes should
             be changed, the values passed to the JavaScript and the element names must not be changed.
             The selection below changes the language option passed to the router and selects the
             output language not the web page language, the links are for that.  The router itself uses
             the translations.xml file for the translated versions of the output. -->

        <div id="hideshow_language_div" style="display: none;">
          <table>
            <tr>
              <td><a href="router.html.nl" title="Nederlandse web pagina">Nederlands</a>
              <td>(NL)
              <td><input name="language" type="radio" value="nl" onchange="formSetLanguage('nl')" checked><!-- language -->
            <tr>
              <td><a href="router.html.en" title="Engelstalige web pagina">English</a>
              <td>(EN)
              <td><input name="language" type="radio" value="en" onchange="formSetLanguage('en')" ><!-- language -->
            <tr>
              <td>German
              <td>(DE)
              <td><input name="language" type="radio" value="de" onchange="formSetLanguage('de')"><!-- language -->
            <tr>
              <td>Russian
              <td>(RU)
              <td><input name="language" type="radio" value="ru" onchange="formSetLanguage('ru')"><!-- language -->
          </table>
        </div>
      </div>

      <div class="hideshow_box">
        <span id="hideshow_waypoint_show" onclick="hideshow_show('waypoint');" class="hideshow_hide">+</span>
        <span id="hideshow_waypoint_hide" onclick="hideshow_hide('waypoint');" class="hideshow_show">-</span>
        <span class="hideshow_title">Coordinaten (waypoints)</span>
        <div id="hideshow_waypoint_div">
          <table>
            <tr id="point1">
              <td>
                <img name="waypoint1" src="icons/marker-1-grey.png" title="Waypoint 1 Position - (click voor plaatsen/verwijderen op map)" alt="Waypoint 1" onmousedown="markerToggleMap(1)">&nbsp;
              <td>
                <input name="lon1" type="text" size=7 title="Waypoint 1 Longitude" onchange="formSetCoords(1);"><!-- lon1 --> E&nbsp;
              <td>
                <input name="lat1" type="text" size=7 title="Waypoint 1 Latitude"  onchange="formSetCoords(1);"><!-- lat1 --> N&nbsp;
              <td>
                <img alt="o" src="icons/waypoint-centre.png" title="Centreer dit punt op map"     onmousedown="markerCentre(1);"  >
                <img alt="^" src="icons/waypoint-up.png"     title="Beweeg dit punt naar boven"   onmousedown="markerMoveUp(1);"  >
                <img alt="+" src="icons/waypoint-add.png"    title="Voeg hierna punt toe"         onmousedown="markerAddAfter(1);"><br>
                <img alt="~" src="icons/waypoint-home.png"   title="Toggle als thuis locatie"     onmousedown="markerHome(1);"    >
                <img alt="v" src="icons/waypoint-down.png"   title="Beweeg dit punt naar beneden" onmousedown="markerMoveDown(1);">
                <img alt="-" src="icons/waypoint-remove.png" title="Verwijder dit punt"           onmousedown="markerRemove(1);"  >
            <tr id="point2">
              <td>
                <img name="waypoint2" src="icons/marker-2-grey.png" title="Waypoint 2 Position - (click voor plaatsen/verwijderen op map)" alt="Waypoint 2" onmousedown="markerToggleMap(2)">&nbsp;
              <td>
                <input name="lon2" type="text" size=7 title="Waypoint 2 Longitude" onchange="formSetCoords(2);"><!-- lon2 --> E&nbsp;
              <td>
                <input name="lat2" type="text" size=7 title="Waypoint 2 Latitude"  onchange="formSetCoords(2);"><!-- lat2 --> N&nbsp;
              <td>
                <img alt="o" src="icons/waypoint-centre.png" title="Centreer dit punt op map"     onmousedown="markerCentre(2);">
                <img alt="^" src="icons/waypoint-up.png"     title="Beweeg dit punt naar boven"   onmousedown="markerMoveUp(2);"  >
                <img alt="+" src="icons/waypoint-add.png"    title="Voeg hierna punt toe"         onmousedown="markerAddAfter(2);"><br>
                <img alt="~" src="icons/waypoint-home.png"   title="Toggle als thuis locatie"     onmousedown="markerHome(2);"    >
                <img alt="v" src="icons/waypoint-down.png"   title="Beweeg dit punt naar beneden" onmousedown="markerMoveDown(2);">
                <img alt="-" src="icons/waypoint-remove.png" title="Verwijder dit punt"           onmousedown="markerRemove(2);"  >
            <tr id="point3">
              <td>
                <img name="waypoint3" src="icons/marker-3-grey.png" title="Waypoint 3 Position - (click voor plaatsen/verwijderen on map)" alt="Waypoint 3" onmousedown="markerToggleMap(3)">&nbsp;
              <td>
                <input name="lon3" type="text" size=7 title="Waypoint 3 Longitude" onchange="formSetCoords(3);"><!-- lon3 --> E&nbsp;
              <td>
                <input name="lat3" type="text" size=7 title="Waypoint 3 Latitude"  onchange="formSetCoords(3);"><!-- lat3 --> N&nbsp;
              <td>
                <img alt="o" src="icons/waypoint-centre.png" title="Centreer dit punt op map"     onmousedown="markerCentre(3);">
                <img alt="^" src="icons/waypoint-up.png"     title="Beweeg dit punt naar boven"   onmousedown="markerMoveUp(3);"  >
                <img alt="+" src="icons/waypoint-add.png"    title="Voeg hierna punt toe"         onmousedown="markerAddAfter(3);"><br>
                <img alt="~" src="icons/waypoint-home.png"   title="Toggle als thuis locatie"     onmousedown="markerHome(3);"    >
                <img alt="v" src="icons/waypoint-down.png"   title="Beweeg dit punt naar beneden" onmousedown="markerMoveDown(3);">
                <img alt="-" src="icons/waypoint-remove.png" title="Verwijder dit punt"           onmousedown="markerRemove(3);"  >
            <tr id="point4">
              <td>
                <img name="waypoint4" src="icons/marker-4-grey.png" title="Waypoint 4 Position - (click voor plaatsen/verwijderen op map)" alt="Waypoint 4" onmousedown="markerToggleMap(4)">&nbsp;
              <td>
                <input name="lon4" type="text" size=7 title="Waypoint 4 Longitude" onchange="formSetCoords(4);"><!-- lon4 --> E&nbsp;
              <td>
                <input name="lat4" type="text" size=7 title="Waypoint 4 Latitude"  onchange="formSetCoords(4);"><!-- lat4 --> N&nbsp;
              <td>
                <img alt="o" src="icons/waypoint-centre.png" title="Centreer dit punt op map"     onmousedown="markerCentre(4);">
                <img alt="^" src="icons/waypoint-up.png"     title="Beweeg dit punt naar boven"   onmousedown="markerMoveUp(4);"  >
                <img alt="+" src="icons/waypoint-add.png"    title="Voeg hierna punt toe"         onmousedown="markerAddAfter(4);"><br>
                <img alt="~" src="icons/waypoint-home.png"   title="Toggle als thuis locatie"     onmousedown="markerHome(4);"    >
                <img alt="v" src="icons/waypoint-down.png"   title="Beweeg dit punt naar beneden" onmousedown="markerMoveDown(4);">
                <img alt="-" src="icons/waypoint-remove.png" title="Verwijder dit punt"           onmousedown="markerRemove(4);"  >
            <tr id="point5">
              <td>
                <img name="waypoint5" src="icons/marker-5-grey.png" title="Waypoint 5 Position - (click voor plaatsen/verwijderen op map)" alt="Waypoint 5" onmousedown="markerToggleMap(5)">&nbsp;
              <td>
                <input name="lon5" type="text" size=7 title="Waypoint 5 Longitude" onchange="formSetCoords(5);"><!-- lon5 --> E&nbsp;
              <td>
                <input name="lat5" type="text" size=7 title="Waypoint 5 Latitude"  onchange="formSetCoords(5);"><!-- lat5 --> N&nbsp;
              <td>
                <img alt="o" src="icons/waypoint-centre.png" title="Centreer dit punt op map"     onmousedown="markerCentre(5);">
                <img alt="^" src="icons/waypoint-up.png"     title="Beweeg dit punt naar boven"   onmousedown="markerMoveUp(5);"  >
                <img alt="+" src="icons/waypoint-add.png"    title="Voeg hierna punt toe"         onmousedown="markerAddAfter(5);"><br>
                <img alt="~" src="icons/waypoint-home.png"   title="Toggle als thuis locatie"     onmousedown="markerHome(5);"    >
                <img alt="v" src="icons/waypoint-down.png"   title="Beweeg dit punt naar beneden" onmousedown="markerMoveDown(5);">
                <img alt="-" src="icons/waypoint-remove.png" title="Verwijder dit punt"           onmousedown="markerRemove(5);"  >
            <tr id="point6">
              <td>
                <img name="waypoint6" src="icons/marker-6-grey.png" title="Waypoint 6 Position - (click voor plaatsen/verwijderen op map)" alt="Waypoint 6" onmousedown="markerToggleMap(6)">&nbsp;
              <td>
                <input name="lon6" type="text" size=7 title="Waypoint 6 Longitude" onchange="formSetCoords(6);"><!-- lon6 --> E&nbsp;
              <td>
                <input name="lat6" type="text" size=7 title="Waypoint 6 Latitude"  onchange="formSetCoords(6);"><!-- lat6 --> N&nbsp;
              <td>
                <img alt="o" src="icons/waypoint-centre.png" title="Centreer dit punt op map"     onmousedown="markerCentre(6);">
                <img alt="^" src="icons/waypoint-up.png"     title="Beweeg dit punt naar boven"   onmousedown="markerMoveUp(6);"  >
                <img alt="+" src="icons/waypoint-add.png"    title="Voeg hierna punt toe"         onmousedown="markerAddAfter(6);"><br>
                <img alt="~" src="icons/waypoint-home.png"   title="Toggle als thuis locatie"     onmousedown="markerHome(6);"    >
                <img alt="v" src="icons/waypoint-down.png"   title="Beweeg dit punt naar beneden" onmousedown="markerMoveDown(6);">
                <img alt="-" src="icons/waypoint-remove.png" title="Verwijder dit punt"           onmousedown="markerRemove(6);"  >
            <tr id="point7">
              <td>
                <img name="waypoint7" src="icons/marker-7-grey.png" title="Waypoint 7 Position - (click voor plaatsen/verwijderen op map)" alt="Waypoint 7" onmousedown="markerToggleMap(7)">&nbsp;
              <td>
                <input name="lon7" type="text" size=7 title="Waypoint 7 Longitude" onchange="formSetCoords(7);"><!-- lon7 --> E&nbsp;
              <td>
                <input name="lat7" type="text" size=7 title="Waypoint 7 Latitude"  onchange="formSetCoords(7);"><!-- lat7 --> N&nbsp;
              <td>
                <img alt="o" src="icons/waypoint-centre.png" title="Centreer dit punt op map"     onmousedown="markerCentre(7);">
                <img alt="^" src="icons/waypoint-up.png"     title="Beweeg dit punt naar boven"   onmousedown="markerMoveUp(7);"  >
                <img alt="+" src="icons/waypoint-add.png"    title="Voeg hierna punt toe"         onmousedown="markerAddAfter(7);"><br>
                <img alt="~" src="icons/waypoint-home.png"   title="Toggle als thuis locatie"     onmousedown="markerHome(7);"    >
                <img alt="v" src="icons/waypoint-down.png"   title="Beweeg dit punt naar beneden" onmousedown="markerMoveDown(7);">
                <img alt="-" src="icons/waypoint-remove.png" title="Verwijder dit punt"           onmousedown="markerRemove(7);"  >
            <tr id="point8">
              <td>
                <img name="waypoint8" src="icons/marker-8-grey.png" title="Waypoint 8 Position - (click voor plaatsen/verwijderen op map)" alt="Waypoint 8" onmousedown="markerToggleMap(8)">&nbsp;
              <td>
                <input name="lon8" type="text" size=7 title="Waypoint 8 Longitude" onchange="formSetCoords(8);"><!-- lon8 --> E&nbsp;
              <td>
                <input name="lat8" type="text" size=7 title="Waypoint 8 Latitude"  onchange="formSetCoords(8);"><!-- lat8 --> N&nbsp;
              <td>
                <img alt="o" src="icons/waypoint-centre.png" title="Centreer dit punt op map"     onmousedown="markerCentre(8);">
                <img alt="^" src="icons/waypoint-up.png"     title="Beweeg dit punt naar boven"   onmousedown="markerMoveUp(8);"  >
                <img alt="+" src="icons/waypoint-add.png"    title="Voeg hierna punt toe"         onmousedown="markerAddAfter(8);"><br>
                <img alt="~" src="icons/waypoint-home.png"   title="Toggle als thuis locatie"     onmousedown="markerHome(8);"    >
                <img alt="v" src="icons/waypoint-down.png"   title="Beweeg dit punt naar beneden" onmousedown="markerMoveDown(8);">
                <img alt="-" src="icons/waypoint-remove.png" title="Verwijder dit punt"           onmousedown="markerRemove(8);"  >
            <tr id="point9">
              <td>
                <img name="waypoint9" src="icons/marker-9-grey.png" title="Waypoint 9 Position - (click voor plaatsen/verwijderen op map)" alt="Waypoint 9" onmousedown="markerToggleMap(9)">&nbsp;
              <td>
                <input name="lon9" type="text" size=7 title="Waypoint 9 Longitude" onchange="formSetCoords(9);"><!-- lon9 --> E&nbsp;
              <td>
                <input name="lat9" type="text" size=7 title="Waypoint 9 Latitude"  onchange="formSetCoords(9);"><!-- lat9 --> N&nbsp;
              <td>
                <img alt="o" src="icons/waypoint-centre.png" title="Centreer dit punt op map"     onmousedown="markerCentre(9);">
                <img alt="^" src="icons/waypoint-up.png"     title="Beweeg dit punt naar boven"   onmousedown="markerMoveUp(9);"  >
                <img alt="+" src="icons/waypoint-add.png"    title="Voeg hierna punt toe"         onmousedown="markerAddAfter(9);"><br>
                <img alt="~" src="icons/waypoint-home.png"   title="Toggle als thuis locatie"     onmousedown="markerHome(9);"    >
                <img alt="v" src="icons/waypoint-down.png"   title="Beweeg dit punt naar beneden" onmousedown="markerMoveDown(9);">
                <img alt="-" src="icons/waypoint-remove.png" title="Verwijder dit punt"           onmousedown="markerRemove(9);"  >
            <!-- Up to 99 markers can be included here in the HTML  -->
            <tr>
              <td colspan="4" align="center"><input type="button" title="Keer volgorde punten om" value="Keer volgorde punten om" onmousedown="markersReverse();">
          </table>
        </div>
      </div>

      <div class="hideshow_box">
        <span id="hideshow_transport_show" onclick="hideshow_show('transport');" class="hideshow_hide">+</span>
        <span id="hideshow_transport_hide" onclick="hideshow_hide('transport');" class="hideshow_show">-</span>
        <span class="hideshow_title">Transport Type</span>
        <div id="hideshow_transport_div">
          <table>
            <tr><td>Te voet          <td><input name="transport" type="radio" value="foot"       onchange="formSetTransport('foot'      )"><!-- transport -->
            <tr><td>Paard            <td><input name="transport" type="radio" value="horse"      onchange="formSetTransport('horse'     )"><!-- transport -->
            <tr><td>Rolstoel         <td><input name="transport" type="radio" value="wheelchair" onchange="formSetTransport('wheelchair')"><!-- transport -->
            <tr><td>Fiets            <td><input name="transport" type="radio" value="bicycle"    onchange="formSetTransport('bicycle'   )"><!-- transport -->
            <tr><td>Brommer          <td><input name="transport" type="radio" value="moped"      onchange="formSetTransport('moped'     )"><!-- transport -->
            <tr><td>Motorfiets       <td><input name="transport" type="radio" value="motorbike"  onchange="formSetTransport('motorbike' )"><!-- transport -->
            <tr><td>Auto             <td><input name="transport" type="radio" value="motorcar"   onchange="formSetTransport('motorcar'  )"><!-- transport -->
            <tr><td>Goederen         <td><input name="transport" type="radio" value="goods"      onchange="formSetTransport('goods'     )"><!-- transport -->
            <tr><td>Zwaar transport  <td><input name="transport" type="radio" value="hgv"        onchange="formSetTransport('hgv'       )"><!-- transport -->
            <tr><td>Publiek transport<td><input name="transport" type="radio" value="psv"        onchange="formSetTransport('psv'       )"><!-- transport -->
          </table>
        </div>
      </div>

      <div class="hideshow_box">
        <span id="hideshow_highway_show" onclick="hideshow_show('highway');" class="hideshow_show">+</span>
        <span id="hideshow_highway_hide" onclick="hideshow_hide('highway');" class="hideshow_hide">-</span>
        <span class="hideshow_title">Voorkeur Wegtype</span>
        <div id="hideshow_highway_div" style="display: none;">
          <table>
            <tr><td>Autostrade           <td><input name="highway-motorway"     type="text" size=3 onchange="formSetHighway('motorway'    )"><!-- highway-motorway     --><td>%
            <tr><td>Autoweg:             <td><input name="highway-trunk"        type="text" size=3 onchange="formSetHighway('trunk'       )"><!-- highway-trunk        --><td>%
            <tr><td>Provinciale wegen:   <td><input name="highway-primary"      type="text" size=3 onchange="formSetHighway('primary'     )"><!-- highway-primary      --><td>%
            <tr><td>Nationale wegen:     <td><input name="highway-secondary"    type="text" size=3 onchange="formSetHighway('secondary'   )"><!-- highway-secondary    --><td>%
            <tr><td>Doorgangsweg:        <td><input name="highway-tertiary"     type="text" size=3 onchange="formSetHighway('tertiary'    )"><!-- highway-tertiary     --><td>%
            <tr><td>Niet geclassificeerd:<td><input name="highway-unclassified" type="text" size=3 onchange="formSetHighway('unclassified')"><!-- highway-unclassified --><td>%
            <tr><td>Woongebied:          <td><input name="highway-residential"  type="text" size=3 onchange="formSetHighway('residential' )"><!-- highway-residential  --><td>%
            <tr><td>Toegangsweg:         <td><input name="highway-service"      type="text" size=3 onchange="formSetHighway('service'     )"><!-- highway-service      --><td>%
            <tr><td>Veldweg:             <td><input name="highway-track"        type="text" size=3 onchange="formSetHighway('track'       )"><!-- highway-track        --><td>%
            <tr><td>Fietspad:            <td><input name="highway-cycleway"     type="text" size=3 onchange="formSetHighway('cycleway'    )"><!-- highway-cycleway     --><td>%
            <tr><td>Pad:                 <td><input name="highway-path"         type="text" size=3 onchange="formSetHighway('path'        )"><!-- highway-path         --><td>%
            <tr><td>Trap:                <td><input name="highway-steps"        type="text" size=3 onchange="formSetHighway('steps'       )"><!-- highway-steps        --><td>%
            <tr><td>Ferry:               <td><input name="highway-ferry"        type="text" size=3 onchange="formSetHighway('ferry'       )"><!-- highway-ferry        --><td>%
          </table>
        </div>
      </div>

      <div class="hideshow_box">
        <span id="hideshow_speed_show" onclick="hideshow_show('speed');" class="hideshow_show">+</span>
        <span id="hideshow_speed_hide" onclick="hideshow_hide('speed');" class="hideshow_hide">-</span>
        <span class="hideshow_title">Snelheidslimieten</span>
        <div id="hideshow_speed_div" style="display: none;">
          <table>
            <tr><td>Autostrade           <td><input name="speed-motorway"     type="text" size=3 onchange="formSetSpeed('motorway'    )"><!-- speed-motorway     --><td>km/hr
            <tr><td>Autoweg:             <td><input name="speed-trunk"        type="text" size=3 onchange="formSetSpeed('trunk'       )"><!-- speed-trunk        --><td>km/hr
            <tr><td>Provinciale wegen:   <td><input name="speed-primary"      type="text" size=3 onchange="formSetSpeed('primary'     )"><!-- speed-primary      --><td>km/hr
            <tr><td>Nationale wegen:     <td><input name="speed-secondary"    type="text" size=3 onchange="formSetSpeed('secondary'   )"><!-- speed-secondary    --><td>km/hr
            <tr><td>Doorgangsweg:        <td><input name="speed-tertiary"     type="text" size=3 onchange="formSetSpeed('tertiary'    )"><!-- speed-tertiary     --><td>km/hr
            <tr><td>Niet geclassificeerd:<td><input name="speed-unclassified" type="text" size=3 onchange="formSetSpeed('unclassified')"><!-- speed-unclassified --><td>km/hr
            <tr><td>Woongebied:          <td><input name="speed-residential"  type="text" size=3 onchange="formSetSpeed('residential' )"><!-- speed-residential  --><td>km/hr
            <tr><td>Toegangsweg:         <td><input name="speed-service"      type="text" size=3 onchange="formSetSpeed('service'     )"><!-- speed-service      --><td>km/hr
            <tr><td>Veldweg:             <td><input name="speed-track"        type="text" size=3 onchange="formSetSpeed('track'       )"><!-- speed-track        --><td>km/hr
            <tr><td>Fietspad:            <td><input name="speed-cycleway"     type="text" size=3 onchange="formSetSpeed('cycleway'    )"><!-- speed-cycleway     --><td>km/hr
            <tr><td>Pad:                 <td><input name="speed-path"         type="text" size=3 onchange="formSetSpeed('path'        )"><!-- speed-path         --><td>km/hr
            <tr><td>Trap:                <td><input name="speed-steps"        type="text" size=3 onchange="formSetSpeed('steps'       )"><!-- speed-steps        --><td>km/hr
            <tr><td>Ferry:               <td><input name="speed-ferry"        type="text" size=3 onchange="formSetSpeed('ferry'       )"><!-- speed-ferry        --><td>km/hr
          </table>
        </div>
      </div>

      <div class="hideshow_box">
        <span id="hideshow_property_show" onclick="hideshow_show('property');" class="hideshow_show">+</span>
        <span id="hideshow_property_hide" onclick="hideshow_hide('property');" class="hideshow_hide">-</span>
        <span class="hideshow_title">Weg Eigenschappen</span>
        <div id="hideshow_property_div" style="display: none;">
          <table>
            <tr><td>Verhard:         <td><input name="property-paved"        type="text" size=3 onchange="formSetProperty('paved'       )"><!-- property-paved        --><td>%
            <tr><td>Meerdere Stroken:<td><input name="property-multilane"    type="text" size=3 onchange="formSetProperty('multilane'   )"><!-- property-multilane    --><td>%
            <tr><td>Brug:            <td><input name="property-bridge"       type="text" size=3 onchange="formSetProperty('bridge'      )"><!-- property-bridge       --><td>%
            <tr><td>Tunnel:          <td><input name="property-tunnel"       type="text" size=3 onchange="formSetProperty('tunnel'      )"><!-- property-tunnel       --><td>%
            <tr><td>Walking Route:   <td><input name="property-footroute"    type="text" size=3 onchange="formSetProperty('footroute'   )"><!-- property-footroute    --><td>%
            <tr><td>Bicycle Route:   <td><input name="property-bicycleroute" type="text" size=3 onchange="formSetProperty('bicycleroute')"><!-- property-bicycleroute --><td>%
          </table>
        </div>
      </div>

      <div class="hideshow_box">
        <span id="hideshow_restriction_show" onclick="hideshow_show('restriction');" class="hideshow_show">+</span>
        <span id="hideshow_restriction_hide" onclick="hideshow_hide('restriction');" class="hideshow_hide">-</span>
        <span class="hideshow_title">Andere Beperkingen</span>
        <div id="hideshow_restriction_div" style="display: none;">
          <table>
            <tr><td>Volg Eenrichtingsverkeer:<td><input name="restrict-oneway" type="checkbox"    onchange="formSetRestriction('oneway')"><!-- oneway --><td>
            <tr><td>Obey turn restrictions:  <td><input name="restrict-turns"  type="checkbox"    onchange="formSetRestriction('turns' )"><!-- turns  --><td>
            <tr><td>Gewicht:                 <td><input name="restrict-weight" type="text" size=3 onchange="formSetRestriction('weight')"><!-- weight --><td> ton
            <tr><td>Hoogte:                  <td><input name="restrict-height" type="text" size=3 onchange="formSetRestriction('height')"><!-- height --><td> meter
            <tr><td>Breedte:                 <td><input name="restrict-width"  type="text" size=3 onchange="formSetRestriction('width' )"><!-- width  --><td> meter
            <tr><td>Lengte:                  <td><input name="restrict-length" type="text" size=3 onchange="formSetRestriction('length')"><!-- length --><td> meter
          </table>
        </div>
      </div>

      <div class="hideshow_box">
        <span class="hideshow_title">Zoek Route</span>
        <input type="button" title="Zoek de kortste route" id="shortest" value="Kortste" onclick="findRoute('shortest');">
        <input type="button" title="Zoek de snelste route" id="quickest" value="Snelste" onclick="findRoute('quickest');">
      </div>

      <div class="hideshow_box">
        <span class="hideshow_title">Links</span>
        <a id="link_url"                href="router.html">Permanente link naar deze parameters</a>
        <br>
        <a id="edit_url" target="other" href="http://wiki.openstreetmap.org/wiki/NL:Mapper">Lees hoe je OSM data kan inbrengen met Potlatch</a>
      </div>

      <div class="hideshow_box">
        <span id="hideshow_help_options_show" onclick="hideshow_show('help_options');" class="hideshow_hide">+</span>
        <span id="hideshow_help_options_hide" onclick="hideshow_hide('help_options');" class="hideshow_show">-</span>
        <span class="hideshow_title">Help</span>
        <div id="hideshow_help_options_div">
          <div class="scrollable">
            <p>
            <b>Quick Start</b>
            <br>
            Click op marker-icoontje (Waypoints) om ze op de map te plaatsen (rechts).
            Sleep ze vervolgens naar de gewenste positie. 
            Het is best om eerst naar straat niveau te zoomen op de kaart. 
            Selecteer het transport type, toegestane weg-types,
            snelheidslimieten, wegeigenschappen en andere restricties uit de
            opties.
            Selecteer  "Kortste" of "Snelste" om de route te berekenen en te tekenen op de map. 
            <p>
            <b>Coordinaten (Waypoints)</b>
            <br>
            Click op het marker icoontje, nog eens clicken voor aan/uit.
            Wanneer de route berekend wordt, zal dit nauwkeurig aansluiten bij de volgorde van deze punten. (rekening houdend met transport type)
            <p>
            <b>Transport Type</b>
            <br>
            Wanneer je een bepaald transport type kiest wordt bij berekenen
            route hiermede rekening gehouden.
            Het transport type bestaat uit een lijst met default waarden voor
            ieder wegtype.
            Deze percentages kunnen ook nog eens manueel aangepast worden.
            <p>
            <b>Voorkeur Wegtype</b>
            <br>
            De voorkeur voor een bepaald type weg wordt uitgedrukt in een percentage.  
            Bijvoorbeeld wanneer u het Transport Type "Fiets" kiest, dan zal er
            voor Autostrade 0% staan, en voor Fietspad 100%.
            Wanneer u Autowegen, Nationale wegen wil vermijden of beperken bij
            het maken van een fietsroute, kan u percentage naar beneden
            aanpassen.
            <p>
            <b>Snelheid limieten</b>
            <br>
            De snelheidslimieten worden afgeleid van het type weg. Het is
            mogelijk dat er voor een bepaalde weg andere beperkingen gelden. In
            dat geval worden die gekoezen. (het geval dat deze lager zijn dan de
            default)
            <p>
            <b>Weg Eigenschappen</b>
            <br>
            Voor het berekenen van de route, kan de de voorkeur gegeven worden
            aan een bepaalde wegeigenschap.
            Wanneer u kiest voor 25% verhard, zal er automatisch de voorkeur aan
            75% onverhard worden gegeven.
            Ook al is het onverharde stuk 3 X langer, toch kan er dan de
            voorkeur aan gegeven worden.
            <p>
            <b>Andere Beperkingen</b>
            <br>
            Deze zullen toelaten dat er een route berekend wordt die rekening
            houdt met gewicht, hoogte, breedte of lengte.
            Het is ook mogelijk geen rekening te houden met eenrichtingsverkeer
            (bijvoorbeeld als voetganger of fietser)
          </div>
        </div>
      </div>
    </form>
  </div>


  <div class="tab_content" id="tab_results_div" style="display: none;">

    <div class="hideshow_box">
      <span class="hideshow_title">Status</span>
      <div id="result_status">
        <div id="result_status_not_run">
          <b><i>Router niet in gebruik</i></b>
        </div>
        <div id="result_status_running"  style="display: none;">
          <b>Router werkt...</b>
        </div>
        <div id="result_status_complete" style="display: none;">
          <b>Routing voltooid</b>
          <br>
          <a id="router_log_complete" target="router_log" href="#">View Details</a>
        </div>
        <div id="result_status_error"    style="display: none;">
          <b>Router error</b>
          <br>
          <a id="router_log_error" target="router_log" href="#">View Details</a>
        </div>
        <div id="result_status_failed"   style="display: none;">
          <b>Router werkt niet</b>
        </div>
      </div>
    </div>

    <div class="hideshow_box">
      <span id="hideshow_shortest_show" onclick="hideshow_show('shortest');" class="hideshow_show">+</span>
      <span id="hideshow_shortest_hide" onclick="hideshow_hide('shortest');" class="hideshow_hide">-</span>
      <span class="hideshow_title">Kortste Route</span>
      <div id="shortest_status">
        <div id="shortest_status_no_info">
          <b><i>No Information</i></b>
        </div>
        <div id="shortest_status_info" style="display: none;">
        </div>
      </div>
      <div id="hideshow_shortest_div" style="display: none;">
        <div id="shortest_links" style="display: none;">
          <table>
            <tr><td>HTML directions:  <td><a id="shortest_html"      target="shortest_html"      href="#">Open Popup</a>
            <tr><td>GPX track bestand:<td><a id="shortest_gpx_track" target="shortest_gpx_track" href="#">Open Popup</a>
            <tr><td>GPX route bestand:<td><a id="shortest_gpx_route" target="shortest_gpx_route" href="#">Open Popup</a>
            <tr><td>Full text bestand:<td><a id="shortest_text_all"  target="shortest_text_all"  href="#">Open Popup</a>
            <tr><td>Text bestand:     <td><a id="shortest_text"      target="shortest_text"      href="#">Open Popup</a>
          </table>
          <hr>
        </div>
        <div id="shortest_route">
        </div>
      </div>
    </div>

    <div class="hideshow_box">
      <span id="hideshow_quickest_show" onclick="hideshow_show('quickest');" class="hideshow_show">+</span>
      <span id="hideshow_quickest_hide" onclick="hideshow_hide('quickest');" class="hideshow_hide">-</span>
      <span class="hideshow_title">Snelste Route</span>
      <div id="quickest_status">
        <div id="quickest_status_no_info">
          <b><i>No Information</i></b>
        </div>
        <div id="quickest_status_info" style="display: none;">
        </div>
      </div>
      <div id="hideshow_quickest_div" style="display: none;">
        <div id="quickest_links" style="display: none;">
          <table>
            <tr><td>HTML directions:  <td><a id="quickest_html"      target="quickest_html"      href="#">Open Popup</a>
            <tr><td>GPX track bestand:<td><a id="quickest_gpx_track" target="quickest_gpx_track" href="#">Open Popup</a>
            <tr><td>GPX route bestand:<td><a id="quickest_gpx_route" target="quickest_gpx_route" href="#">Open Popup</a>
            <tr><td>Full text bestand:<td><a id="quickest_text_all"  target="quickest_text_all"  href="#">Open Popup</a>
            <tr><td>Text bestand:     <td><a id="quickest_text"      target="quickest_text"      href="#">Open Popup</a>
          </table>
          <hr>
        </div>
        <div id="quickest_route">
        </div>
      </div>
    </div>

    <div class="hideshow_box">
      <span id="hideshow_help_route_show" onclick="hideshow_show('help_route');" class="hideshow_hide">+</span>
      <span id="hideshow_help_route_hide" onclick="hideshow_hide('help_route');" class="hideshow_show">-</span>
      <span class="hideshow_title">Help</span>
      <div id="hideshow_help_route_div">
        <div class="scrollable">
          <p>
          <b>Quick Start</b>
          <br>
          Na het berekenen van een route, kan het GPX bestand, of de beschrijving als tekstbestand downloaden.
          Door met muis over de beschrijving te bewegen, ziet u die ook op de kaart gesitueerd.
          <p style="margin-bottom: 0px">
          <b>Problem Solving</b>
          <br>
          Als de router eindigt met een fout, dan is de meest waarschijnlijke
          oorzaak, dat er geen route mogelijk is tussen de gekozen punten.
          Het verplaatsen van de punten, of het aanpassen van weg-eigenschappen
          of voertuigtype kan een oplossing bieden.
          <p style="margin-bottom: 0px">
          <b>Output Formats</b>
          <br>
          <dl style="margin-top: 0px">
            <dt>HTML instructies
            <dd>Een beschrijving van de route, met de te nemen afslag aan iedere splitsing.
            <dt>GPX track bestand
            <dd>Dezelfde informatie die op de kaart wordt weergegeven. Met
              coordinaten voor ieder knooppunt, en een track voor ieder segment.
            <dt>GPX route bestand
            <dd>Dezelfde informatie dat is opgenomen in de tekst van de route,
              met een coordinaat voor iedere belangrijke splitsing.
            <dt>Full text bestand
            <dd>Een lijst met alle coordinaten, met de afstand hier tussen. En
              een cumulatieve afstand voor iedere stap op de route.
            <dt>Text bestand
            <dd>Dezelfde informatie als wordt weergegeven in de tekst voor de route.
          </dl>
        </div>
      </div>
    </div>
  </div>


  <div class="tab_content" id="tab_data_div" style="display: none;">
    <div class="hideshow_box">
      <span class="hideshow_title">Statistics</span>
      <div id="statistics_data"></div>
      <a id="statistics_link" href="statistics.cgi" onclick="displayStatistics();return(false);">Display data statistics</a>
    </div>

    <div class="hideshow_box">
      <span class="hideshow_title">Visualiser</span>
      Om te kijken hoe routino omgaat met de basisdata, is er een tooltje dat de onderliggende data toont op verschillende manieren.
      <br>
      <a id="visualiser_url" target="other" href="visualiser.html">Custom link to this map view</a>
    </div>
  </div>

</div>

<!-- Right hand side of window - map -->

<div class="right_panel">
  <div class="map" id="map">
    <noscript>
      Javascript is <em>required</em> to use this web page because of the
      interactive map.
    </noscript>
  </div>
  <div class="attribution">
    <a target="other" href="http://www.routino.org/" title="Routino">Router: Routino</a>
    |
    <a target="other" href="http://www.openstreetmap.org/" title="Copyright: OpenStreetMap.org; License: Creative Commons Attribution-Share Alike 2.0">Geo Data: OpenStreetMap</a>
  </div>
</div>

</BODY>
</HTML>
