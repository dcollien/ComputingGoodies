var TileViewer, Vect, math, v;

math = {
  lerp: function(t, from, to) {
    return t * to + (1 - t) * from;
  },
  sq: function(x) {
    return x * x;
  },
  cube: function(x) {
    return x * x * x;
  }
};

Vect = function(x, y) {
  this.x = x;
  this.y = y;
};

Vect.prototype.len = function() {
  return Math.sqrt(math.sq(this.x) + math.sq(this.y));
};

Vect.prototype.angle = function() {
  return Math.atan2(this.y, this.x);
};

v = function(x, y) {
  if (y != null) {
    return new Vect(x, y);
  } else {
    return new Vect(x.x, x.y);
  }
};

v.rotate = function(v1, v2) {
  return v(v1.x * v2.x - v1.y * v2.y, v1.x * v2.y + v1.y * v2.x);
};

v.forAngle = function(a) {
  return v(Math.cos(a), Math.sin(a));
};

v.add = function(v1, v2) {
  return v(v1.x + v2.x, v1.y + v2.y);
};

v.sub = function(v1, v2) {
  return v(v1.x - v2.x, v1.y - v2.y);
};

v.mul = function(v1, s) {
  return v(v1.x * s, v1.y * s);
};

v.div = function(v1, s) {
  return v(v1.x / s, v1.y / s);
};

v.dot = function(v1, v2) {
  return v1.x * v2.x + v1.y * v2.y;
};

v.unit = function(v1) {
  var len;
  len = v1.len();
  return v(v1.x / len, v1.y / len);
};

v.lerp = function(t, v1, v2) {
  return v(math.lerp(t, v1.x, v2.x), math.lerp(t, v1.y, v2.y));
};

v.eq = function(v1, v2) {
  return v1.x === v2.x && v1.y === v2.y;
};

v.map = function(f, v1) {
  return v(f(v1.x), f(v1.y));
};

TileViewer = (function() {

  function TileViewer(element, options) {
    var x, y;
    this.element = element;
    this.tileSize = options.tileSize || 512;
    x = options.x || 0;
    y = options.y || 0;
    this.center = v(x, y);
    this.zoom = options.zoom || 0;
    this.url = options.url || "http://localhost:8081/X{x}_Y{y}_Z{zoom}.bmp";
    this.wheelScroll = 0;
    this.wheelScrollThreshold = 1;
    this.buildCanvas();
  }

  TileViewer.prototype.buildCanvas = function() {
    var $element, resizeCanvas,
      _this = this;
    $element = $(this.element);
    this.canvas = $('<canvas>').appendTo($element);
    this.dragHandler = function(evt) {
      return _this.onDrag(evt);
    };
    this.canvas.bind('mousedown', function(evt) {
      var mouseCoord;
      mouseCoord = _this.getMouseCoord(evt);
      _this.startDrag = {
        x: mouseCoord.x - _this.x,
        y: mouseCoord.y - _this.y
      };
      return _this.canvas.bind('mousemove', _this.dragHandler);
    });
    this.canvas.bind('mouseup', function(evt) {
      return _this.canvas.unbind('mousemove', _this.dragHandler);
    });
    this.canvas.bind('mousewheel', function(evt, delta) {
      return _this.onMouseWheel(evt(delta));
    });
    this.canvas.bind('dblclick', function(evt) {
      return _this.onDoubleClick(evt);
    });
    /*
          @canvas.bind 'click', (evt) =>
             coord = @getMouseCoord evt
             @moveTo coord.x, coord.y
    */
    resizeCanvas = function() {
      _this.width = window.innerWidth;
      _this.height = window.innerHeight;
      _this.canvas[0].width = _this.width;
      return _this.canvas[0].height = _this.height;
    };
    window.addEventListener('resize', resizeCanvas, false);
    resizeCanvas();
    return this.ctx = this.canvas[0].getContext('2d');
  };

  TileViewer.prototype.getMouseCoord = function(evt) {
    var offset;
    offset = this.canvas.offset();
    return v.add((v(evt.pageX, evt.pageY))(v(offset.left, offset.top)));
  };

  TileViewer.prototype.update = function() {
    var ctx, img, resolution, tileParams, tileUrl, x, y, _results;
    console.log(this.x, this.y, this.zoom);
    resolution = Math.pow(2, -this.zoom);
    ctx = this.ctx;
    x = -1;
    y = -1;
    _results = [];
    while (y < this.height) {
      while (x < this.width) {
        tileParams = {
          x: (this.x + x) * resolution,
          y: (this.y + y) * resolution,
          zoom: this.zoom
        };
        tileUrl = this.parseUrl(tileParams);
        console.log(tileUrl);
        img = new Image();
        img.onload = function() {
          return ctx.drawImage(this, x, y, this.tileSize, this.tileSize);
        };
        img.src = tileUrl;
        x += this.tileSize;
      }
      _results.push(y += this.tileSize);
    }
    return _results;
  };

  TileViewer.prototype.zoomIn = function() {
    return this.zoom(1);
  };

  TileViewer.prototype.zoomOut = function() {
    return this.zoom(-1);
  };

  TileViewer.prototype.zoom = function(distance) {
    if (distance == null) distance = 1;
    this.zoom += distance;
    return this.update();
  };

  TileViewer.prototype.onMouseWheel = function(evt, delta) {
    var mouseCoord;
    this.wheelScroll += delta;
    mouseCoord = this.getMouseCoord(evt);
    if (this.wheelScroll > this.wheelScrollThreshold) {
      this.moveTo(mouseCoord.x, mouseCoord.y);
      this.zoomIn();
      return this.wheelScroll = 0;
    } else if (this.wheelScroll < -this.wheelScrollThreshold) {
      this.moveTo(mouseCoord.x, mouseCoord.y);
      this.zoomOut();
      return this.wheelScroll = 0;
    }
  };

  TileViewer.prototype.onDoubleClick = function(evt) {
    var mouseCoord;
    mouseCoord = this.getMouseCoord(evt);
    this.moveTo(mouseCoord.x, mouseCoord.y);
    return this.zoomIn();
  };

  TileViewer.prototype.onDrag = function(evt) {
    var dx, dy, mouseCoord;
    mouseCoord = this.getMouseCoord(evt);
    dx = this.startDrag.x - mouseCoord.x;
    dy = this.startDrag.y - mouseCoord.y;
    return this.moveTo(this.x + dx, this.y + dy);
  };

  TileViewer.prototype.parseUrl = function(data) {
    return this.url.replace(/\{ *([\w_]+) *\}/g, function(variable, key) {
      return data[key];
    });
  };

  return TileViewer;

})();
