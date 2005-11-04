#!/usr/bin/perl
#
# Generate distribution file
# Author: Jonathan Voigt voigtjr@gmail.com
# Date: November 2005
# 

use strict;
use File::Path;

if ($#ARGV != 5) {
  print "Usage: sf_user dev dist version tag\n";
  print "  sf_user   username on sourceforge.net\n";
  print "  dev       full path to development installation\n";
  print "  dist      full path to destination for distribution, contents overwritten!\n";
  print "            dist path will be destroyed! rm -rf dist!\n";
  print "  version   foo-bar produces Soar-Suite-foo-bar\n";
  print "  tag       cvs tag to export.  HEAD is for latest\n";
  print "\n";
  print "I do development in /home/voigtjr/soar/dev, so I would pass that for dev\n";
  print "I'd like the dist files out of that same soar directory, so I would pass\n";
  print "/home/voigtjr/soar/dist for dist.\n";
  exit 0;
}

my $sf_user = $ARGV[0];	# sourceforge username
my $dev     = $ARGV[1]; # dev directory
my $dist    = $ARGV[2]; # dist directory
my $version = $ARGV[3]; # version string
my $tag     = $ARGV[4]; # cvs revision

my @modules =
(
  "SoarKernel",
  "gSKI",
  "SoarIO",
  "soar-library",
  "SoarJavaDebugger",
  "JavaMissionaries",
  "JavaTOH",
  "JavaBaseEnvironment",
  "JavaTanksoar",
  "JavaEaters",
);

my @distfiles = 
(
  "Makefile.am",
  "build-everything.pl",
  "configure.ac",
  "install-sh",
  "missing",
);

# Flush dir
rmtree "$dist/Soar-Suite-$version";
mkdir "$dist/Soar-Suite-$version" or die "Could not create new package dir: $dist/Soar-Suite-$version";
chdir "$dist/Soar-Suite-$version" or die "Could not change to package dir: $dist/Soar-Suite-$version";

# Check out modules
foreach $module (@modules) {
  my $ret = system "cvs -Q -d /cvsroot/soar export -r $tag $module";
  if ($ret != 0) {
    die "Error: cvs returned error for module $module";
  }
}

my $ret = system "cvs -Q -d :ext:$sf_user\@cvs.sourceforge.net:/cvsroot/soar export -r $tag visualsoar";
if ($ret != 0) {
  die "Error: cvs returned error for module $module";
}

# Move dist files to top level
mv ${PACKAGE_ROOT}/SoarIO/dist/configure.ac ${PACKAGE_ROOT}
mv ${PACKAGE_ROOT}/SoarIO/dist/install-sh ${PACKAGE_ROOT}
mv ${PACKAGE_ROOT}/SoarIO/dist/Makefile.am ${PACKAGE_ROOT}
mv ${PACKAGE_ROOT}/SoarIO/dist/build-everything.sh ${PACKAGE_ROOT}
mv ${PACKAGE_ROOT}/SoarIO/dist/missing ${PACKAGE_ROOT}
rm -rf ${PACKAGE_ROOT}/SoarIO/dist

# Copy top level docs
cp $PACKAGE_ROOT/SoarIO/COPYING $DIST_ROOT/Soar-Suite-8.6.1/license.txt
cp $PACKAGE_ROOT/Documentation/announce.txt $DIST_ROOT/Soar-Suite-8.6.1

# Initialize configure scripts (this recurses)
cd ${PACKAGE_ROOT}; autoreconf 2>/dev/null

# Add binaries and data
cp ${DEV_ROOT}/soar-library/swt.jar ${PACKAGE_ROOT}/soar-library
mkdir ${PACKAGE_ROOT}/soar-library/mac
cp ${DEV_ROOT}/soar-library/mac/* ${PACKAGE_ROOT}/soar-library/mac

# Remove unwanted files
rm -rf ${PACKAGE_ROOT}/soar-library/*.dll
rm -rf ${PACKAGE_ROOT}/soar-library/java_swt ${PACKAGE_ROOT}/soar-library/libswt-carbon-* ${PACKAGE_ROOT}/soar-library/libswt-pi-carbon-* ${PACKAGE_ROOT}/soar-library/libswt-webkit-carbon-* ${PACKAGE_ROOT}/soar-library/swt-carbon.jar ${PACKAGE_ROOT}/soar-library/swt-pi-carbon.jar

# Create the tarball
cd ${DIST_ROOT}
tar cfj Soar-Suite-8.6.1.tar.bz2 Soar-Suite-8.6.1
