Tile.prototype = $('<img></img>');
function Tile(src, left, top) {
	this.attr('src',src);
	this.css({
		position: 'absolute',
		height: '256px', width: '256px',
		left: left, top: top
	});
}

function MapQuestLayer(parent, template) {
	var layer = this; // Simpler
	
	var level;
	var tiles = [];
	
	layer.onmove = function() {
		var 
	}
	
	layer.onresize = function() {
		var heightTiles = Math.ceil(parent.height()/256);
		var widthTiles = Math.ceil(parent.width()/256);
		
		while(heightTiles < tiles.length) tiles.pop();
		while(heightTiles > tiles.length) tiles.push([]);
		
		tiles.foreach(function(row, rowindex) {
			while(widthTiles < row.length) row.pop();
			while(widthTiles > row.length) row.push(newTile(row.length,rowindex));
		});
	}
	
	layer.onzoom = function(newLevel) {
		zoom = parent.width();
	}
	
	function newTile(x, y) {
		return $('<img></img>')
			.attr('src',template
				.replace('{n}',(x*2 & 2 | y & 1) + 1)
				.replace('{x}',x)
				.replace('{y}',y)
				.replace('{z}'.level))
			.css({height: '256px', width: '256px'});
	}
	
	layer.onresize();
}

Map.prototype = $('<div></div>');
function Map() {
	// Private variables
	var map = this; // Avoid confusion
	
	var pos;
	
	// Private classes
	
	function Point(x, y, z) {
		this.x = x;
		this.y = y;
		this.z = z;
	}
	
	// Private functions
	
	function latLongToTile(zoom, lat, long) {
		function degToRad(deg) {return deg/180*Math.PI;}
		
		var scale = 1 << zoom;
		var x = (long + 180)%360/360*scale;
		var y = (1 - Math.log(Math.tan(degToRad(lat)) + 1/Math.cos(degToRad(lat)))/Math.PI)/2*scale;
		return new Point(x,y,zoom);
	}
	
	function checkVisibleTiles() {
	}
	
	// Public functions
	
	map.template = (function() {
		var template, counter;
		
		return function(newTemplate, newCounter) {
			if(newTemplate == undefined)
				return template.replace('{n}',counter());
			else {
				template = newTemplate;
				if(newCounter == undefined)
					counter = function() {return '';}
				else counter = newCounter;
			}
		}
	})();
	
	map.moveTo = function(lat, long) {
	}
	
	map.rotateTo = function(angle) {
	}
	
	map.zoomTo = function(zoom) {
	}
	
	// Initialization code
	
	map.css({
		'background-color': 'lightgray',
		height: '240px',
		overflow: 'hidden',
		width: '320px'
	});
	
	// Viewport
	var viewport = $('<div></div>');
	map.append(viewport);
	
	// Start out somewhere interesting
	map.template('http://oatile1.mqcdn.com/tiles/1.0.0/sat/{z}/{x}/{y}.jpg');
	map.moveTo(45.045357,-92.82585);
	map.zoomTo(15);
}
