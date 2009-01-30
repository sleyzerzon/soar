package soar2d.map;

import java.io.File;
import java.util.ArrayList;

public interface GridMap {
	public void reset() throws Exception;
	public int size();
	public Cell getCell(int[] xy);
	public boolean isAvailable(int[] location);
	public int[] getAvailableLocationAmortized();
	public boolean isInBounds(int[] xy);
	public CellObject createObjectByName(String name);
	public File getMetadataFile();
	public ArrayList<CellObject> getTemplatesWithProperty(String name);
}
