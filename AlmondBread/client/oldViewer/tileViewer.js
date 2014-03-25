testLRUCache( );


var limitZoom = false;
var maxZoom = 30;
var minZoom = 7;
var doubleClickZooms = false;

var tileserver = "http://localhost:8081/X{x}_Y{y}_Z{zoom}.png";

var pixelviewer = "http://www.google.com/";

// Bladerunner Mode:
var bladeRunner = false;
var flicker = 0.2;
var flickerColor = "128,128,128"; // must use r,g,b format. 
var overlay;
var bar;


var context;
var imageCache;
var canvas;
var info;

var size = {x: 0, y: 0};
var center = {x: 0, y: 0};
var topLeft = {x: 0, y: 0};
var tileSize = {x: 512, y: 512};
var zoomLevel = 7;
var dragOffset = null;
var mouse = {x: 0, y: 0};
var totalDelta = 0;
var scrollThreshold = 1;
var connectionError = false;

var loadingColor = "#888";
var sliderColor = "#FFF";

var inButtonPos = {x:10, y:10};
var outButtonPos = {x:10, y:30};

var showDropdowns = true;

var fps = 15;

if (limitZoom) {
    var sliderStart = {x: 10, y: 30};
    var sliderEnd = {x: 20, y: 200};
    var sliderNotchWidth = 5;

    var sliderNotches = maxZoom - minZoom + 1;
    var sliderStep = (sliderEnd.y-sliderStart.y)/(sliderNotches - 1);

    var draggingSlider = false;

    outButtonPos = {x: 10, y:sliderEnd.y+10};
} else {
    var sliderStart = {x: 0, y: 0};
    var sliderEnd = {x: 0, y: 0};
}

var buttons = {};

function createButton( id, src, pos, action ) {
    var img = new Image( );
    img.id = id;
    img.data = {
        pos: pos,
        action: action,
        size: {x: 0, y: 0},
        ready: false,
    };
    
    img.onload = function( ) {
        this.data.size = {x: this.width, y: this.height};
        this.data.ready = true;
        buttons[this.id] = this.data;
        buttons[this.id].image = this;
        delete this.data;
    };
    
    img.src = src;
}

function drawLine( start, end ) {
	context.beginPath( );
	context.moveTo( start.x, start.y );
	context.lineTo( end.x, end.y );
	context.closePath( );
	context.stroke( );
}

function drawInterface( ) {
    var id;
    var i;
    
    for (id in buttons) {
        button = buttons[id];
        if (button.ready) {
            context.drawImage( button.image, button.pos.x, button.pos.y, button.size.x, button.size.y );
        }
    }
    
    if (limitZoom) {
        // init line drawing
    	context.strokeStyle = sliderColor;
    	context.fillStyle = sliderColor;
    	context.lineWidth = 1;
    	//context.lineCap = 'square';
        
        // draw slider:
        sliderCenter = (sliderStart.x+sliderEnd.x)/2;
        drawLine( {x: sliderCenter, y: sliderStart.y},
                  {x: sliderCenter, y: sliderEnd.y} );
        for (i = 0; i < sliderNotches; i++) {
            y = sliderStart.y + sliderStep*i;
            drawLine( {x: sliderCenter - sliderNotchWidth/2, y: y},
                      {x: sliderCenter + sliderNotchWidth/2, y: y} );
        }
        
        // draw box:
        if (draggingSlider) {
            y = mouse.y;
            if (y < sliderStart.y) {
                y = sliderStart.y;
            }
            if (y > sliderEnd.y) {
                y = sliderEnd.y;
            }
        } else {
            y = -(zoomLevel - minZoom) * sliderStep + sliderEnd.y;
        }
        context.fillRect( sliderStart.x, y - 2.5, sliderEnd.x - sliderStart.x, 5 );
    }
}

function clickInterface( ev ) {
    var id;
    
    mouse = getMouse( ev );
    
    // if the click is within the bounding box for a button
    // evaluate that button's action.
    for (id in buttons) {
        button = buttons[id];
        if (button.ready) {
            if (mouse.x > button.pos.x &&
                mouse.y > button.pos.y &&
                mouse.x < button.pos.x + button.size.x &&
                mouse.y < button.pos.y + button.size.y) {
        
                eval(button.action);
            }
        }
    }
    
    if (mouse.x > sliderStart.x &&
        mouse.y > sliderStart.y &&
        mouse.x < sliderEnd.x &&
        mouse.y < sliderEnd.y) {
        moveSlider(mouse.y);
    }
}

function moveSlider(mouseY) {
    // handle slider click
    if (mouseY < sliderStart.y) {
        mouseY = sliderStart.y;
    }
    if (mouseY > sliderEnd.y) {
        mouseY = sliderEnd.y;
    }
    notch = minZoom + (-Math.round((mouseY - sliderEnd.y) / sliderStep));
    delta = notch - zoomLevel;
    if (delta !== 0) {
        zoom(delta, center, center);
    } else {
        redraw( );
    }
}

