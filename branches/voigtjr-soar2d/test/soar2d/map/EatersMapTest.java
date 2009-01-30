package soar2d.map;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertEquals;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class EatersMapTest {
	@Before
	public void setUp() throws Exception {
	}
	
	@After
	public void tearDown() throws Exception {
		
	}
	
	@Test
	public void testLoadDefault() throws Exception {
		new EatersMap("maps/eaters/random-walls.map", false, .35, .85);
	}
}
