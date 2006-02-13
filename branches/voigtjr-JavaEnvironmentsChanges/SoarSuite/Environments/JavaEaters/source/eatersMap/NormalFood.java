package eatersMap;

public class NormalFood implements EatersSquare {

    private int myWorth;
    private boolean canEnter = true;
    
    public NormalFood(int worth){
        myWorth = worth;
    }
    
    public int getWorth(){
        return(myWorth);
    }
    
    public String getName(){
        return("normalfood");
    }
    
    public boolean canEnter(){
        return(canEnter);
    }
    
}
