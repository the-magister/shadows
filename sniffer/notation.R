### libraries
rm(list=ls())
require(ggplot2)
require(ggthemes)
require(dplyr)
require(reshape2)


# these need to match Network.h definitions
BASE_LEN = 720 # length of LED strips (centainches)
HALF_BASE = 360 # halfway along the LED strip (centainches)
SENSOR_DIST = 755  # distance between sensors
HEIGHT_LEN  = 654 # height of the sensor over the LEDs (centainches)
IN_CORNER = 120 # any sensor distance closer than this indicates the object is 

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
	y1 = c(HEIGHT_LEN, 0, HEIGHT_LEN),
	ang = c(to.rad(0), to.rad(180+60), to.rad(120))
	)

base.plot = ggplot() +
	aes(color=name) +
	geom_segment(data=edges, size=1, aes(x=x0,y=y0,xend=x1,yend=y1)) +
	geom_text(data=vertices, aes(x=x0,y=y0,label=name,angle=ang.cen*360/2/pi), hjust=1.2) +
	geom_text(data=edges,aes(label=name,x=(x1+x0)/2,y=(y1+y0)/2,angle=ang*360/2/pi),vjust=-0.7) +
	geom_point(data=vertices, size=5, aes(x=x0,y=y0), shape=17) +
	coord_fixed(ylim=c(-50,SENSOR_DIST+50),xlim=c(-50,SENSOR_DIST+50),ratio=1) +
	scale_color_colorblind() +
	labs(x="Horizontal, in*10",y="Vertical, in*10", color="Object") +
	geom_spoke(data=vertices, aes(x=x0, y=y0, angle=ang.left), radius=50, color="black", arrow=arrow())

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
	geom_spoke(data=d, aes(x=x0,y=y0,radius=d,angle=ang.right+to.rad(2)),size=0.5,linetype="dashed") +
	geom_spoke(data=d, aes(x=x0,y=y0,radius=d,angle=ang.right+to.rad(15)),size=0.5,linetype="dashed") +
	geom_spoke(data=d, aes(x=x0,y=y0,radius=d,angle=ang.right+to.rad(30)),size=0.5,linetype="dashed") +
	geom_spoke(data=d, aes(x=x0,y=y0,radius=d,angle=ang.right+to.rad(45)),size=0.5,linetype="dashed") +
	geom_spoke(data=d, aes(x=x0,y=y0,radius=d,angle=ang.right+to.rad(60-2)),size=0.5,linetype="dashed") +
	geom_point(data=loc, aes(x=xl, y=yl, color="Target"), shape=9, size=5)
#print(loc.plot)

locDist = d$d
#locDist = c(6538, 6538, 6538) 

semiPerimiter = function(lI, rI) {
	floor( 
		( SENSOR_DIST + locDist[lI] + locDist[rI] ) / 2
	)
}

##  https://en.wikipedia.org/wiki/Trilinear_coordinates
## https://en.wikipedia.org/wiki/Altitude_(triangle)#Altitude_in_terms_of_the_sides
altitudeHeight = function(locDist) {
	
	a = SENSOR_DIST
	s = c(0,0,0)
	s[1] = semiPerimiter(2, 3)
	s[2] = semiPerimiter(3, 1)
	s[3] = semiPerimiter(1, 2)
	
	ret=c(
		# note: we return actual distances (a',b',c') not x:y:z
		# a'
		floor(2/a * round(sqrt(s[1]*(s[1]-locDist[2])))*round(sqrt((s[1]-a)*(s[1]-locDist[3])) ) ),
		# b' 
		floor(2/a * round(sqrt(s[2]*(s[2]-locDist[3])))*round(sqrt((s[2]-a)*(s[2]-locDist[1])) ) ),
		# c'
		floor(2/a * round(sqrt(s[3]*(s[3]-locDist[1])))*round(sqrt((s[3]-a)*(s[3]-locDist[2])) ) )
	)
	ret[is.na(ret)]=0
	return( ret )
}
ah=altitudeHeight(locDist)

# use Vivani's theorem to adjust sum of heights to total altitude
correctAltitudeHeight = function(ah) {

	# compute the difference in sum of heights from SL
	deltaAh = HEIGHT_LEN
	for(i in 1:3) deltaAh = deltaAh - ah[i]
	
#	print(ah)
#	print(deltaAh)
	
	# are we done?
	if( deltaAh == 0 ) return(ah)
	
	# what do we do with it?
	if( deltaAh<3 && deltaAh>-3 ) {
		# small amount, so finish it
		if( ah[1] < ah[2] && ah[1] < ah[3] ) { 
			ah[1] = ah[1] + deltaAh 
		} else if( ah[2] < ah[1] && ah[2] < ah[3] ) { 
			ah[2] = ah[2] + deltaAh 
		} else { 
			ah[3] = ah[3] + deltaAh
		}

		return(ah)
	} else {
		# dole that difference out equally.
		addTo = floor(deltaAh/3)
		for(i in 1:3) ah[i] = ah[i] + addTo
		
		# and do this again to grab remainder
		correctAltitudeHeight(ah)
	}
}
ah=correctAltitudeHeight(ah)

