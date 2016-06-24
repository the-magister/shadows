### libraries
require(ggplot2)
require(ggthemes)
require(dplyr)
require(reshape2)

# these need to match Network.h definitions
BASE_LEN = 7200 # length of LED strips (centainches)
HALF_BASE = 3600 # halfway along the LED strip (centainches)
SENSOR_DIST = 7550  # distance between sensors
HEIGHT_LEN  = 6550 # height of the sensor over the LEDs (centainches)
IN_CORNER = 1200 # any sensor distance closer than this indicates the object is 

# thetas
to.rad = function(theta) { theta*pi/180 }

vertices = data_frame(
	name = c("S10","S11","S12"),
	o.name = c("L20","L21","L22"),
	x0 = c(SENSOR_DIST/2, 0, SENSOR_DIST),
	y0 = c(0, HEIGHT_LEN, HEIGHT_LEN),
	ang.right = c(to.rad(60), to.rad(360-60), to.rad(180)),
	ang.cen = ang.right+to.rad(30),
	ang.left = ang.cen+to.rad(30)
)

edges = data_frame(
	name = c("L20","L21","L22"),
	o.name = c("S10","S11","S12"),
	x0 = c(0, SENSOR_DIST, SENSOR_DIST/2),
	y0 = c(HEIGHT_LEN, HEIGHT_LEN, 0),
	x1 = c(SENSOR_DIST, SENSOR_DIST/2, 0),
	y1 = c(HEIGHT_LEN, 0, HEIGHT_LEN)
	)

base.plot = ggplot() +
	aes(color=name) +
	geom_segment(data=edges, size=2, aes(x=x0,y=y0,xend=x1,yend=y1)) +
	geom_text(data=vertices, aes(x=x0,y=y0,label=name)) +
	geom_point(data=vertices, size=5, aes(x=x0,y=y0), shape=17) +
	coord_equal() +
	scale_color_colorblind() +
	labs(x="Horizontal, in*100",y="Vertical, in*100", color="Object")
print(base.plot)

random.target = function() {
	ang = runif(1,vertices$ang.right[1],vertices$ang.left[1])
	dis = runif(1,0,SENSOR_DIST)

	x = vertices$x0[1] + round(cos(ang)*dis)
	y = vertices$y0[1] + round(sin(ang)*dis)
	y = ifelse(y>HEIGHT_LEN, HEIGHT_LEN, y)
	
	loc = data_frame(xl=x, yl=y)
	d = vertices %>%
		mutate( d=round(sqrt((x0-x)^2+(y0-y)^2)) )
			
	list(
		loc=loc,
		d=d
	)
}


t = random.target()
d.true = t$d
d = t$d %>%
	mutate( d=ifelse(d>HEIGHT_LEN, HEIGHT_LEN, d) )


loc = t$loc
loc.plot = base.plot + 
	geom_spoke(data=d, aes(x=x0,y=y0,radius=d,angle=ang.right+to.rad(2)),size=1) +
	geom_spoke(data=d, aes(x=x0,y=y0,radius=d,angle=ang.right+to.rad(15)),size=1) +
	geom_spoke(data=d, aes(x=x0,y=y0,radius=d,angle=ang.right+to.rad(30)),size=1) +
	geom_spoke(data=d, aes(x=x0,y=y0,radius=d,angle=ang.right+to.rad(45)),size=1) +
	geom_spoke(data=d, aes(x=x0,y=y0,radius=d,angle=ang.right+to.rad(60-2)),size=1) +
	geom_point(data=loc, aes(x=xl, y=yl, color="Target"), shape=9, size=5)
print(loc.plot)

###
# notation options
notationType = factor(1:4,
	labels=c("distance","barycentric","trilinear","sideline")
)
# message structure. always sent in distance notation, as that's the native notation for measurements
locDist = d$d



### notational change functions
asSideline = function(locDist) {
	# from https://en.wikipedia.org/wiki/Heron%27s_formula#Algebraic_proof_using_the_Pythagorean_theorem
	ss = SENSOR_DIST*SENSOR_DIST
	s2 = SENSOR_DIST*2
	aa=locDist[1]*locDist[1]
	bb=locDist[2]*locDist[2]
	cc=locDist[3]*locDist[3]
	c(
		# a
		floor(SENSOR_DIST - ((ss+cc)-(bb))/s2),
		# b
		floor(SENSOR_DIST - ((ss+aa)-(cc))/s2),
		# c
		floor(SENSOR_DIST - ((ss+bb)-(aa))/s2)
	)
}
asSideline(locDist)

