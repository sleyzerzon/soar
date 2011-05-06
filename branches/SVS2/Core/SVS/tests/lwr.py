#/usr/bin/env python2

from __future__ import print_function
import os, sys
import numpy as np
import heapq
import cPickle

K = 20
KP = -3.0

class LWR:
	def __init__(self, xdim, ydim, nnbrs):
		self.xs = []
		self.ys = []
		self.xarray = np.zeros((0,0))
		self.yarray = np.zeros((0,0))
		self.xdim = xdim
		self.ydim = ydim
		self.nnbrs = nnbrs
	
	def update(self, x, y):
		if len(x) != self.xdim or len(y) != self.ydim:
			print('dim mismatch', file=sys.stderr)
			return
			
		self.xs.append(x)
		self.ys.append(y)
	
	def choose_nearest(self, x):
		k = min(self.nnbrs, self.xarray.shape[0])
		distsq = np.power(self.xarray - x, 2).sum(axis=1)
		inds = heapq.nsmallest(k, range(len(distsq)), key=lambda i: distsq[i])
		#print(np.min(distsq[inds]), np.mean(distsq[inds]), np.max(distsq[inds]), end=' ')
		return inds, distsq[inds]
	
	def predict(self, x):
		if len(self.xs) == 0:
			return None
		
		if self.xarray.shape[0] != len(self.xs):
			self.xarray = np.vstack(self.xs)
			self.yarray = np.vstack(self.ys)

		inds, dists = self.choose_nearest(x)
		
		w = np.power(dists, KP) # apply kernel to dists
		
		# these points are going to cause trouble for regression, hack around it
		isclose = np.logical_or(np.equal(dists, 0), np.isinf(w))
		if np.any(isclose):
			closeinds = [ i for i, v in enumerate(isclose) if v ]
			return np.sum(self.yarray[closeinds], axis=0) / float(len(closeinds))
		
		X = np.hstack((self.xarray[inds], np.ones((len(inds), 1))))
		
		yt = self.yarray[inds]
		changed = [ i for i in range(yt.shape[1]) if np.any(yt[:,i]-yt[0,i]) ]
		y = yt[:,changed]
		W = np.diag(w) 
		Z = np.dot(W, X)
		v = np.dot(W, y)
		C = np.linalg.lstsq(Z, v)[0].transpose()
		x1 = np.append(x, 1)
		p = np.dot(C, x1)
		
		p1 = x[0:len(yt[0])].copy()
		for i1, i2 in enumerate(changed):
			p1[i2] = p[i1]
		return p1
