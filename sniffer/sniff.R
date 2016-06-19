### libraries
require(ggplot2)
require(ggthemes)
require(dplyr)
require(reshape2)
require(serial)
require(tcltk)

txt = 'Network. MSG from 12\td 0=3460 1=2430 2=1860\tp 0=3500- 1200|\tp 1=3300- 900|\tp 2=3600- 1400|';

# these need to match Network.h definitions
BASE_LEN = 7200 # length of LED strips (centainches)
HALF_BASE = 3600 # halfway along the LED strip (centainches)
SENSOR_DIST = 7550  # distance between sensors
HEIGHT_LEN  = 6550 # height of the sensor over the LEDs (centainches)

# stand height
stand.y = 2400

# thetas
to.rad = function(theta) { theta*pi/180 }

vertices = data_frame(
	name = c("S10","S12","S11"),
	x0 = c(0, +SENSOR_DIST/2, -SENSOR_DIST/2),
	y0 = stand.y+c(0, +HEIGHT_LEN, +HEIGHT_LEN),
	ang = c(to.rad(60), to.rad(180), to.rad(300)),
	o.name = c("L20","L22","L21")
)

edges = data_frame(
	name = c("L20","L21","L22"),
	x0 = c(-SENSOR_DIST/2, +SENSOR_DIST/2, 0),
	x1 = c(+SENSOR_DIST/2, 0, -SENSOR_DIST/2),
	y0 = stand.y+c(+HEIGHT_LEN, +HEIGHT_LEN, 0),
	y1 = stand.y+c(+HEIGHT_LEN, 0, +HEIGHT_LEN),
	ang = c(to.rad(0), to.rad(180+60), to.rad(120))
)

base.plot = ggplot() +
	aes(color=name) +
	geom_segment(data=edges, size=2, aes(x=x0,y=y0,xend=x1,yend=y1)) +
	geom_point(data=vertices, size=5, aes(x=x0,y=y0), shape=17) +
	coord_equal() +
	scale_color_colorblind() +
	labs(x="Horizontal, in*100",y="Vertical, in*100", color="Object")
print(base.plot)

parse.txt = function(txt) {
	msg = strsplit(txt, '\t')[[1]]
	
	dist = as.numeric(substr(strsplit(msg[2]," ")[[1]][2:4],3,999))
	L20 = as.numeric(strsplit(substr(msg[3],5,nchar(msg[3])-1),'-')[[1]])
	L21 = as.numeric(strsplit(substr(msg[4],5,nchar(msg[4])-1),'-')[[1]])
	L22 = as.numeric(strsplit(substr(msg[5],5,nchar(msg[5])-1),'-')[[1]])

	list(
		d = data_frame(name=vertices$name, d=dist),
		s = data_frame(name=edges$name, inter=c(L20[1],L21[1],L22[1]), range=c(L20[2],L21[2],L22[2]))
	)
}


serial.open = function(com) {
	serial.close()
	open = try( .Tcl(paste0('set R_com [open "',com,'" r+]')), silent=T ) # open channel
	if( class(open) != 'try-error' ) {
		conf = .Tcl('fconfigure $R_com -mode "115200,n,8,1" -buffering none -blocking 0')
		serial.isOpen <<- TRUE
	} else {
		serial.isOpen <<- FALSE
		return( open[[1]] )
	}

}

serial.close = function() {
	close = try( .Tcl(paste0('close $R_com')), silent=T ) # close channel
}

serial.read = function(maxWaitForResponse=0.25) {
	tic = Sys.time()
	ret = ""
	if( !serial.isOpen ) return("(no serial opened)")
	keepReading = T
	while( keepReading ) {
		line = tclvalue(.Tcl('gets $R_com')) 
		if( line != "" ) {
			ret = c(ret, line)
		} else if( Sys.time()-tic>maxWaitForResponse ) {
			keepReading=F
		}
	}
	return(paste0(ret, collapse="\n"))
}

serial.send = function(send, maxWaitForResponse=0.25) {
	if( !serial.isOpen ) return( "(no Serial opened)" )
	
	drop = serial.read(0) # dump anything in the serial buffer
	
	send = .Tcl(paste0('puts -nonewline $R_com {',send,'}')) 

	ret = paste0("", serial.read(maxWaitForResponse))
	return( ret )
}


msg=parse.txt(txt)

d = msg$d %>%
	left_join(vertices, by="name")  
	
s = msg$s %>%
	left_join(edges, by="name") %>%
#	left_join(select(vertices, "name"=o.name, ang), by="name") %>%
	mutate(
		x=x0+cos(ang)*inter,
		y=y0+sin(ang)*inter
	)

base.plot + 
	geom_spoke(data=d, aes(x=x0,y=y0,radius=d,angle=ang+to.rad(2)),size=1) +
	geom_spoke(data=d, aes(x=x0,y=y0,radius=d,angle=ang+to.rad(15)),size=1) +
	geom_spoke(data=d, aes(x=x0,y=y0,radius=d,angle=ang+to.rad(30)),size=1) +
	geom_spoke(data=d, aes(x=x0,y=y0,radius=d,angle=ang+to.rad(45)),size=1) +
	geom_spoke(data=d, aes(x=x0,y=y0,radius=d,angle=ang+to.rad(60-2)),size=1) +
	geom_point(data=s, aes(x=x,y=y), size=6, shape=1, show.legend=F) +
	geom_spoke(data=s, aes(x=x,y=y,radius=range,angle=ang-to.rad(90)), size=2, arrow=arrow(ends="both"), show.legend=F)

