### get an example
source('notation.R')

# previous calculation to compare back to.
ab.soln = ab
ah.soln = ah
cb.soln = cb
ch.soln = ch
bl.soln = bl

# constant at compile time, taking care to promote to 32-bit
s2 = SENSOR_DIST*SENSOR_DIST
stwo = SENSOR_DIST*2

# indexing constrains
# C: minIndex = 0
# R:
minIndex = 1
maxIndex = minIndex+2

# get indexes
# left index
left = function(i) { ifelse(i>minIndex, i-1, maxIndex) }
# right index
right = function(i) { ifelse(i<maxIndex, i+1, minIndex) }

# store the distances
D = locDist

# fixed point x^s operation.
# promote the operand to 32-bit first; return 32-bit
squared = function(x) { x*x }
squareRoot = function(x) { round(sqrt(x)) }

# altitude base
altitudeBase = function(i) {
	floor( 
		((s2+squared(D[right(i)]))-squared(D[left(i)]))/stwo 
	)
}
Ab = c(0,0,0)
Ab[1] = altitudeBase(1)
Ab[2] = altitudeBase(2)
Ab[3] = altitudeBase(3)
if( !all(Ab == ab.soln) ) stop("altitude base failed.")

# altitude height
altitudeHeight = function(i) {
	if( Ab[i] > D[right(i)] ) return(0)
	squareRoot(squared(D[right(i)]) - squared(Ab[i]))
}
Ah = c(0,0,0)
Ah[1] = altitudeHeight(1)
Ah[2] = altitudeHeight(2)
Ah[3] = altitudeHeight(3)
	
# use Vivani's theorem to adjust sum of heights to total altitude
deltaAh = round( (HEIGHT_LEN - (Ah[1]+Ah[2]+Ah[3]))/2 )
Ah[1] = ifelse(Ah[1]==0, deltaAh, Ah[1])
Ah[2] = ifelse(Ah[2]==0, deltaAh, Ah[2])
Ah[3] = ifelse(Ah[3]==0, deltaAh, Ah[3])
if( !all(Ah == ah.soln) ) stop("altitude height failed.")

# collinear base
collinearBase = function(i) {
	floor(
		(Ah[left(i)]*SENSOR_DIST)/(Ah[left(i)]+Ah[right(i)])
	)
}
Cb = c(0,0,0)
Cb[1] = collinearBase(1)
Cb[2] = collinearBase(2)
Cb[3] = collinearBase(3)
if( !all(Cb == cb.soln) ) stop("collinear base failed.")

# collinear height
collinearHeight = function(i) {
	if( Cb[i]>=Ab[i] ) {
		return( squareRoot( squared(Ah[i]) + squared(Cb[i]-Ab[i]) ) )
	} else {
		return( squareRoot( squared(Ah[i]) + squared(Ab[i]-Cb[i]) ) )
	}
}
Ch = c(0,0,0)
Ch[1] = collinearHeight(1)
Ch[2] = collinearHeight(2)
Ch[3] = collinearHeight(3)
if( !all(Ch == ch.soln) ) stop("collinear height failed.")

# area
baryloc = function(i) {
	Ah[i]/(Ah[1]+Ah[2]+Ah[3])
}
bary = c(0,0,0)
bary[1] = baryloc(1)
bary[2] = baryloc(2)
bary[3] = baryloc(3)
if( !all(signif(bary,3) == signif(bl.soln,3)) ) stop("barycentric area failed.")

area = function(i) {
	floor(
		(2^16-1)*Ah[i]/SENSOR_DIST
	)
}
Area = c(0,0,0)
Area[1] = area(1)
Area[2] = area(2)
Area[3] = area(3)
if( !all(signif(Area/sum(Area),3) == signif(bl.soln,3)) ) stop("area failed.")
