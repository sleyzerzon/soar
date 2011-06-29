from __future__ import print_function
import sys, os, socket, time
import random, math
import numpy as num
import pymunk as munk
import Tkinter as tk

WHEEL_DIAMETER     = 0.25
BASELINE           = 0.35
TORQUE_CONSTANT    = 3.0   # torque (Nm) per amp
EMF_CONSTANT       = 2.0   # volts per rad_per_sec
WINDING_RESISTANCE = 5.5   # ohms
INERTIA            = 0.5   # kg*m^2
DRAG_CONSTANT      = 1.0   # drag (Nm per rad_per_sec) ( >= 0)
DT                 = 0.016 # need a better way to figure this out

MASS               = 1.0
LENGTH             = 0.5
SCALE              = 50.0

TERMSTRING         = '\n***\n'

nupdates = 0

def rotate(rot, v):
	halfroll = rot[0] / 2
	halfpitch = rot[1] / 2
	halfyaw = rot[2] / 2

	sin_r2 = math.sin(halfroll)
	sin_p2 = math.sin(halfpitch)
	sin_y2 = math.sin(halfyaw)

	cos_r2 = math.cos(halfroll)
	cos_p2 = math.cos(halfpitch)
	cos_y2 = math.cos(halfyaw)

	a = cos_r2 * cos_p2 * cos_y2 + sin_r2 * sin_p2 * sin_y2
	b = sin_r2 * cos_p2 * cos_y2 - cos_r2 * sin_p2 * sin_y2
	c = cos_r2 * sin_p2 * cos_y2 + sin_r2 * cos_p2 * sin_y2
	d = cos_r2 * cos_p2 * sin_y2 - sin_r2 * sin_p2 * cos_y2
	
	t2 = a*b;
	t3 = a*c;
	t4 = a*d;
	t5 = -b*b;
	t6 = b*c;
	t7 = b*d;
	t8 = -c*b;
	t9 = c*d;
	t10 = -d*d;

	r0 = 2*((t8+t10)*v[0] + (t6-t4)*v[1]  + (t3+t7)*v[2]) + v[0];
	r1 = 2*((t4+t6)*v[0]  + (t5+t10)*v[1] + (t9-t2)*v[2]) + v[1];
	r2 = 2*((t7-t3)*v[0]  + (t2+t9)*v[1]  + (t5+t8)*v[2]) + v[2];
	
	return num.array([r0, r1, r2])

def close(a, b, tol):
	t = abs(a * tol)
	return a - t <= b <= a + t
	
def calc_rps(rps, volts):
	volts_emf = rps * EMF_CONSTANT
	amps = (volts - volts_emf) / WINDING_RESISTANCE
	torque0 = amps * TORQUE_CONSTANT
	torque_drag = rps * DRAG_CONSTANT
	torque_net = torque0 - torque_drag
	acceleration = torque_net / INERTIA
	return rps + acceleration * DT

class Sock(object):
	def __init__(self):
		self.sock = None
		self.recvbuf = ''

	def connect(self, path_or_host, port=None):
		if port == None:
			self.sock = socket.socket(socket.AF_UNIX)
			print('waiting for domain socket {} to be created'.format(path_or_host), file=sys.stderr)
			while not os.path.exists(path_or_host):
				time.sleep(0.1)
				
			while True:
				try:
					self.sock.connect(path_or_host)
					break
				except socket.error:
					time.sleep(0.5)
			
			print('connected to {}'.format(path_or_host))
		else:
			self.sock = socket.socket(socket.AF_INET)
			self.sock.connect((path_or_host, port))
	
	def serve(self, path):
		listener = socket.socket(socket.AF_UNIX)
		if os.path.exists(path):
			os.unlink(path)
		listener.bind(path)
		listener.listen(1)
		self.sock, addr = listener.accept()
		
	def receive(self):
		while not TERMSTRING in self.recvbuf:
			r = self.sock.recv(1024)
			if len(r) == 0:
				return None
			self.recvbuf += r
		
		msg, self.recvbuf = self.recvbuf.split(TERMSTRING, 1)
		return msg.decode('ascii')
	
	def send(self, msg):
		self.sock.sendall(msg + TERMSTRING)

	def close(self):
		self.sock.close()

	def has_buffered(self):
		return TERMSTRING in self.recvbuf

