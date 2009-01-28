package soar2d.map;

import static org.junit.Assert.assertFalse;

import org.junit.Test;

public class CellObjectTest {
	@Test
	public void testEquals() {
		CellObject a = new CellObject("a");
		CellObject b = new CellObject("b");
		assertFalse(a.equals(b));

		CellObject a1 = new CellObject("a");
		CellObject a2 = new CellObject("a");
		assertFalse(a1.equals(a2));
	}
	
	@Test(expected = NullPointerException.class)
	public void testNullName() {
		String s = null;
		new CellObject(s);
	}
	
	@Test(expected = NullPointerException.class)
	public void testNullCopyConstructor() {
		CellObject c = null;
		new CellObject(c);
	}
}