function onButton( pos ) {
    var id;
    
    // check bounding boxes
    for (id in buttons) {
        button = buttons[id];
        if (button.ready) {
            if (pos.x > button.pos.x &&
                pos.y > button.pos.y &&
                pos.x < button.pos.x + button.size.x &&
                pos.y < button.pos.y + button.size.y) {
        
                return true;
            }
        }
    }
    if (mouse.x > sliderStart.x &&
        mouse.y > sliderStart.y &&
        mouse.x < sliderEnd.x &&
        mouse.y < sliderEnd.y) {

        return true;
    }
    return false;
}

function createInterface( ) {
    createButton("in", "media/in.png", inButtonPos, "zoom(1, center, center);");
    createButton("out", "media/out.png", outButtonPos, "zoom(-1, center, center);");
}

function tileURL( x, y, zoom ) {
    resolution = Math.pow(2, -zoom);
    
    var center = {x: (x + tileSize.x/2) * resolution,
                  y: - (y + tileSize.y/2) * resolution};              
    
    var data = {
        x: center.x,
        y: center.y,
        zoom: zoom
    };

    var tilename = tileserver.replace(/\{ *([\w_]+) *\}/g, function(variable, key) {
      return data[key];
    });

    //var tilename = prefix + center.x + "_" + center.y + "_" + zoom + suffix + "." + fileType + "?width=512&height=512&colourmap=6";
    
    return tilename;
}

function rnd_snd() {
	return (Math.random()*2-1)+(Math.random()*2-1)+(Math.random()*2-1);
}

function rnd(mean, stdev) {
	return Math.round(rnd_snd()*stdev+mean);
}

function drawVisibleTiles( ) {
    var start = {x: topLeft.x - (topLeft.x % tileSize.x) - tileSize.x,
                 y: topLeft.y - (topLeft.y % tileSize.y) - tileSize.y};
    
    var end = {x: topLeft.x + size.x,
               y: topLeft.y + size.y};

    var image;

    var x, y;

    for ( x = start.x; x < end.x; x += tileSize.x ) {
        for ( y = start.y; y < end.y; y += tileSize.y ) {
            image = imageCache.get( tileURL( x, y, zoomLevel ) );
            if ( image.complete && image.width !== 0) {
                context.drawImage( image, x-topLeft.x, y-topLeft.y, tileSize.x, tileSize.y );
                //console.log((x-topLeft.x) +" "+ (y-topLeft.y) +" "+ tileSize.x +" "+ tileSize.y)
            } else {
                //setTimeout(redraw, 500);
            	context.fillStyle = loadingColor;
                context.fillRect( x-topLeft.x, y-topLeft.y, tileSize.x, tileSize.y );
                
                if (image.complete && !connectionError) {
                        connectionError = true;
                        alert("Connection error.");
                }
            }
        }
    }
    
    if ( bladeRunner && overlay.complete ) {
    	context.fillStyle = "rgba("+flickerColor+","+(Math.random()*flicker)+")";
        context.fillRect( 0, 0, canvas.width( ), canvas.height( ) );
        context.drawImage( overlay, 0, 0, canvas.width( ), canvas.height( ) );

        context.drawImage( bar, 0, canvas.height( ) + rnd(-40, 5), canvas.width( ), bar.height );
        context.drawImage( bar, 0, rnd(-60, 5), canvas.width( ), bar.height );
    }
}

function redraw( ) {
    drawVisibleTiles( );
    drawInterface( );
}

function imageLoadedEvent( img ) {
    if ( img.width === 0 && !connectionError ) {
        connectionError = true;
        alert("Connection error.");
    }
    console.log(img.src);
    //redraw( );
}

function imageLoader( key ) {
    var img = new Image( );

    img.onload = function( ) {
        imageLoadedEvent( img );
    };

    img.src = key;

    return img;
}

function getFractalPoint( screenPoint ) {
    return {x: topLeft.x + screenPoint.x,
            y: topLeft.y + screenPoint.y};
}

function moveFractalPointTo( fractalPoint, screenPoint ) {
    topLeft = {x: fractalPoint.x - screenPoint.x,
               y: fractalPoint.y - screenPoint.y};
}

function zoom( delta, point, newPoint ) {
    if ( !limitZoom ||
         (zoomLevel + delta <= maxZoom &&
          zoomLevel + delta >= minZoom) ) {

        var fractalPoint = getFractalPoint(point);
        fractalPoint.x *= Math.pow(2, delta);
        fractalPoint.y *= Math.pow(2, delta);
        zoomLevel += delta;
        moveFractalPointTo( fractalPoint, newPoint );
        redraw();
        updateInfo( );
    }
}

function getMouse( ev ) {
    return {x: ev.pageX - canvas.offset( ).left,
            y: ev.pageY - canvas.offset( ).top};
}

function mousewheel( ev, delta ) {
    totalDelta += delta;
    if (totalDelta > scrollThreshold) {
        zoom( 1, mouse, mouse );
        totalDelta = 0;
    } else if (totalDelta < -scrollThreshold){
        zoom( -1, mouse, mouse );
        totalDelta = 0;
    }
    //redraw( );
    
    updateInfo( );
}

