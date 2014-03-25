/*
 * Least-Recently-Used Cache
 * David Collien, 2011
 * All Rights Reserved
 */
function LRUCacheNode( key, value ) {
   this.key = key;
   this.value = value;
   this.next = null;
   this.prev = null;
}

function LRUCache( maxSize, sourceFunction ) {
   this.sourceFunction = sourceFunction;
   this.maxSize = maxSize;
   this.size = 0;
   this.table = { };
   this.first = null;
   this.last  = null;
};

LRUCache.prototype.listToString = function( ) {
   var string = "";
   var node = this.first;
   while ( node != null ) {
      if ( node != this.first ) {
         string += " <--> ";
      }
      string += "(" + node.key + ", " + node.value + ")";
      node = node.next;
   }

   return string;
};

LRUCache.prototype.addToFront = function( node ) {
   // place in front of the first
   node.next = this.first;
   node.prev = null;
   
   // link the first one back to this node
   if ( this.first != null ) {
      this.first.prev = node;
   } else {
      // no first node, this is also the last node
      this.last = node;
   }
   this.first = node;
};

LRUCache.prototype.moveToFront = function( node ) {
   this.removeNode( node );
   this.addToFront( node );
};

LRUCache.prototype.removeNode = function( node ) {
   var prev = node.prev;
   var next = node.next;

   // make the previous node skip forward over this node
   if ( prev != null ) {
      prev.next = next;
   }
   
   // make the next node skip back over this node
   if ( next != null ) {
      next.prev = prev;
   }
   
   // make sure the first and last pointers are correct
	if ( node == this.last && node == this.first ) {
		this.first = null;
		this.last  = null;
	} else if ( node == this.last ) {
      this.last = node.prev;
   } else if ( node == this.first ) {
      this.first = node.next;
   }
};


// LRUCache Interface

LRUCache.prototype.add = function( key, value ) {
   var node = new LRUCacheNode( key, value );
   this.table[key] = node;
   
   // add a node at the front
   this.addToFront( node );
   
   if ( this.size >= this.maxSize ) {
      // cache is full, delete last node
      var last = this.last;
      this.removeNode( last );
      delete this.table[last.key];
      delete last.key;
      delete last.value;
      delete last;
   } else {
		// nothing removed, so cache grows
      this.size++;
   }
};

LRUCache.prototype.get = function( key ) {
   var value;
   if ( this.table.hasOwnProperty( key ) ) {
      // look up the key in the table
      var node = this.table[key];
      value = node.value;
            
      if ( this.first != node ) {
         // move the node to the front of the list
         this.moveToFront( node );
      }
      
   } else {
      // generate a new value
      value = this.sourceFunction( key );
      this.add( key, value );
   }
   
   return value;
};

LRUCache.prototype.clear = function( ) {
	while ( this.first != null ) {
		this.remove( this.first.key );
	}
}

LRUCache.prototype.remove = function( key ) {
	if ( this.table.hasOwnProperty( key ) ) {
      // look up the key in the table
      var node = this.table[key];

		// remove from the table
		delete this.table[key];
		
		// remove from the list
		this.removeNode( node );
      
		// remove node properties
		delete node.key;
		delete node.value;
		delete node;
		
		// cache shrinks
		this.size--;
   }
}


// Test Cases

function testLRUCache( ) {
   var passed = true;
   
   function loadValue( key ) {
      return key.length + 100;
   }
   
   var cache = new LRUCache( 10, loadValue );
   cache.add( 'a', 1 );
   cache.add( 'be', 2 );
   cache.add( 'cee', 3 );
   cache.add( 'deee', 4 );
   cache.add( 'eeeee', 5 );
   cache.add( 'ffffff', 6 );
   cache.add( 'geeeeee', 7 );
   cache.add( 'haichhhh', 8 );
   cache.add( 'ieeeeeeee', 9 );
   cache.add( 'jaaaaaaaay', 10 );
   cache.add( 'kaaaaaaaaay', 11 );
   
   if ( cache.get( 'a' ) != 101 ) {
      passed = false;
      alert( "Test 1 Failed" );
   }
   delete cache;
   
   
   var cache = new LRUCache( 10, loadValue );
   cache.add( 'a', 1 );
   cache.add( 'be', 2 );
   cache.add( 'cee', 3 );
   cache.add( 'deee', 4 );
   cache.add( 'eeeee', 5 );
   cache.add( 'ffffff', 6 );
	// refreshes 'a' so it's pushed to the front
   cache.get( 'a' );
   cache.add( 'geeeeee', 7 );
   cache.add( 'haichhhh', 8 );
   cache.add( 'ieeeeeeee', 9 );
   cache.add( 'jaaaaaaaay', 10 );
   cache.add( 'kaaaaaaaaay', 11 );    
   
	// 'a' was refreshed only a few 'add's ago, should still be in the cache
	// (101 is the regenerated value, so it should still be 1)
   if ( cache.get( 'a' ) != 1 ) {
		// isn't the initial value, something went wrong
      passed = false;
      alert( "Test 2 Failed" );
   }

	// 'be' was never refreshed (with get) and would have fallen off the end (limit 10)
	// by now, so it should be 102 (regenerated value)
   if ( cache.get( 'be' ) != 102 ) {
      passed = false;
      alert( "Test 3 Failed" );
   }

   if ( cache.get( 'cee' ) != 103 ) {
      passed = false;
      alert( "Test 4 Failed" );
   }
   
	// 'eeeee' should have survived
   if ( cache.get( 'eeeee' ) != 5 ) {
      passed = false;
      alert( "Test 5 Failed" );
   }
   
	cache.clear( );
	// 'eeeee' should not be its initial value, because the cache has been cleared
	// so it should be 105 (regenerated value)
   if ( cache.get( 'eeeee' ) == 5 ) {
		passed = false;
		alert( "Test 6 Failed" );
	}

   return passed;
}