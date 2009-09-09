package edu.umich.soar.gridmap2d.core;

public class ErrorEvent extends AbstractNotifyEvent {
	ErrorEvent(String message) {
		super(message);
	}	
	
	ErrorEvent(String title, String message) {
		super(title, message);
	}
}
