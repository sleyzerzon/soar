#
#  Waterjug puzzle
#
#-----------------------------------------------------------------------
#
# Tcl/Tk implementation of the waterjug puzzle.
#
#	last update: 5.26.00 Acar Altinsel
#

package require Soar

proc drawscreen {w} {
	catch {destroy $w}
	toplevel $w
	
	wm title $w "Water Jugs Demonstration"
	wm iconname $w "Water Jugs"
	message $w.msg -font -Adobe-times-medium-r-normal--*-180* \
            	-aspect 400 -text "Water Jugs Demonstration"
	frame $w.frame -width 480 -height 360 -borderwidth 4 \
	    	-relief ridge -bg Grey70

	button $w.solve -text Solve -command "run"
	button $w.stop -text "Stop" -command "stop-soar"
	button $w.init -text "Init Soar" -command "init-soar; picture 0 0 state"
	button $w.exch -text "Excise Chunks" -command "excise -chunks"
	button $w.ok -text Close -command "dismiss $w"
       
	pack $w.msg -side top
	pack $w.frame -side top -padx 20
	pack $w.solve $w.stop $w.init $w.exch -side left -padx 1m -pady 2m
	pack $w.ok -side right -padx 1m -pady 2m 	

	picture 0 0 state		
}

proc dismiss {w} {
    excise -all
    init-soar
    destroy $w
}

proc timer {pic} {
	global solveslow
	catch {destroy .water.frame.picture}
	image create photo test -file $pic
	label .water.frame.picture -image test -height 360 -width 480
	pack .water.frame.picture
	set m 1
	after 100 set m 0
	vwait m
}

proc picture {vol3 vol5 action} {
global soar_library

switch $action {
	3pour5 	{
		for {set i $vol5} {$i <= 5} {incr i} {			
			if {$vol3 >=0} {
				set filename "[file join $soar_library .. demos waterjug images $action-$vol3$vol5.gif]"
				timer $filename
				set vol3 [expr $vol3 - 1]
				incr vol5

			}
		}

	}
	5pour3 {
		for {set i $vol3} {$i <= 3} {incr i} {
			if {$vol5 >= 0} {
				set filename "[file join $soar_library .. demos waterjug images $action-$vol3$vol5.gif]"
				timer $filename
				set vol5 [expr $vol5 - 1]
				incr vol3
			}
		}

	}
	dump3  {
		for {set i $vol3} {$i >= 0} {set i [expr $i - 1]} {
			set filename "[file join $soar_library .. demos waterjug images $action-$vol3$vol5.gif]"
			timer $filename
			set vol3 [expr $vol3 - 1]
			}

		}
	dump5  {

		for {set i $vol5} {$i >= 0} {set i [expr $i - 1]} {
			set filename "[file join $soar_library .. demos waterjug images $action-$vol3$vol5.gif]"
			timer $filename
			set vol5 [expr $vol5 - 1]
			}


		}
	fill3 {
		set filename "[file join $soar_library .. demos waterjug images $action-$vol5.gif]"
		timer $filename


		}
	fill5 {
		set filename "[file join $soar_library .. demos waterjug images $action-$vol3.gif]"
		timer $filename
		}
	state  {

		set filename "[file join $soar_library .. demos waterjug images $vol3$vol5.gif]"
		timer $filename
		}
	}
}

#-----------------------------------------------------------------------
#
# Initialization and GUI creation

# Load rules defining waterjug solution method

global soar_library
global interp_type

excise -all
source waterjug.soar

drawscreen .water

###
### WATERJUG: 
### MONITOR STATE AND
### OPERATORS
###

sp {waterjug*monitor*state 
    (state <s> ^jug <i> <j>)
    (<i> ^volume 3 ^contents <icon>)
    (<j> ^volume 5 ^contents <jcon>)
    --> 
    (tcl |picture | <icon> | | <jcon> | state|)
    (write (crlf)
           | 3:| <icon> 
           | 5:| <jcon> )}

