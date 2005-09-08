/* File: ControlPanel.java
 * Jul 15, 2004
 */

package edu.umich.JavaBaseEnvironment;

import org.eclipse.swt.widgets.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.*;
import java.io.*;

import org.eclipse.swt.layout.*;

/**
 * The <code>ControlPanel</code> is the GUI interface to a Soar simulation through
 * which a user can load and destroy agents, run and stop and step the simulation,
 * quit the simulation, and load and send maps messages to be visible.
 * @author John Duchi
 */
public class ControlPanel implements SimulationControlListener{

	private Display myDisplay;
	private Shell myShell;
	private SimulationControl myEC;
	private File myLatest;
	private String myTask;
	private String myTaskNoun;
	private char myTaskFirstChar;
	private Menu eatersDrop;
	private String currentAgentPath = "Click \"Load Agent\" to select productions";
	private String currentColorName;

	private boolean amOpen = false;
	
	private Button createAgentButton;
	private Button createHumanAgentButton;
	private Button destroyAgentButton;
	private Button colorButton;
	private Button runSimulationButton;
	private Button stopSimulationButton;
	private Button stepSimulationButton;
	private Button quitSimulationButton;
	
	private MenuItem showMinimap;
	
	private Slider speedSlider;
	public static final int MaxSpeed = 19;
	private int currentSpeed = MaxSpeed;
	private SWindowManager myManager;
  //FIXME //TODO remove this to a child class - will cause eaters to use same def path
  private String defaultMapsPath = "tanksoar" + File.separator + "maps"; // this is where we expect maps to be located
  private String defaultAgentsPath = "tanksoar" + File.separator + "agents"; // this is where we expect agents to be located
  
  protected String stankEquiv = null;
  protected String stankEquivDescr = null;
	
    public ControlPanel(SimulationControl ec, String task, String taskNoun, Display display, String ext, String extDes)
    {
    	this(ec, task, taskNoun, display);
    	stankEquiv = ext;
    	stankEquivDescr = extDes;
    }
  
	public ControlPanel(SimulationControl ec, String task, String taskNoun, Display display){
		if(task == null || task.length() == 0) task = "?";
		if(taskNoun == null || taskNoun.length() == 0) taskNoun = task + "s";
		myTask = task;
		myTaskNoun = taskNoun;
		myEC = ec;
		myEC.addSimulationControlListener(this);
		myTaskFirstChar = myTask.toLowerCase().charAt(0);
		myDisplay = display;
	}
	
	public ControlPanel(SimulationControl ec, Display display){
		this(ec, null, null, display);
	}
  
  public String getDefaultMapsPath(){ return defaultMapsPath;}
	
	public void open(){
		if(myShell == null || myShell.isDisposed()){
			amOpen = true;
			myDisplay.asyncExec(new Runnable(){
				public void run(){
					initDisplay();
				}
			});
		}
	}
	
	public boolean isOpen(){
		return (amOpen);//myShell != null && !myShell.isDisposed());
	}
	
	private void initDisplay(){
		myShell = new Shell(myDisplay);
		setMenus();
		setButtons();
		myShell.setText(myTask + " control");
		myShell.addShellListener(new ShellAdapter(){
			public void shellClosed(ShellEvent e){
				if (myEC.isQuittable())
				{
					MessageBox mb = new MessageBox(myShell, SWT.YES | SWT.NO | SWT.ICON_QUESTION);
					mb.setText("Quit " + myTaskNoun);
					mb.setMessage("Sure you want to quit " + myTaskNoun);
					e.doit = (mb.open() == SWT.YES);
					if(e.doit){
						amOpen = false;
						myEC.quitSimulation();
					}
				}
				else
				{
					MessageBox mb = new MessageBox(myShell, SWT.ICON_INFORMATION);
					mb.setText("Simulation is running.");
					mb.setMessage("You must stop the simulation before you may quit.");
					mb.open();
					e.doit = false;
				}
			}
		});
		myShell.pack();
		myShell.open();
	}
	
