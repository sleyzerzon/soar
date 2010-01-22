all:
	cd Core && scons

checkout:
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/Core
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/Java
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/Python

update:
	cd Core && svn update
	cd Java && svn update
	cd Python && svn update

clean:
	cd Core && scons -c

