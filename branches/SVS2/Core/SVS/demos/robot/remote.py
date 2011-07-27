import sys, os
import Tkinter as tk

sys.path.append('../')
import sock

sck = sock.Sock()
sck.connect('ctrl')

currcmd = (0.0, 0.0)

def handle_key(evt):
	global currcmd
	
	currcmd = {
		'w' : (1.0, 1.0),
		'a' : (-.5, .5),
		'r' : (-1.0, -1.0),
		's' : (.5, -.5)
	}.get(evt.char, (0.0, 0.0))

def handle_release(evt):
	global currcmd
	currcmd = (0.0, 0.0)
	
def handle_msg(file, mask):
	sck.receive()
	sck.send('left {}\nright {}'.format(*currcmd))
	
win = tk.Tk()
win.bind('<KeyPress>', handle_key)
win.bind('<KeyRelease>', handle_release)
win.tk.createfilehandler(sck.sock, tk.READABLE, handle_msg)
tk.mainloop()
