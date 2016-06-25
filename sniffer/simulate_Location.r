### get an example
source('notation.R')

# previous calculation to compare back to.
ab.soln = ab
ah.soln = ah
cb.soln = cb
ch.soln = ch
bl.soln = bl

# these need to match Network.h definitions
BASE_LEN = 7200 # length of LED strips (centainches)
HALF_BASE = 3600 # halfway along the LED strip (centainches)
SENSOR_DIST = 7550  # distance between sensors
HEIGHT_LEN  = 6550 # height of the sensor over the LEDs (centainches)
IN_CORNER = 1200 # any sensor distance closer than this indicates the object is 

# constant at compile time, taking care to promote to 32-bit
s2 = SENSOR_DIST*SENSOR_DIST
stwo = SENSOR_DIST*2

# check for edge 


# pick a node
for( myNodeID in 20:22 ) {
	#myNodeID = 21

	# indexing constrains
	# C: minIndex = 0
	# R:
	minIndex = 1
	maxIndex = minIndex+2

	# get indexes
	mI = myNodeID - 20 + minIndex
	# left index
	left = function(i) { ifelse(i>minIndex, i-1, maxIndex) }
	lI = left(mI)
	# right index
	right = function(i) { ifelse(i<maxIndex, i+1, minIndex) }
	rI = right(mI)

	# store the distances
	D = locDist

	# fixed point x^s operation.
	# promote the operand to 32-bit first; return 32-bit
	squared = function(x) { x*x }

	# altitude base
	altitudeBase = function(dist1, dist2) {
		((s2+squared(dist1))-squared(dist2))/stwo
	}
	Ab = c(0,0,0)
	Ab[lI] = floor( altitudeBase(D[mI],D[rI]) )
	Ab[mI] = floor( altitudeBase(D[rI], D[lI]) )
	Ab[rI] = floor( altitudeBase(D[lI],D[mI]) )
	if( !all(Ab == ab.soln) ) stop("altitude base failed.")

	# altitude height
	altitudeHeight = function(hyp, height) {
		if( height > hyp ) return(0)
		sqrt(squared(hyp) - squared(height))
	}
	Ah = c(0,0,0)
	Ah[lI] = floor( altitudeHeight(D[mI], Ab[lI]) )
	Ah[mI] = floor( altitudeHeight(D[rI], Ab[mI]) )
	Ah[rI] = floor( altitudeHeight(D[lI], Ab[rI]) )
	
	# use Vivani's theorem to adjust sum of heights to total altitude
	deltaAh = round( (HEIGHT_LEN - (Ah[lI]+Ah[mI]+Ah[rI]))/2 )
	Ah[lI] = ifelse(Ah[lI]==0, deltaAh, Ah[lI])
	Ah[mI] = ifelse(Ah[mI]==0, deltaAh, Ah[mI])
	Ah[rI] = ifelse(Ah[rI]==0, deltaAh, Ah[rI])
	if( !all(Ah == ah.soln) ) stop("altitude height failed.")

	# collinear base
	collinearBase = function(leftAh, rightAh) {
		(leftAh*SENSOR_DIST)/(leftAh+rightAh)
	}
	mCb = round( collinearBase(Ah[lI], Ah[rI]) )
	if( mCb != cb.soln[mI] ) stop("collinear base failed.")

	# collinear height
	collinearHeight = function(aHeight, colBase, altBase) {
		if( colBase>=altBase ) {
			return( sqrt( squared(aHeight) + squared(colBase-altBase) ) )
		} else {
			return( sqrt( squared(aHeight) + squared(altBase-colBase) ) )
		}
	}
	mCh = round( collinearHeight(Ah[mI], mCb, Ab[mI]) )
	if( mCh != ch.soln[mI] ) stop("collinear height failed.")

	# area
	area = function(aHeight, aHeight1, aHeight2) {
		aHeight/(aHeight+aHeight1+aHeight2)
	}
	mArea = area(Ah[mI], Ah[lI], Ah[rI])
	if( mArea != bl.soln[mI] ) stop("barycentric area failed.")
}