# https://en.wikipedia.org/wiki/Trilinear_coordinates
asTrilinear = function(locDist) {
	# from https://en.wikipedia.org/wiki/Heron%27s_formula#Algebraic_proof_using_the_Pythagorean_theorem
	d = SENSOR_DIST - asSideline(locDist)
	dd = d*d
	aa=locDist[1]*locDist[1]
	bb=locDist[2]*locDist[2]
	cc=locDist[3]*locDist[3]
	c(
		# note: we return actual distances (a',b',c') not x:y:z
		# a'
		floor(sqrt(cc-dd[1])),
		# b' 
		floor(sqrt(aa-dd[2])),
		# c'
		floor(sqrt(bb-dd[3]))
	)
}
tl=asTrilinear(locDist)
sum(tl)

# https://en.wikipedia.org/wiki/Barycentric_coordinate_system
asBarycentric = function(locDist) {
	# could use lib8tion's fract16
	locTril = asTrilinear(locDist)
	sumLoc = locTril[1]+locTril[2]+locTril[3]
	locTril/sumLoc
}
bl=asBarycentric(locDist)
sum(bl)

# https://en.wikipedia.org/wiki/Cevian
# http://mathworld.wolfram.com/TrilinearCoordinates.html
asCevian = function(locDist) {
	# object location
	locBary = asBarycentric(locDist)
	# vertex locations
	verBary = c(0,0,1)
	# colinearity projection to edge
	# edgeBary = c(0,k1,1-k1)
	
	sumLoc = locTril[1]+locTril[2]+locTril[3]
	locTril/sumLoc
}
bl=asBarycentric(locDist)
sum(bl)



asBarycentric = function(locDist) {

	# A is (1,0,0) or (HEIGHT_LEN,0,0) in real units
	# B is (0,1,0) or (0,HEIGHT_LEN,0) in real units
	# C is (0,0,1) or (0,0,HEIGHT_LEN) in real units
	# we prefer real units to [0,1] to avoid floating point representation

}




### work from distances to back-calculate location
soln = d 

# 0. pick a sensor to be.
# assume I'm S10.
mI = 1
lI = ifelse(mI-1<=0, 3, mI-1)
rI = ifelse(mI+1>=4, 1, mI+1)

x.soln = NULL
y.soln = NULL

# 1. is this sensor OOR?  If so, pass, as I can't add any meaningful contribution to this calculation.
if( soln$d[mI] >= HEIGHT_LEN ) {
	# send the message as-is, with our distance information logged
	return;
}

# 2. find x,y
if( soln$d[rI] < HEIGHT_LEN ) {
	# is the sensor on the right in range, as we'd prefer that more recent ranging information.
	
	c = SENSOR_DIST
	b = soln$d[mI]
	a = soln$d[rI]
	
	# find the distance, d, cutting the SENSOR_DIST below the altitude, h.
	d = round((-a^2+b^2+c^2)/(2*c))
	x.d = soln$x0[mI] + round(cos(soln$ang.right[mI])*d)
	y.d = soln$y0[mI] + round(sin(soln$ang.right[mI])*d)
	loc.plot+annotate(geom="point",x=x.d,y=y.d,size=5)
	
	# find the height, h, above the triangle
	h = round(sqrt(b^2-d^2))
	
	# get a solution
	x.soln = x.d + round(cos(soln$ang.right[mI]+to.rad(90))*h)
	y.soln = y.d + round(sin(soln$ang.right[mI]+to.rad(90))*h)
	loc.plot+annotate(geom="point",x=x.soln,y=y.soln,size=5)
} else if ( soln$d[lI] < HEIGHT_LEN ) {
	# if not, then we can use the one the left, but it's older.
	c = SENSOR_DIST
	b = soln$d[mI]
	a = soln$d[lI]
	
	# find the distance, d, cutting the SENSOR_DIST below the altitude, h.
	d = round((-a^2+b^2+c^2)/(2*c))
	x.d = soln$x0[mI] + round(cos(soln$ang.left[mI])*d)
	y.d = soln$y0[mI] + round(sin(soln$ang.left[mI])*d)
	loc.plot+annotate(geom="point",x=x.d,y=y.d,size=5)
	
	# find the height, h, above the triangle
	h = round(sqrt(b^2-d^2))
	
	# get a solution
	x.soln = x.d + round(cos(soln$ang.left[mI]-to.rad(90))*h)
	y.soln = y.d + round(sin(soln$ang.left[mI]-to.rad(90))*h)
	loc.plot+annotate(geom="point",x=x.soln,y=y.soln,size=5)
} else {
	# otherwise, this is a "corner case", literally.  The object is deep in my corner
	# we can't determine what angle the object is at, so assume it's right down the middle
	x.soln = soln$x0[mI] + round(cos(soln$ang.cen[mI])*soln$d[mI])
	y.soln = soln$y0[mI] + round(sin(soln$ang.cen[mI])*soln$d[mI])
	loc.plot+annotate(geom="point",x=x.soln,y=y.soln,size=5)
}

# 3. update the intercept and range information for each edge
ang = atan( (x.soln - soln$x0[mI])/(y.soln - soln$y0[mI]) ) + soln$ang.cen[mI]

