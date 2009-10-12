package edu.umich.soar.room;

import java.awt.event.ActionEvent;

public class GoAction extends AbstractGridmap2DAction {

	private static final long serialVersionUID = -3347861376925708892L;

	private GoProvider gop;
	
	public GoAction(ActionManager manager) {
		super(manager, "Go");
	}
	
	public enum GoType {
		FOREVER, STEP, TICK;
	}
	
	public interface GoProvider {
		public GoType getGoType();
		public int getQuantity();
	}
	
	public void setGoProvider(GoProvider gop) {
		this.gop = gop;
	}

	@Override
	public void update() {
		boolean running = getApplication().getSim().isRunning();
		boolean players = !getApplication().getSim().getWorld().getPlayers().isEmpty();
        setEnabled(!running && players);
	}

	@Override
	public void actionPerformed(ActionEvent e) {
		if (gop == null) {
			getApplication().doRunForever();
		} else {
			switch (gop.getGoType()) {
			case FOREVER:
				getApplication().doRunForever();
				break;
			case STEP:
				getApplication().doRunStep(gop.getQuantity());
				break;
			case TICK:
				getApplication().doRunTick(gop.getQuantity());
				break;
			}
		}
	}

}
