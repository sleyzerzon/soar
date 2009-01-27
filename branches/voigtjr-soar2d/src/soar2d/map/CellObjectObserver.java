package soar2d.map;

interface CellObjectObserver {
	void addStateUpdate(int[] xy, CellObject object);
	void removalStateUpdate(int[] xy, CellObject object);
}