# we want to use the two longest measurements that are most recent to make the location assessment
soln = soln %>%
	mutate(
		OOR = d >= HEIGHT_LEN
	)

# assume I'm S10.
myIndex = 1
lI = soln$lI[myIndex]
aI = soln$aI[myIndex]
rI = soln$rI[myIndex]


%>%
	mutate(
		inter = round((-d[rI]^2+d[lI]^2+SENSOR_DIST^2)/(2*SENSOR_DIST)),
		height = round(sqrt(d[lI]^2-inter^2)),
		ang = 
	)

s = select(soln, "name"=o.name, inter, height, ang.right) %>%
	left_join(edges, by="name") %>%
#	left_join(select(vertices, "name"=o.name, ang), by="name") %>%
	mutate(
		x=x0+cos(ang.right)*inter,
		y=y0+sin(ang.right)*inter
	)
	
soln.plot = loc.plot +
	geom_point(data=s, aes(x=x,y=y), size=6, shape=1, show.legend=F) +
	geom_spoke(data=soln, aes(x=x,y=y,radius=height,angle=ang.right-to.rad(90)), size=2, arrow=arrow(ends="both"), show.legend=F)
print(soln.plot)


# we know three relationships between distance and location:
#  dN^2=(xN-x)^2+(yN-y)^2
# so:
# xN = 
# adjust the distances to meet Vivani's theorem
adjustDistances = function(msg) {
	sumD = msg$d[1]+msg$d[2]+msg$d[3]
	diffD = HEIGHT_LEN - sumD


# detect a corner case
haveOutOfRange = function(msg) {
	OOR = 0
	OOR = ifelse(msg$d[1]>=HEIGHT_LEN, OOR+1, OOR)
	OOR = ifelse(msg$d[2]>=HEIGHT_LEN, OOR+1, OOR)
	OOR = ifelse(msg$d[3]>=HEIGHT_LEN, OOR+1, OOR)
	return( OOR )
}

cornerCase = function(msg, aI, rI, lI) {
    # across
    msg$inter[aI+1] = SENSOR_DIST/2;
    msg$range[aI+1] = HEIGHT_LEN - IN_CORNER;
    # to the right
    msg$inter[rI+1] = IN_CORNER/2;
    msg$range[rI+1] = IN_CORNER/2;
    # to the left
    msg$inter[lI+1] = SENSOR_DIST - IN_CORNER/2;
    msg$range[lI+1] = IN_CORNER/2;
}


intercept = function(lR, rR) {
  
  # see https://en.m.wikipedia.org/wiki/Heron%27s_formula "Algebraic proof using the Pythagorean theorem"
  cSq = SENSOR_DIST * SENSOR_DIST;
  cTwo = SENSOR_DIST * 2;

  lSq = lR * lR;
  rSq = rR * rR;

  d = round(((lSq + cSq)-rSq) / cTwo);

  d = ifelse(d>SENSOR_DIST, SENSOR_DIST, d)
 
  return( d ); 
}

calculatePosition = function(msg) {

  # corner cases... literally.
  if( msg$d[0+1]<=IN_CORNER ) {
    cornerCase(msg, 0, 1, 2);
    return(msg);
  }
  if( msg$d[1+1]<=IN_CORNER ) {
    cornerCase(msg, 1, 2, 0);
    return(msg);
  }
  if( msg$d[2+1]<=IN_CORNER ) {
    cornerCase(msg, 2, 0, 1);
    return(msg);
  }

  # use left and right sensors to determine intercept at base
  msg$inter[0+1] = intercept(msg$d[2+1], msg$d[1+1]);
  msg$inter[1+1] = intercept(msg$d[0+1], msg$d[2+1]);
  msg$inter[2+1] = intercept(msg$d[1+1], msg$d[0+1]);

   # use the across sensor to approximate height
#  for( byte i=0; i<3; i++ ) {
#    if( msg.d[i] < HEIGHT_LEN ) {
#      msg.range[i] = HEIGHT_LEN - msg.d[i];
#    } else {
#      msg.range[i] = 0;
#    }
#  }
  msg$range = ifelse(msg$d<HEIGHT_LEN, HEIGHT_LEN - msg$d, 0)

  return(msg)
}

msg = calculatePosition(d)

d = select(msg, name, x0, y0, ang, d)
s = select(msg, "name"=o.name, inter, range) %>%
	left_join(edges, by="name") %>%
#	left_join(select(vertices, "name"=o.name, ang), by="name") %>%
	mutate(
		x=x0+cos(ang)*inter,
		y=y0+sin(ang)*inter
	)

loc.plot + 
	geom_point(data=s, aes(x=x,y=y), size=6, shape=1, show.legend=F) +
	geom_spoke(data=s, aes(x=x,y=y,radius=range,angle=ang-to.rad(90)), size=2, arrow=arrow(ends="both"), show.legend=F)


	
a=msg$d[1]
b=msg$d[2]
c=msg$d[3]

K = a+b+c