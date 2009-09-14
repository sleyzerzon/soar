package edu.umich.soar.gridmap2d.core.events;

public class ErrorEvent extends AbstractNotifyEvent {
	public ErrorEvent(String message) {
		super(message);
	}	
	
	public ErrorEvent(String title, String message) {
		super(title, message);
	}
}
