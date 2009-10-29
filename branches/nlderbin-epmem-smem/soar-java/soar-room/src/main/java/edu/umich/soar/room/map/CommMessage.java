package edu.umich.soar.room.map;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

// Immutable
public class CommMessage {
	// TODO consider rolling in to sml-java
	CommMessage(String from, String to, List<String> tokens) {
		this.from = from;
		this.destination = to;
		this.tokens = new ArrayList<String>(tokens);
	}
	
	CommMessage(String from, List<String> tokens) {
		this.from = from;
		this.destination = null;
		this.tokens = new ArrayList<String>(tokens);
	}
	
	private final String from;
	private final List<String> tokens;
	private final String destination;
	
	public boolean isBroadcast() {
		return destination == null;
	}
	
	public String getDestination() {
		return destination;
	}
	
	public String getFrom() {
		return from;
	}
	
	public List<String> getTokens() {
		return Collections.unmodifiableList(this.tokens);
	}
	
	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append(from);
		if (destination != null) {
			sb.append(" -> ");
			sb.append(destination);
		}
		sb.append(":");
		for (String token : tokens) {
			sb.append(" ");
			sb.append(token);
		}
		return sb.toString();
	}

}