### notational change functions
altitudeBase = function(locDist) {
	# from https://en.wikipedia.org/wiki/Heron%27s_formula#Algebraic_proof_using_the_Pythagorean_theorem
	ah = correctAltitudeHeight(altitudeHeight(locDist))
#	ah = altitudeHeight(locDist)
	c(
		# a
		round( sqrt( locDist[2]^2 - ah[1]^2 ) ),
		# b
		round( sqrt( locDist[3]^2 - ah[2]^2 ) ),
		# c
		round( sqrt( locDist[1]^2 - ah[3]^2 ) ) 
	)
}
ab=altitudeBase(locDist)

soln.ab = edges %>%
	mutate(
		inter = ab,
		x.ab = x0 + round(cos(ang)*inter),
		y.ab = y0 + round(sin(ang)*inter)
	)

soln.plot = loc.plot +
	geom_point(data=soln.ab, aes(x=x.ab,y.ab), shape=1, size=4)
#print(soln.plot)

soln.ah = soln.ab %>%
	mutate(
		height = ah,
		x.ah = x.ab + round(cos(ang-to.rad(90))*height),
		y.ah = y.ab + round(sin(ang-to.rad(90))*height)
	)

soln.plot = soln.plot +
	geom_segment(data=soln.ah, aes(x=x.ab,y.ab,xend=x.ah,yend=y.ah))
#print(soln.plot)

# https://en.wikipedia.org/wiki/Barycentric_coordinate_system
asBarycentric = function(ah) {
	sumLoc = ah[1]+ah[2]+ah[3]
	ah/sumLoc
}
bl=asBarycentric(ah)
sum(bl)

soln.barycentric = vertices %>%
	mutate( 
		lambda = bl,
		xl = x0*lambda,
		yl = y0*lambda
	) %>%
	summarize(
		name = "Bary soln",
		x = round(sum(xl)),
		y = round(sum(yl))
	)

soln.plot = soln.plot +
	geom_point(data=soln.barycentric, aes(x=x,y=y), size=2)
#print(soln.plot)

# https://en.wikipedia.org/wiki/Cevian
# http://mathworld.wolfram.com/TrilinearCoordinates.html
collinearBase = function(ah) {
	c(
		# α'
		floor( (ah[3]*SENSOR_DIST)/(ah[3]+ah[2]) ),
		# ß'
		floor( (ah[1]*SENSOR_DIST)/(ah[1]+ah[3]) ),
		# Γ'
		floor( (ah[2]*SENSOR_DIST)/(ah[2]+ah[1]) )
	)
}
cb=collinearBase(ah)
sum(cb)

soln.cb = edges %>%
	mutate(
		inter = cb,
		x.cb = x0 + round(cos(ang)*inter),
		y.cb = y0 + round(sin(ang)*inter)
	)
soln.plot = soln.plot +
	geom_point(data=soln.cb, aes(x=x.cb,y=y.cb), shape=12, size=4)
#print(soln.plot)

# https://en.wikipedia.org/wiki/Cevian
# http://mathworld.wolfram.com/TrilinearCoordinates.html
collinearHeight = function(altHeight, colBase, altBase) {
	c(
		# aa
		round( sqrt( altHeight[1]^2 + (colBase[1] - altBase[1])^2 ) ),
		# bb
		round( sqrt( altHeight[2]^2 + (colBase[2] - altBase[2])^2 ) ),
		# cc
		round( sqrt( altHeight[3]^2 + (colBase[3] - altBase[3])^2 ) )
	)
}
ch=collinearHeight(ah, cb, ab)
sum(ch)

soln.ch = soln.cb %>%
	mutate(
		x.ch = soln.barycentric$x,
		y.ch = soln.barycentric$y
	)

soln.plot = soln.plot +
	geom_segment(data=soln.ch, aes(x=x.cb,y.cb,xend=x.ch,yend=y.ch))
#print(soln.plot)

soln.plot = soln.plot +
	with(soln.ch, annotate(geom="text", x=x.cb, y=y.cb, label="Cb", angle=360*ang/2/pi, vjust=-1)) +
	with(soln.ah, annotate(geom="text", x=x.ab, y=y.ab, label="Ab", angle=360*ang/2/pi, vjust=-1)) +
	with(soln.ch, annotate(geom="text", x=(x.cb+x.ch)/2, y=(y.cb+y.ch)/2, label="Ch", angle=360*ang/2/pi, vjust=-1)) +
	with(soln.ah, annotate(geom="text", x=(x.ab+x.ah)/2, y=(y.ab+y.ah)/2, label="Ah", angle=360*ang/2/pi, vjust=-1)) 
	
print(soln.plot)
ggsave("solution_plot.png",height=8,width=8,units="in",dpi=150)
