package eatersMap;

public class EatersWall implements eatersMap.EatersSquare {

    private int myWorth = 0;

	public int getWorth() {
		return (myWorth);
	}

    public String getName() {
        return("wall");
    }

    public boolean canEnter() {
        return(false);
    }

}