	private void setMenus(){
		Menu top = new Menu(myShell, SWT.BAR);
		myShell.setMenuBar(top);
		
		/* File menu */
		Menu fileDrop = new Menu(myShell, SWT.DROP_DOWN);
		MenuItem file = new MenuItem(top, SWT.CASCADE);
		file.setText("File");
		file.setMenu(fileDrop);
		MenuItem exit = new MenuItem(fileDrop, SWT.PUSH);
		exit.setText("Exit");
		exit.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				myShell.close();
				//myEC.quitSimulation();
			}
		});
		
		/* Eaters Menu */
		eatersDrop = new Menu(myShell, SWT.DROP_DOWN);
		MenuItem task = new MenuItem(top, SWT.CASCADE);
		task.setMenu(eatersDrop);
		task.setText(myTaskNoun);
		
		eatersDrop.addMenuListener(new MenuAdapter(){
			public void menuShown(MenuEvent e){
				addAgentMenus();
			}
		});
		
		/* Map Menu */
		Menu mapDrop = new Menu(myShell, SWT.DROP_DOWN);
		MenuItem map = new MenuItem(top, SWT.CASCADE);
		map.setMenu(mapDrop);
		map.setText("Maps");
		MenuItem resetMaps = new MenuItem(mapDrop, SWT.PUSH);
		resetMaps.setText("Reset map");
		resetMaps.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				myEC.resetMap();
			}
		});
		MenuItem randomMap = new MenuItem(mapDrop, SWT.PUSH);
		randomMap.setText("Random map");
		randomMap.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				myEC.newRandomMap();
			}
		});
		MenuItem loadMap = new MenuItem(mapDrop, SWT.PUSH);
		loadMap.setText("Load map");
		loadMap.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				FileDialog fd = new FileDialog(myShell, SWT.OPEN);
				fd.setFilterExtensions(new String[] {"*." + myTaskFirstChar + "map", "*.*"});
        fd.setFilterPath(defaultMapsPath);

				fd.setFilterNames(new String[] {myTask + "s maps (*." + myTaskFirstChar + "map)", "All files (*.*)"});
				String path = fd.open();
				if(path != null && !path.equals("")){
					myEC.loadMap(new File(path));
				}
			}
		});
		
		/* Window Menu */
		Menu windowDrop = new Menu(myShell, SWT.DROP_DOWN); 
		MenuItem windowItem = new MenuItem(top, SWT.CASCADE);
		windowItem.setMenu(windowDrop);
		windowItem.setText("Window");
		MenuItem showMap = new MenuItem(windowDrop, SWT.PUSH);
		showMap.setText("Show " + myTask + "s map");
		showMap.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				fireOpenMapNotification();
			}
		});

		showMinimap = new MenuItem(windowDrop, SWT.CASCADE);
		final Menu minimaps = new Menu(showMinimap);
		showMinimap.setText("Show " + myTask + " mini-map");
		showMinimap.setMenu(minimaps);
		minimaps.addMenuListener(new MenuAdapter(){
			public void menuShown(MenuEvent e){
				addAgentWindowMenus(minimaps);
			}
		});
	}
	
	private void setButtons(){
		
		myShell.setLayout(new RowLayout(SWT.HORIZONTAL));
		Composite left = new Composite(myShell, SWT.NONE);
		Composite right = new Composite(myShell, SWT.NONE);
		
		/* Load buttons */
		left.setLayout(new RowLayout(SWT.VERTICAL));
		((RowLayout)left.getLayout()).pack = false;
		final Label l = new Label(left, SWT.NONE);
		l.setText(currentAgentPath);
		l.setToolTipText("Displays the name of agent currently selected to load on \'Create\'");
		Button b = new Button(left, SWT.PUSH);
		b.setText("Load Agent");
		b.setToolTipText("Select file to be used to load agent");
		b.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				FileDialog fd = new FileDialog(myShell, SWT.OPEN);
				if (stankEquiv != null)
				{
					fd.setFilterExtensions(new String[] {"*.soar", stankEquiv, "*.*"});
					fd.setFilterNames(new String[] {"Soar Agents (*.soar)", stankEquivDescr, "All files (*.*)"});
				}
				else
				{
					fd.setFilterExtensions(new String[] {"*.soar", "*.*"});
					fd.setFilterNames(new String[] {"Soar Agents (*.soar)", "All files (*.*)"});
				}
				fd.setFilterPath(defaultAgentsPath);
				String oldPath = currentAgentPath;
				currentAgentPath = fd.open();
				if(currentAgentPath != null && !currentAgentPath.equals("")){
					l.setText(new File(currentAgentPath).getName());
				} else {
					currentAgentPath = oldPath;
				}
				UpdateButtonEnables();
			}
		});
		
		createAgentButton = new Button(left, SWT.PUSH);
		colorButton = new Button(left, SWT.PUSH);
		
		createAgentButton.setText("Create Agent");
		createAgentButton.setToolTipText("Create an agent with path specified and of color"
				+ "\nshown in box below. Random color assigned if none chosen.");
		createAgentButton.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				SoarAgent sa = myEC.loadAgent(new File(currentAgentPath), currentColorName);
				if(sa != null){
					fireOpenAgentViewNotification(sa);
				}
				String[] ss = myEC.getColorsAvailable();
				if(ss.length > 0){
					colorButton.setText(ss[0]);
					currentColorName = ss[0];
				} else {
					colorButton.setText("*None*");
					colorButton.setEnabled(false);
				}
			}
		});
		
		final Menu m = new Menu(myShell, SWT.CASCADE);
		colorButton.setText("Colors");
		colorButton.setToolTipText("Select a color for next agent created.\n" 
				+ "If none selected, random color will be assigned");
		colorButton.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				m.setVisible(true);
			}
		});
		m.addMenuListener(new MenuAdapter(){
			public void menuShown(MenuEvent e){
				MenuItem[] mis = m.getItems();
				for(int i = 0; i < mis.length; i++){
					mis[i].dispose();
				}
				String[] ss = myEC.getColorsAvailable();
				if(ss.length == 0){
					MenuItem mi = new MenuItem(m, SWT.PUSH);
					mi.setText("*None*");
				}
				for(int i = 0; i < ss.length; i++){
					final MenuItem mi = new MenuItem(m, SWT.PUSH);
					mi.setText(ss[i]);
					mi.addSelectionListener(new SelectionAdapter(){
						public void widgetSelected(SelectionEvent e){
							currentColorName = mi.getText();
							colorButton.setText(currentColorName);
						}
					});
				}
			}
		});
		destroyAgentButton = new Button(left, SWT.PUSH);
		final Menu m2 = new Menu(myShell, SWT.CASCADE);
		destroyAgentButton.setText("Destroy Agent");
		destroyAgentButton.setToolTipText("Destroys agent of color selected in drop down menu");
		destroyAgentButton.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				m2.setVisible(true);
			}
		});
		m2.addMenuListener(new MenuAdapter(){
			public void menuShown(MenuEvent e){
				removeMenuItems(m2);
				String[] ss = myEC.getColorsUsed();
				if(ss.length == 0){
					MenuItem mi = new MenuItem(m2, SWT.PUSH);
					mi.setText("*None*");
				}
				for(int i = 0; i < ss.length; i++){
					final MenuItem mi = new MenuItem(m2, SWT.PUSH);
					mi.setText(ss[i]);
					mi.addSelectionListener(new SelectionAdapter(){
						public void widgetSelected(SelectionEvent e){
							if(myEC.canDestroyAgent()){
								myEC.destroyAgent(mi.getText());
							}
						}
					});
				}
			}
		});
		
		createHumanAgentButton = new Button(left, SWT.PUSH);
		createHumanAgentButton.setText("Create Human Agent");
		createHumanAgentButton.setToolTipText("Creates a human-controlled agent for " + myTaskNoun);
		createHumanAgentButton.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				if(!myEC.isRunning()){
					myManager.createHumanAgent(currentColorName);
					String[] ss = myEC.getColorsAvailable();
					if(ss.length > 0){
						colorButton.setText(ss[0]);
						currentColorName = ss[0];
					} else {
						colorButton.setText("*None*");
						colorButton.setEnabled(false);
					}
				}
			}
		});
		
		Label lab = new Label(left, SWT.NONE);
		lab.setText("Speed");
		lab.setToolTipText("Slider controls speed of simulation.\nWill slow down between steps");
		lab.setAlignment(SWT.CENTER);
		speedSlider = new Slider(left, SWT.HORIZONTAL);
		speedSlider.setMaximum(MaxSpeed + 1);
		speedSlider.setMinimum(0);
		speedSlider.setIncrement(1);
		speedSlider.setThumb(1);
		speedSlider.setSelection(currentSpeed);
		speedSlider.setToolTipText("Controls speed of simulation.\nWill slow down between steps");
		speedSlider.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				currentSpeed = speedSlider.getSelection();
			}
		});
		
		/* RIGHT SIDE OF CONTROL PANEL */
		
		right.setLayout(new RowLayout(SWT.VERTICAL));
		((RowLayout)right.getLayout()).pack = false;
		b = new Button(right, SWT.PUSH);
		b.setText("Step");
		b.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				if(myEC.isRunning()){
					myEC.stopSimulation();
				}
				new Thread(){
					public void run(){
						myEC.singleStep();
					}
				}.start();
			}
		});
		stepSimulationButton = b;
		
		b = new Button(right, SWT.PUSH);
		b.setText("Run");
		b.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				if(!myEC.isRunning()){
					new Thread(){
						public void run(){
							myEC.runSimulation();
						}
					}.start();
				}
			}
		});
		runSimulationButton = b;
		
		b = new Button(right, SWT.PUSH);
		b.setText("Stop");
		b.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				if(myEC.isRunning()){
					myEC.stopSimulation();
				}
			}
		});
		stopSimulationButton = b;
		
		b = new Button(right, SWT.PUSH);
		b.setText("Quit");
		b.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				myShell.close();
			}
		});
		quitSimulationButton = b;
		
		UpdateButtonEnables();
	}
	
	public void setManager(SWindowManager m){
		myManager = m;
	}
	
	private void removeMenuItems(final Menu m){
		MenuItem[] mis = m.getItems();
		for(int i = 0; i < mis.length; i++){
			mis[i].dispose();
		}
	}
	
	private void addAgentWindowMenus(Menu minimaps){
		MenuItem[] mis = minimaps.getItems();
		for(int i = 0; i < mis.length; i++){
			mis[i].dispose();
		}
		String[] ss = myEC.getColorsUsed();
		if(ss == null) return;
		for(int i = 0; i < ss.length; i++){
			final MenuItem mi = new MenuItem(minimaps, SWT.PUSH);
			mi.setText(ss[i]);
			mi.addSelectionListener(new SelectionAdapter(){
				public void widgetSelected(SelectionEvent e){
					SoarAgent sa = findAgent(mi.getText());
					fireOpenAgentViewNotification(sa);
				}
			});
		}
	}
	
	/**
	 * Creates the menus for the agents drop down menu. These include methods to destroy 
	 * all agents, run til output generated, and to raise
	 * and open Soar command-line windows for the different agents.
	 * <p>><b>Note: All these menu items are not implemented. They have no effect.</b></p>
	 */
	private void addAgentMenus(){
		MenuItem[] mis = eatersDrop.getItems();
		for(int i = 0; i < mis.length; i++){
			mis[i].dispose();
		}
		MenuItem destroyAll = new MenuItem(eatersDrop, SWT.PUSH);
		destroyAll.setText("Destroy all " + myTask + "s agents");
		destroyAll.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				if(myEC.canDestroyAgent()){
					myEC.destroyAllAgents();
				}
			}
		});
		//TODO Set up the run til output generated
		/*
		MenuItem waitOutput = new MenuItem(eatersDrop, SWT.PUSH);
		waitOutput.setText("Run til output generated");
		*/
		
		final String[] ecs = myEC.getColorsUsed();
		if(ecs == null) return;
		for(int i = 0; i < ecs.length; i++){
//TODO Attach the excise productions to something.
			MenuItem mi = new MenuItem(eatersDrop, SWT.CASCADE);
			mi.setText(ecs[i]);
			Menu men = new Menu(mi);
			mi.setMenu(men);
			//MenuItem showWindow = new MenuItem(men, SWT.PUSH);
			//MenuItem raiseWindow = new MenuItem(men, SWT.PUSH);
			//MenuItem reload = new MenuItem(men, SWT.PUSH);
			//MenuItem reset = new MenuItem(men, SWT.PUSH);
			MenuItem exciseAll = new MenuItem(men, SWT.PUSH);
			MenuItem exciseChunks = new MenuItem(men, SWT.PUSH);
			//showWindow.setText("Show " + myTask + " command-line window");
//TODO These menu items all do nothing right now.
			//raiseWindow.setText("Raise " + myTask + " command-line window");
			//reload.setText("Reload productions");
			//reset.setText("Reset " + myTask);
			exciseAll.setText("Excise all productions");
			exciseChunks.setText("Excise chunks");
			{
				final int j = i;
				exciseAll.addSelectionListener(new SelectionAdapter() {
					private String myColor = ecs[j];
					public void widgetSelected(SelectionEvent e){
						myEC.exciseAll(myColor);
					}
				});
				
				exciseChunks.addSelectionListener(new SelectionAdapter() {
					private String myColor = ecs[j];
					public void widgetSelected(SelectionEvent e){
						myEC.exciseChunks(myColor);
					}
				});
				
				
			}
		}
	}
	
	/**
	 * Gives the speed to which the speed slider has been set. The speed is in a range
	 * from <code>[0, MaxSpeed]</code>, with 0 being the result when the slider is
	 * farthest to the left, <code>MaxSpeed</code> the result when the slider is farthest
	 * to the right.
	 * @return The <code>int</code> value of the current speed setting.
	 * @see ControlPanel#MaxSpeed
	 */
	public int getSpeed(){
		return(MaxSpeed - currentSpeed);
	}
	
	private SoarAgent findAgent(String colorName){
		SoarAgent[] ags = myEC.getAllAgents();
		for(int i = 0; i < ags.length; i++){
			if(ags[i].getColorName().equals(colorName)) return (ags[i]);
		}
		return null;
	}
	
	private void fireOpenMapNotification(){
		if(myManager == null) return;
		myManager.openMap();
		
		/*if(sWindowListeners == null) return;
		Iterator iter = sWindowListeners.iterator();
		while(iter.hasNext()){
			((SWindowListener)iter.next()).openMap();
		}*/
	}
	
	private void fireOpenAgentViewNotification(SoarAgent sa){
		if(myManager == null) return;
		myManager.openAgentView(sa);

		/*if(sWindowListeners == null) return;
		Iterator iter = sWindowListeners.iterator();
		while(iter.hasNext()){
			((SWindowListener)iter.next()).openAgentView(sa);
		}*/
	}
	
	private void fireCloseAgentViewNotification(SoarAgent sa){
		if(myManager == null) return;
		myManager.closeAgentView(sa);
		
		/*if(sWindowListeners == null) return;
		Iterator iter = sWindowListeners.iterator();
		while(iter.hasNext()){
			((SWindowListener)iter.next()).closeAgentView(sa);
		}*/
	}
		
	public void agentDestroyed(final SoarAgent destroyed){
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				colorButton.setEnabled(true);
				colorButton.setText(destroyed.getColorName());
				createAgentButton.setEnabled(true);
				UpdateButtonEnables();
			}
		});
	}
	
	public void agentCreated(SoarAgent created){
		if(myEC.getColorsAvailable() == null || myEC.getColorsAvailable().length == 0){
			myDisplay.asyncExec(new Runnable(){
				public void run(){
					createAgentButton.setEnabled(false);
				}
			});
		}
		UpdateButtonEnables();
	}
	
	public void simStarted(){
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				UpdateButtonEnables();
			}
		});
	}
	
	public void simQuit(){
		UpdateButtonEnables();
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				if(myShell != null && !myShell.isDisposed()){
					amOpen = false;
					myShell.close();
				}
			}
		});
	}
	
	public void simEnded(String message){
		try{
			myDisplay.syncExec(new Runnable(){
				public void run(){
					if(createAgentButton.isDisposed()) return;
					UpdateButtonEnables();
				}
			});
		}catch(SWTException displayDisposedIgnored){}
	}
	
	public void worldCountChanged(int newWorldCount){
		//Later
	}
	
	public void locationChanged(Location loc){

	}
	
	public void newMap(String message){
		//Later
	}
	
	public void UpdateButtonEnables()
	{
		runSimulationButton.setEnabled(myEC.isRunnable());
		stopSimulationButton.setEnabled(myEC.isStoppable());
		stepSimulationButton.setEnabled(myEC.isSteppable());
		quitSimulationButton.setEnabled(myEC.isQuittable());
		
		createAgentButton.setEnabled(currentAgentPath != "Click \"Load Agent\" to select productions" && !myEC.isRunning());
		destroyAgentButton.setEnabled(myEC.getAllAgents().length != 0 && !myEC.isRunning());
		createHumanAgentButton.setEnabled(!myEC.isRunning());
		showMinimap.setEnabled(myEC.getAllAgents().length != 0);
	}
	
}
