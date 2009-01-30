package soar2d.players.scripted;

import java.util.ArrayList;
import java.util.List;

import soar2d.map.EatersMap;
import soar2d.players.CommandInfo;
import soar2d.players.EaterCommander;

public class ScriptedEater implements EaterCommander {
	
	List<CommandInfo> commands = new ArrayList<CommandInfo>();
	int index;
	
	public ScriptedEater(List<CommandInfo> commands) throws Exception {
		reset();
		this.commands = new ArrayList<CommandInfo>(commands);
	}

	public CommandInfo getCommand() throws Exception {
		return commands.get(index++);
	}

	public void reset() throws Exception {
		index = 0;
	}

	public void shutdown() throws Exception {
	}

	public void update(EatersMap eatersMap) throws Exception {
	}
}