class Display(object):
	def __init__(self, root):
		self.canvas = tk.Canvas(root)
		self.canvas.bind('<Button-1>', lambda evt: self.canvas.scan_mark(evt.x, evt.y))
		self.canvas.bind('<B1-Motion>', lambda evt: self.canvas.scan_dragto(evt.x, evt.y))
		self.canvas.create_oval((0,0,5,5))
		self.canvas.pack(fill=tk.BOTH, expand=True)
		self.splinter = None
		self.prevpos = None
	
	def update_splinter(self, pos, verts):
		p = (pos[0] * SCALE, pos[1] * SCALE)
		if self.splinter != None:
			self.canvas.delete(self.splinter)
			self.canvas.create_line(self.prevpos, p, fill='blue')
		
		sverts = [ (x * SCALE, y * SCALE) for x, y in verts ]
		self.splinter = self.canvas.create_polygon(sverts, fill='', outline='blue')
		self.prevpos = p

class Splinter(object):
	def __init__(self, space, display, sockfile):
		self.display = display
		self.sock = Sock()
		self.sock.connect(sockfile)
		self.display.canvas.tk.createfilehandler(self.sock.sock, tk.READABLE, self.read)
		
		self.space = space
		self.body = munk.Body(MASS, munk.moment_for_box(MASS, LENGTH, BASELINE))
		l2 = LENGTH / 2
		b2 = BASELINE / 2
		verts = [(-l2, -b2), (-l2, b2), (l2, b2), (l2, -b2)]
		self.shape = munk.Poly(self.body, verts)
		self.space.add(self.body, self.shape)

		self.lrps = 0.
		self.rrps = 0.
		self.lvolt = 0.
		self.rvolt = 0.
		
		self.update(True)
		
	def update(self, first=False):
		global nupdates
		nupdates += 1
		if nupdates < 0:
			self.lrps = calc_rps(self.lrps, self.lvolt * -12)
			self.rrps = calc_rps(self.rrps, self.rvolt * -12)
		else:
			self.lrps = calc_rps(self.lrps, self.lvolt * 12)
			self.rrps = calc_rps(self.rrps, self.rvolt * 12)
		
		dleft  = DT * self.lrps * WHEEL_DIAMETER;
		dright = DT * self.rrps * WHEEL_DIAMETER;
	
		rot = (0., 0., self.body.angle)
		self.body.velocity = munk.Vec2d(rotate(rot, num.array([(dleft + dright) / 2.0, 0.0, 0.0]))[:2])
		self.body.angular_velocity = (dright - dleft) / BASELINE

		self.space.step(1.0)
		self.display.update_splinter(self.body.position, self.shape.get_points())
		self.print_sgel(first)
		
		p = self.body.position
		dist = math.sqrt((5 - p[0]) ** 2 + (-5 - p[1]) ** 2)
		print('Distance', dist)
		sys.stdout.flush()
	
	def print_sgel(self, first):
		lines = []
		ps = ' '.join(str(x) for x in self.body.position) + ' 0'
		rs = '0 0 {}'.format(self.body.angle)
		vs = ' '.join('{} {} 0'.format(x, y) for x, y in self.shape.get_points())
		if first:
			lines.append('a splinter world v {} p {} r {}'.format(vs, ps, rs))
		else:
			lines.append('c splinter p {} r {}'.format(ps, rs))
			
		lines.append('p splinter left_rads_per_sec {}'.format(self.lrps))
		lines.append('p splinter right_rads_per_sec {}'.format(self.rrps))
		for d in range(2):
			lines.append('p splinter vel_{} {}'.format(d, self.body.velocity[d]))
		lines.append('p splinter vel_2 0.0')
		lines.append('p splinter rotation_rate_0 0.')
		lines.append('p splinter rotation_rate_1 0.')
		lines.append('p splinter rotation_rate_2 {}'.format(self.body.angular_velocity))
		self.sock.send('\n'.join(lines))
	
	def read(self, f, m):
		msg = self.sock.receive()
		if msg == None:
			return
			
		for line in msg.split('\n'):
			if len(line) == 0:
				continue
				
			f = line.split()
			if len(f) != 2:
				print('Ignoring input "{}"'.format(line), file=sys.stderr)
			else:
				if f[0] == 'left':
					self.lvolt = float(f[1])
				elif f[0] == 'right':
					self.rvolt = float(f[1])
		
		self.update()
			
if __name__ == '__main__':
	disp = Display(tk.Tk())
	splinter = Splinter(munk.Space(), disp, 'env')
	tk.mainloop()
	