package soar2d.players;

import soar2d.map.EatersMap;

public interface EaterCommander {
	public void update(EatersMap eatersMap) throws Exception;
	public CommandInfo getCommand() throws Exception;
	public void reset() throws Exception;
	public void shutdown() throws Exception;
}
