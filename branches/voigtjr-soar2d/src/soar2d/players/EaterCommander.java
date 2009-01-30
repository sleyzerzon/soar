package soar2d.players;

import soar2d.map.EatersMap;

public interface EaterCommander extends Commander {
	public void update(EatersMap eatersMap) throws Exception;
}
