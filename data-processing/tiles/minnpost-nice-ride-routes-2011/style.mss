@station: #517F10;
@routes: #10517F;
@rw: 0.3; // Route width
@ro: 0.4; // Route opacity
@mag1: 1; // Magnifiers for routes
@mag2: 3;
@mag3: 5;

#route_counts {
  line-smooth: 0.1;
  line-width: @rw;
  line-color: @routes;
  line-opacity: @ro;
  [c > 0]   { line-width: @rw * @mag1 * 1.1; line-opacity: @ro * 1.1; }
  [c > 10]  { line-width: @rw * @mag1 * 1.5; line-opacity: @ro * 1.2; }
  [c > 50]  { line-width: @rw * @mag1 * 2.0; line-opacity: @ro * 1.3; }
  [c > 150] { line-width: @rw * @mag1 * 2.8; line-opacity: @ro * 1.4; }
  [c > 300] { line-width: @rw * @mag1 * 4.0; line-opacity: @ro * 1.5; }
  [c > 500] { line-width: @rw * @mag1 * 5.6; line-opacity: @ro * 1.6; }
}
#route_counts[zoom >= 13] {
  [c > 0]   { line-width: @rw * @mag2 * 1.1; line-opacity: @ro * 1.1; }
  [c > 10]  { line-width: @rw * @mag2 * 1.5; line-opacity: @ro * 1.2; }
  [c > 50]  { line-width: @rw * @mag2 * 2.0; line-opacity: @ro * 1.3; }
  [c > 150] { line-width: @rw * @mag2 * 2.8; line-opacity: @ro * 1.4; }
  [c > 300] { line-width: @rw * @mag2 * 4.0; line-opacity: @ro * 1.5; }
  [c > 500] { line-width: @rw * @mag2 * 5.6; line-opacity: @ro * 1.6; }
}
#route_counts[zoom >= 15] {
  [c > 0]   { line-width: @rw * @mag3 * 1.1; line-opacity: @ro * 1.1; }
  [c > 10]  { line-width: @rw * @mag3 * 1.5; line-opacity: @ro * 1.2; }
  [c > 50]  { line-width: @rw * @mag3 * 2.0; line-opacity: @ro * 1.3; }
  [c > 150] { line-width: @rw * @mag3 * 2.8; line-opacity: @ro * 1.4; }
  [c > 300] { line-width: @rw * @mag3 * 4.0; line-opacity: @ro * 1.5; }
  [c > 500] { line-width: @rw * @mag3 * 5.6; line-opacity: @ro * 1.6; }
}

#stations {
  marker-fill: @station;
  marker-opacity: 1;
  marker-line-width: 0;
  marker-allow-overlap: true;
  marker-width: 2;
 }
 #stations::glowing {
    marker-fill: @station;
    marker-opacity: 0.7;
    marker-width: 4;
    marker-line-width: 0;
    marker-allow-overlap: true;
 }
 #stations[zoom >= 13] { marker-width: 6; }
 #stations::glowing[zoom >= 13] { marker-width: 11; }
 #stations[zoom >= 15] { marker-width: 12; }
 #stations::glowing[zoom >= 15] { marker-width: 21; }
