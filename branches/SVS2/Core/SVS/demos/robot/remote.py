import sys, os
import Tkinter as tk

sys.path.append('../')
import sock

sck = sock.Sock()
sck.connect('ctrl')

wrcount = 0
currcmd = (0.0, 0.0)

def handlekey(evt):
	global currcmd
	
	currcmd = {
		'w' : (1.0, 1.0),
		'a' : (-1.0, 1.0),
		'r' : (-1.0, -1.0),
		's' : (1.0, -1.0)
	}.get(evt.char, (0.0, 0.0))

def handlemsg(file, mask):
	global wrcount
	print(wrcount)
	wrcount += 1
	sck.receive()
	sck.send('left {}\nright {}'.format(*currcmd))
	
win = tk.Tk()
win.bind('<Key>', handlekey)
win.tk.createfilehandler(sck.sock, tk.READABLE, handlemsg)
tk.mainloop()
