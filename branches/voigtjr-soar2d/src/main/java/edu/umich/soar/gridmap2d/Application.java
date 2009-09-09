package edu.umich.soar.gridmap2d;

import java.awt.BorderLayout;
import java.awt.GraphicsEnvironment;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.List;
import java.util.prefs.Preferences;

import javax.swing.JFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JPanel;
import javax.swing.JToolBar;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.UnsupportedLookAndFeelException;

import org.flexdock.docking.DockingManager;
import org.flexdock.view.Viewport;

import edu.umich.soar.gridmap2d.config.SimConfig;
import edu.umich.soar.gridmap2d.core.CognitiveArchitecture;
import edu.umich.soar.gridmap2d.core.Simulation;
import edu.umich.soar.gridmap2d.soar.Soar;

public class Application extends JPanel implements Adaptable {
	private static final long serialVersionUID = 4313201967156814057L;

	public static final Preferences PREFERENCES = Preferences.userNodeForPackage(Application.class);

	public static void main(final String[] args) {
		try {
			final SimConfig config = SimConfig.getInstance(args[0]);
			final Simulation sim = new Simulation();
			
			if (GraphicsEnvironment.isHeadless() || config.generalConfig().headless) {
				sim.initialize(config);
				sim.run();
			} else {
				try {
					// Use the look and feel of the system we're running on rather
					// than Java. If an error occurs, we proceed normally using
					// whatever L&F we get.
					UIManager.setLookAndFeel(UIManager
							.getSystemLookAndFeelClassName());
				} catch (UnsupportedLookAndFeelException e) {
				} catch (ClassNotFoundException e) {
				} catch (InstantiationException e) {
				} catch (IllegalAccessException e) {
				}
				
				SwingUtilities.invokeLater(new Runnable() {
					
					public void run() {
						initialize(args, sim, config);
					}
				});
			}
		} catch (FileNotFoundException e) {
			e.printStackTrace();
			return;
		}
	}
	
	public static Application initialize(String[] args, Simulation sim, SimConfig config) {
		DockingManager.setFloatingEnabled(true);
		
		Application app = new Application(sim, config);
		final JFrame frame = new JFrame();
		frame.setContentPane(app);
		
		app.initialize(frame);
		frame.setVisible(true);
		return app;
	}

	private JFrame frame;
    private final Viewport viewport = new Viewport();
	private final ActionManager actionManager = new ActionManager(this);
    private final List<AbstractAdaptableView> views = new ArrayList<AbstractAdaptableView>();
    private final Simulation sim;
    private final SimConfig config;

    private Application(Simulation sim, SimConfig config) {
    	super(new BorderLayout());
    	
		this.sim = sim;
		this.config = config;
    }
    
    private void initialize(JFrame frame) {
		this.frame = frame;
		
		frame.setSize(600, 600);
		frame.setTitle("Gridmap2D");
		
		initToolbar();

		add(viewport, BorderLayout.CENTER);

		initActions();

		initViews();
		initMenuBar();
	}
	
    private void initToolbar()
    {
        JToolBar bar = new JToolBar();
        bar.setFloatable(false);
        
        bar.add(new RunControlPanel(this));
        
        add(bar, BorderLayout.NORTH);
    }
    
	private void initActions() {
		new ExitAction(actionManager);
	}

	private void initMenuBar() {
		JMenuBar menubar = new JMenuBar();
		JMenu fileMenu = new JMenu("File");
		fileMenu.add(actionManager.getAction(ExitAction.class));
		menubar.add(fileMenu);
		frame.setJMenuBar(menubar);
	}
	
	private void initViews() {
		final WorldView worldView = addView(new WorldView(this));
		viewport.dock(worldView);
	}

    private <T extends AbstractAdaptableView> T addView(T view)
    {
        views.add(view);
        return view;
    }
    
	void exit() {
		System.exit(0);
	}

	@Override
	public Object getAdapter(Class<?> klass) {
		assert false;
		return null;
	}
	
    public void updateActionsAndStatus()
    {
        if(SwingUtilities.isEventDispatchThread())
        {
            actionManager.updateActions();
            //status.refresh(false);
        }
        else
        {
            SwingUtilities.invokeLater(new Runnable() { public void run() {
                updateActionsAndStatus();
            } });
        }
    }
    
    public Simulation getSim() {
    	return sim;
    }
}
