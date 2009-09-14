package edu.umich.soar.gridmap2d.core.events;

public class InfoEvent extends AbstractNotifyEvent {
	public InfoEvent(String message) {
		super(message);
	}	
	
	public InfoEvent(String title, String message) {
		super(title, message);
	}
}
