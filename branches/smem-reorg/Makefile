all:
	cd Core && scons

checkout:
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/Core
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/Java
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/Python

checkout-filterc:
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/FilterC

checkout-javamissionaries:
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/JavaMissionaries

checkout-tohsml:
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/TOHSML

checkout-tcl:
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/Tcl

update:
	cd Core && svn update
	cd Java && svn update
	cd Python && svn update

clean:
	cd Core && scons -c

