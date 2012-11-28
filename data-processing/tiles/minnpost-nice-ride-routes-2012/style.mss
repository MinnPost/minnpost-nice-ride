@station: #517F10;
@routes: #10517F;
@rw: 0.3; // Route width
@ro: 0.5; // Route opacity
@mag1: 1.2; // Magnifiers for routes
@mag2: 3;
@mag3: 5;


#route_counts {
  line-smooth: 0.1;
  line-width: @rw;
  line-color: @routes;
  line-opacity: @ro;
  [c > 0]    { line-width: @rw * @mag1 * 1.1;  line-opacity: @ro * 1.10; }
  [c > 30]   { line-width: @rw * @mag1 * 1.5;  line-opacity: @ro * 1.15; }
  [c > 62]   { line-width: @rw * @mag1 * 2.0;  line-opacity: @ro * 1.00; }
  [c > 125]  { line-width: @rw * @mag1 * 2.8;  line-opacity: @ro * 0.95; }
  [c > 250]  { line-width: @rw * @mag1 * 4.0;  line-opacity: @ro * 0.90; }
  [c > 500]  { line-width: @rw * @mag1 * 5.7;  line-opacity: @ro * 0.85; }
  [c > 1000] { line-width: @rw * @mag1 * 8.0;  line-opacity: @ro * 0.80; }
  [c > 2000] { line-width: @rw * @mag1 * 10.0; line-opacity: @ro * 0.75; }
}
#route_counts[zoom >= 13] {
  [c > 0]    { line-width: @rw * @mag2 * 1.1;  line-opacity: @ro * 1.10; }
  [c > 30]   { line-width: @rw * @mag2 * 1.5;  line-opacity: @ro * 1.15; }
  [c > 62]   { line-width: @rw * @mag2 * 2.0;  line-opacity: @ro * 1.00; }
  [c > 125]  { line-width: @rw * @mag2 * 2.8;  line-opacity: @ro * 0.95; }
  [c > 250]  { line-width: @rw * @mag2 * 4.0;  line-opacity: @ro * 0.90; }
  [c > 500]  { line-width: @rw * @mag2 * 5.7;  line-opacity: @ro * 0.85; }
  [c > 1000] { line-width: @rw * @mag2 * 8.0;  line-opacity: @ro * 0.80; }
  [c > 2000] { line-width: @rw * @mag2 * 10.0; line-opacity: @ro * 0.75; }
}
#route_counts[zoom >= 15] {
  [c > 0]    { line-width: @rw * @mag3 * 1.1;  line-opacity: @ro * 1.10; }
  [c > 30]   { line-width: @rw * @mag3 * 1.5;  line-opacity: @ro * 1.15; }
  [c > 62]   { line-width: @rw * @mag3 * 2.0;  line-opacity: @ro * 1.00; }
  [c > 125]  { line-width: @rw * @mag3 * 2.8;  line-opacity: @ro * 0.95; }
  [c > 250]  { line-width: @rw * @mag3 * 4.0;  line-opacity: @ro * 0.90; }
  [c > 500]  { line-width: @rw * @mag3 * 5.7;  line-opacity: @ro * 0.85; }
  [c > 1000] { line-width: @rw * @mag3 * 8.0;  line-opacity: @ro * 0.80; }
  [c > 2000] { line-width: @rw * @mag3 * 10.0; line-opacity: @ro * 0.75; }
}

#stations {
  marker-fill: @station;
  marker-opacity: 1;
  marker-line-width: 0;
  marker-allow-overlap: true;
  marker-width: [bike_docks] / 5;
 }
 #stations::glowing {
    marker-fill: @station;
    marker-opacity: 0.7;
    marker-line-width: 0;
    marker-allow-overlap: true;
    marker-width: [bike_docks] / 1.5;
 }
 #stations[zoom >= 14] { marker-width: [bike_docks] / 2; }
 #stations::glowing[zoom >= 14] { marker-width: [bike_docks]; }