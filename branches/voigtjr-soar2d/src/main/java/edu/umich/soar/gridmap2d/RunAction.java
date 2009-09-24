package edu.umich.soar.gridmap2d;

import java.awt.event.ActionEvent;

public class RunAction extends AbstractGridmap2DAction {

	private static final long serialVersionUID = -3347861376925708892L;

	public RunAction(ActionManager manager) {
		super(manager, "Run");
	}

	@Override
	public void update() {
        setEnabled(!getApplication().getSim().isRunning());
	}

	@Override
	public void actionPerformed(ActionEvent e) {
		getApplication().doRunForever();
	}

}
