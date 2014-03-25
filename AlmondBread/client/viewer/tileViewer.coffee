math =
   lerp: (t, from, to) -> t * to + (1-t) * from
   sq: (x) -> x * x
   cube: (x) -> x * x * x

Vect = (@x, @y) ->
Vect::len = -> Math.sqrt (math.sq(@x) + math.sq(@y))
Vect::angle = -> Math.atan2 @y, @x

# vector functions
v = (x,y) ->
   if y?
      new Vect x, y
   else
      new Vect x.x, x.y

v.rotate = (v1, v2) -> v(v1.x*v2.x - v1.y*v2.y, v1.x*v2.y + v1.y*v2.x)
v.forAngle = (a) ->  v(Math.cos(a), Math.sin(a))
v.add = (v1, v2) -> v(v1.x+v2.x, v1.y+v2.y)
v.sub = (v1, v2) -> v(v1.x-v2.x, v1.y-v2.y)
v.mul = (v1, s) -> v(v1.x*s, v1.y*s)
v.div = (v1, s) -> v(v1.x/s, v1.y/s)
v.dot = (v1, v2) -> v1.x*v2.x + v1.y*v2.y
v.unit = (v1) ->
   len = v1.len( )
   v(v1.x/len, v1.y/len)
v.lerp = (t, v1, v2) -> v( (math.lerp t, v1.x, v2.x), (math.lerp t, v1.y, v2.y) )
v.eq = (v1, v2) -> v1.x == v2.x and v1.y == v2.y
v.map = (f, v1) -> v( (f v1.x), (f v1.y) )

class TileViewer
   constructor: (@element, options) ->
      @tileSize = options.tileSize or 512
      
      x = options.x or 0
      y = options.y or 0
      @center = v x, y
      
      @zoom = options.zoom or 0
      
      @url = options.url or "http://localhost:8081/X{x}_Y{y}_Z{zoom}.bmp"
      @wheelScroll = 0
      @wheelScrollThreshold = 1
      
      @buildCanvas()

   buildCanvas: ->
      $element = $(@element)
      @canvas = $('<canvas>').appendTo $element

      @dragHandler = (evt) => @onDrag evt

      @canvas.bind 'mousedown', (evt) =>
         mouseCoord = (@getMouseCoord evt)
         @startDrag =
            x: (mouseCoord.x - @x)
            y: (mouseCoord.y - @y)

         @canvas.bind 'mousemove', @dragHandler

      @canvas.bind 'mouseup', (evt) =>
         @canvas.unbind 'mousemove', @dragHandler

      @canvas.bind 'mousewheel', (evt, delta) => @onMouseWheel evt delta
      @canvas.bind 'dblclick', (evt) => @onDoubleClick evt

      ###
      @canvas.bind 'click', (evt) =>
         coord = @getMouseCoord evt
         @moveTo coord.x, coord.y
      ###

      resizeCanvas = =>
         @width = window.innerWidth
         @height = window.innerHeight
         @canvas[0].width = @width
         @canvas[0].height = @height

      window.addEventListener 'resize', resizeCanvas, false

      resizeCanvas()

      @ctx = @canvas[0].getContext '2d'

   getMouseCoord: (evt) ->
      offset = @canvas.offset()
      v.add (v evt.pageX, evt.pageY) (v offset.left, offset.top)

   update: ->
      console.log @x, @y, @zoom

      resolution = Math.pow 2, -@zoom
      ctx = @ctx

      x = -1
      y = -1

      while (y < @height)
         while (x < @width)
            tileParams = 
               x: (@x + x) * resolution
               y: (@y + y) * resolution
               zoom: @zoom


            tileUrl = @parseUrl tileParams

            console.log tileUrl
            img = new Image()
            img.onload = -> ctx.drawImage @, x, y, @tileSize, @tileSize
            img.src = tileUrl

            x += @tileSize
         y += @tileSize
      


   zoomIn: -> @zoom(1)

   zoomOut: -> @zoom(-1)

   zoom: (distance=1) ->
      @zoom += distance

      @update()

   onMouseWheel: (evt, delta) ->
      @wheelScroll += delta
      mouseCoord = (@getMouseCoord evt)
      if (@wheelScroll > @wheelScrollThreshold)
         @moveTo mouseCoord.x, mouseCoord.y
         @zoomIn()
         @wheelScroll = 0
      else if (@wheelScroll < -@wheelScrollThreshold)
         @moveTo mouseCoord.x, mouseCoord.y
         @zoomOut()
         @wheelScroll = 0

   onDoubleClick: (evt) ->
      mouseCoord = (@getMouseCoord evt)
      @moveTo mouseCoord.x, mouseCoord.y
      @zoomIn()

   onDrag: (evt) ->
      mouseCoord = @getMouseCoord evt
      dx = @startDrag.x - mouseCoord.x
      dy = @startDrag.y - mouseCoord.y
      @moveTo @x+dx, @y+dy

   parseUrl: (data) ->
      @url.replace /\{ *([\w_]+) *\}/g, (variable, key) -> data[key]


