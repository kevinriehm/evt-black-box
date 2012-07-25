function Point(x, y, z) {
 this.x = x;
 this.y = y;
 this.z = z;
}

Tile.prototype = $('<img>');

function Tile(point) {
 var tile = this; // Avoid confusion
 
 tile.css({
  height: '256px',
  width: '256px'
 });
 
 tile.attr('src','http://oatile1.mqcdn.com/tiles/1.0.0/sat/'
  + point.z + '/' + Math.floor(point.x) + '/' + Math.floor(point.y) + '.jpg');
}

Map.prototype = $('<div></div>');

function Map() {
 var map = this; // Avoid confusion

 function latLongToTile(zoom, lat, long) {
  function degToRad(deg) {return deg/180*Math.PI;}
  
  var scale = 1 << zoom;
  var x = (long + 180)%360/360*scale;
  var y = (1 - Math.log(Math.tan(degToRad(lat)) + 1/Math.cos(degToRad(lat)))/Math.PI)/2*scale;
  return new Point(x,y,zoom);
 }
 
 map.moveTo = function(lat, long) {
 }

 map.rotateTo = function(angle) {
 }

 map.zoomTo = function(zoom) {
 }
 
 map.css({
  'background-color': 'lightgray',
  height: '240px',
  overflow: 'hidden',
  width: '320px'
 });
 
 // Viewport
 var viewport = $('<div></div>');
 map.append(viewport);viewport.append(new Tile(latLongToTile(15,45.045357,-92.82585)));
}
