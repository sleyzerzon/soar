package edu.umich.soar.gridmap2d;

import java.awt.event.ActionEvent;

public class StepAction extends AbstractGridmap2DAction {

	private static final long serialVersionUID = 1812359437366074252L;

	public StepAction(ActionManager manager) {
		super(manager, "Step");
	}

	@Override
	public void update() {
//        setEnabled(!getApplication().getAgent().isRunning());
	}

	@Override
	public void actionPerformed(ActionEvent e) {
	}

}
