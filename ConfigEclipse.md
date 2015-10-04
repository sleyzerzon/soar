# Setting up Eclipse #

If you want to build Java code that interfaces with Soar, the following instructions will help you set up Eclipse to do that.  This assumes that you've already installed Eclipse as per the build instructions for your particular platform.

  1. First, make sure all Soar code is completely built outside of Eclipse before starting Eclipse.
  1. Linux and Mac users might want to launch Eclipse from the command prompt so that it inherits environment variables such as SOAR\_HOME.


## Set Up Your Environment Variables ##

Variables help Eclipse remain platform independent, you tell it where to find external resources necessary for build.

  1. Open any project's _Properties_ and go to the _Java Build Path_ tab
  1. Click on _Libraries_ on the _Java Build Path_ page to show _Libraries_ settings
  1. Click _Add Variable_ to the right
  1. Click _Configure Variables_
  1. Press _New_ and create the variable `SML` and point it to where you installed `sml.jar`. This will be in your Soar home as defined in the build procedure. The default location is the `out` folder in the top level of your checkout, in a subfolder `share/java`.

Once this variable is defined, things should build.

## Subclipse ##

Having the Subclipse version control system installed is handy when doing development inside Eclipse.

  * Install Subclipse if you're using Eclipse by visiting the Help -> Install new software  dialog and adding the site `http://subclipse.tigris.org/update_1.6.x`.
    * _Important (Windows only):_ If targeting 64-bit, you may need to install (the 64-bit version of) SlikSVN to get Subclipse to work correctly, available at [sliksvn.com](http://www.sliksvn.com/en/download/). Eclipse may need to be restarted to see the SlikSVN libraries.

## Setting Up an Eclipse Project ##

  1. Run Eclipse
  1. Either use the default workspace or make a new one next to where you checked out Soar.
    * Do not have the folder where you checked out Soar be a child of your workspace folder.
  1. Import existing projects into your workspace without copying them in to the workspace folder. Do this by pointing the import wizard to the `SoarSuite` folder and importing all projects.