sp {waterjug*monitor*operator-application*empty
    (state <s> ^operator <o>)
    (<o> ^name empty
    	 ^jug <i>)
    (<i> ^volume <volume> ^contents <contents>)

    (<s> ^jug <x> <y>)
    (<x> ^volume 3 ^contents <xcon>)
    (<y> ^volume 5 ^contents <ycon>)


    -->
    (tcl | picture | <xcon> | | <ycon> | dump| <volume>)
    (write (crlf) 
    	   |  EMPTY(| <volume> 
    	   |:| <contents> 
    	   |)|)}

sp {waterjug*monitor*operator-application*fill
    (state <s> ^operator <o>)
    (<o> ^name fill 
    	 ^jug <i>)
    (<i> ^volume <volume> ^contents <contents>)

    (<s> ^jug <x> <y>)
    (<x> ^volume 3 ^contents <xcon>)
    (<y> ^volume 5 ^contents <ycon>)
    -->
    (tcl | picture | <xcon> | | <ycon> | fill| <volume>)
    (write (crlf) 
    	   |  FILL(| <volume> 
    	   |:| <contents> 
    	   |)|)}

sp {waterjug*monitor*operator-application*pour
    (state <s> ^operator <o>)
    (<o> ^name pour 
    	 ^jug <i>
    	 ^into <j>)
    (<i> ^volume <ivol> ^contents <icon>)
    (<j> ^volume <jvol> ^contents <jcon>)

    (<s> ^jug <x> <y>)
    (<x> ^volume 3 ^contents <xcon>)
    (<y> ^volume 5 ^contents <ycon>)
    -->
    (tcl | picture | <xcon> | | <ycon> | | <ivol> |pour| <jvol>)
    (write (crlf) 
    	   |  POUR(| <ivol> 
    	   |:| <icon> 
    	   |,| <jvol>
    	   |:| <jcon>
    	   |)|)}

sp {waterjug*monitor*tied-operator*empty
    (state <s> ^impasse tie 
               ^attribute operator 
               ^item <item>)
    (<item> ^name empty ^jug <i>)
    (<i> ^volume <volume> ^contents <contents>)

    (<s> ^jug <x> <y>)
    (<x> ^volume 3 ^contents <xcon>)
    (<y> ^volume 5 ^contents <ycon>)

    -->
    (tcl | picture | <xcon> | | <ycon> | dump| <volume>)
    (write (crlf) 
    	   |  | <item> 
    	   |: empty(| <volume> 
    	   |:| <contents> 
    	   |)|)}

sp {waterjug*monitor*tied-operator*fill
    (state <s> ^impasse tie 
               ^attribute operator 
               ^item <item>)
    (<item> ^name fill ^jug <i>)
    (<i> ^volume <volume> ^contents <contents>)

    (<s> ^jug <x> <y>)
    (<x> ^volume 3 ^contents <xcon>)
    (<y> ^volume 5 ^contents <ycon>)
    -->
    (tcl | picture | <xcon> | | <ycon> | fill| <volume>)
    (write (crlf) 
    	   |  | <item> 
    	   |: fill(| <volume> 
    	   |:| <contents> 
    	   |)|)}

sp {waterjug*monitor*tied-operator*pour
    (state <s> ^impasse tie 
               ^attribute operator 
               ^item <item>)
    (<item> ^name pour ^jug <i> ^into <j>)
    (<i> ^volume <ivol> ^contents <icon>)
    (<j> ^volume <jvol> ^contents <jcon>)

    (<s> ^jug <x> <y>)
    (<x> ^volume 3 ^contents <xcon>)
    (<y> ^volume 5 ^contents <ycon>)
    -->
    (tcl | picture | <xcon> | | <ycon> | | <ivol> |pour| <jvol>)
    (write (crlf) 
    	   |  | <item> 
    	   |: pour(| <ivol> 
    	   |:| <icon> 
    	   |,| <jvol>
    	   |:| <jcon>
    	   |)|)}

### eof of waterjug.soar (Version Type: Soar8)