function mouseMove( ev ) {
    mouse = getMouse( ev );
    if (dragOffset !== null) {
        topLeft.x = dragOffset.x - mouse.x;
        topLeft.y = dragOffset.y - mouse.y;
        redraw( );
        updateInfo( );
    } else if ( draggingSlider ) {
        moveSlider( mouse.y );
    }
}

function mouseDown( ev ) {
    mouse = getMouse( ev );
    
    if ( mouse.x > sliderStart.x &&
         mouse.y > sliderStart.y &&
         mouse.x < sliderEnd.x &&
         mouse.y < sliderEnd.y ) {
        draggingSlider = true;
    } else if ( !onButton( mouse ) ){
        dragOffset = {x: topLeft.x + mouse.x,
                      y: topLeft.y + mouse.y};
    }
}

function mouseUp( ev ) {
    mouse = getMouse( ev );
    dragOffset = null;
    if (draggingSlider) {
        draggingSlider = false;
    }
}

function doubleClick( ev ) {
    mouse = getMouse( ev );
    if (doubleClickZooms) {
        if (!onButton(mouse)) {
            zoom( 1, mouse, center );
        }
    } else {
        x = (mouse.x + topLeft.x) * Math.pow(2, -zoomLevel);
        y = (mouse.y + topLeft.y) * Math.pow(2, -zoomLevel);
        window.open( pixelviewer + "?x=" + x + "&y=" + y );
    }
}

function change( ev ) {
    // set zoom level
    z = parseInt( $( "#zoomInput" ).val( ) );
    
    if (!isNaN(z) && ((z >= minZoom && z <= maxZoom) || !limitZoom)) {
        delta = z - zoomLevel;
        if ( delta !== 0 ) {
            zoom( delta, center, center );
        }
    }
    
    // set position
    x = parseFloat( $( "#xInput" ).val( ) ) * Math.pow(2, zoomLevel);
    y = parseFloat( $( "#yInput" ).val( ) ) * Math.pow(2, zoomLevel);
    if (!isNaN(x) && !isNaN(y)) {
        moveFractalPointTo( {x: x, y: y}, center );
    }
}

function updateInfo( ) {
    $( "#xInput" ).val( (topLeft.x + center.x) * Math.pow( 2, -zoomLevel ) );
    $( "#yInput" ).val( (topLeft.y + center.y) * Math.pow( 2, -zoomLevel ) );
    $( "#zoomInput" ).val( zoomLevel );
}

function setupTileViewer( canvasId, infoId, optionsId ) {
    loadSettings();
    
    info = $(infoId);
    info.html('<table>'+
              '<tr><td>X:</td><td><input type="text" id="xInput" class="info"></td><tr>' +
              '<tr><td>Y:</td><td><input type="text" id="yInput" class="info"></td><tr>' +
              '<tr><td>Zoom level:</td><td><input type="text" id="zoomInput" class="info"></td><tr>' +
              '</table>');
    $(".info").keyup(change);
    updateInfo( );

    canvas = $(canvasId);
    canvas.mousedown( mouseDown );
    canvas.mouseup( mouseUp );
    canvas.mousemove( mouseMove );
    canvas.dblclick( doubleClick );
    canvas.mousewheel( mousewheel );
    canvas.click( clickInterface );

    size = {x: canvas.width( ), y: canvas.height( )};
    center = {x: size.x/2, y: size.y/2};
    topLeft = {x: -size.x/2, y: -size.y/2};
    context = canvas[0].getContext( "2d" );

    imageCache = new LRUCache( 10, imageLoader );

    createInterface( );

    if (bladeRunner) {
        overlay = new Image( );
        overlay.src = "media/mask.png";
        bar = new Image( );
        bar.src = "media/bar.png";
    }
    
    if (showDropdowns) {
        html = '<select id="yourPixelMode">'+
               '<option value="normal">Normal</option>'+
               '<option value="black">Black</option>'+
               '<option value="white">White</option>'+
               '<option value="highlight">Highlight</option>'+
               '</select>'+
               '<select id="othersPixelMode">'+
               '<option value="normal">Normal</option>'+
               '<option value="black">Black</option>'+
               '<option value="white">White</option>'+
               '<option value="highlight">Highlight</option>'+
               '</select>';
        $(optionsId).html(html);
        
        $( "#othersPixelMode" ).change( modeChange );
        $( "#yourPixelMode" ).change( modeChange );
        modeChange( );
    }
    
    //redraw( );
    setInterval( redraw, 1000/fps );
}

function modeChange( ev ) {
    suffix = "_" + $( "#yourPixelMode" ).val( ) + "_" + $( "#othersPixelMode" ).val( );
    
    imageCache.clear( );
    redraw( );
}

function loadSettings( ev ) {
    /*
    if (tileViewerSettings != undefined) {
        for (var i in tileViewerSettings) {
            eval( i + " = " + tileViewerSettings + ";" );
        }
    }*/
}

$(document).ready(function() {
    $("#viewer").html('<canvas id="viewerCanvas" width="800" height="600"></canvas><div id="viewerInfo"></div><div id="options"></div>');

    setupTileViewer("#viewerCanvas", "#viewerInfo", "#options");
});
