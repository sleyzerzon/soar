package edu.umich.soar.gridmap2d.core;

public class InfoEvent extends AbstractNotifyEvent {
	InfoEvent(String message) {
		super(message);
	}	
	
	InfoEvent(String title, String message) {
		super(title, message);
	}
}
