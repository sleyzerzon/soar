package edu.umich.soar.gridmap2d.map;

import java.util.ArrayList;
import java.util.List;




public class ScriptedEater implements EaterCommander {
	
	List<EaterCommand> commands = new ArrayList<EaterCommand>();
	int index;
	
	public ScriptedEater(List<EaterCommand> commands) {
		reset();
		this.commands = new ArrayList<EaterCommand>(commands);
	}

	@Override
	public EaterCommand nextCommand() {
		EaterCommand command;
		if (index >= commands.size()) {
			command = new EaterCommand.Builder().build();
		} else {
			command = commands.get(index++);
		}
		return command;
	}

	@Override
	public void reset() {
		index = 0;
	}

	@Override
	public void shutdown() {
	}
}
