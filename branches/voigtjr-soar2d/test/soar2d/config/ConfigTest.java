package soar2d.config;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertEquals;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Map.Entry;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class ConfigTest {
	class ConfigTestPair {
		ConfigTestPair(String key, String value) {
			this.key = key;
			this.value = value;
		}
		String key;
		String value;
	}
	
	final String largeTest = "Environments/Soar2D/test/soar2d/config/test.cnf";
	ArrayList<ConfigTestPair> largeTestData = new ArrayList<ConfigTestPair>();

	@Before
	public void setUp() throws Exception {
		largeTestData.add(new ConfigTestPair("hello", "[world]"));
		largeTestData.add(new ConfigTestPair("block.inside", "[true]"));
		largeTestData.add(new ConfigTestPair("block.outside", "[false]"));
		largeTestData.add(new ConfigTestPair("block", "[also with values]"));
		largeTestData.add(new ConfigTestPair("parent.child1.one", "[1]"));
		largeTestData.add(new ConfigTestPair("parent.child1.two", "[2]"));
		largeTestData.add(new ConfigTestPair("parent.child2.one", "[11]"));
		largeTestData.add(new ConfigTestPair("parent.child2.two", "[22]"));
		largeTestData.add(new ConfigTestPair("quotes", "[quotes]"));
		largeTestData.add(new ConfigTestPair("noquotes", "[noquotes]"));
		largeTestData.add(new ConfigTestPair("quotesspace", "[quotes space]"));
		largeTestData.add(new ConfigTestPair("noquotesspace", "[noquotesspace]"));
		largeTestData.add(new ConfigTestPair("single", "[data]"));
		largeTestData.add(new ConfigTestPair("single_element_array", "[data]"));
		largeTestData.add(new ConfigTestPair("trailing_comma_ok", "[data]"));
		largeTestData.add(new ConfigTestPair("two_element_array", "[data, banks]"));
		largeTestData.add(new ConfigTestPair("two_element_array_with_trailer", "[data, banks]"));
		largeTestData.add(new ConfigTestPair("array_no_quotes", "[data, banks]"));
		largeTestData.add(new ConfigTestPair("array_spaces_no_quotes", "[databanks, startrek]"));
		largeTestData.add(new ConfigTestPair("array_spaces_no_quotes_with_trailer", "[databanks, startrek]"));
		largeTestData.add(new ConfigTestPair("double_noquotes", "[1.23]"));
		largeTestData.add(new ConfigTestPair("double_quotes", "[1.23]"));
		largeTestData.add(new ConfigTestPair("split_across_lines", "[testing]"));
		largeTestData.add(new ConfigTestPair("array_line_split", "[hey, you, guys]"));
		largeTestData.add(new ConfigTestPair("array_line_split_noquotes", "[hey, you, guys]"));
		largeTestData.add(new ConfigTestPair("crazy.spacing", "[databanks]"));
		largeTestData.add(new ConfigTestPair("key1", "[value1]"));
		largeTestData.add(new ConfigTestPair("key2", "[value2]"));
		largeTestData.add(new ConfigTestPair("key3", "[value3.1, value3.2, value3.3]"));
		largeTestData.add(new ConfigTestPair("key4", "[value4.1, value4.2]"));
		largeTestData.add(new ConfigTestPair("key5", "[value5.1, value5.2]"));
		largeTestData.add(new ConfigTestPair("key6.subkey6.1", "[value6.1]"));
		largeTestData.add(new ConfigTestPair("key6.subkey6.2", "[value6.2]"));
		largeTestData.add(new ConfigTestPair("dashes-ok", "[yes]"));
		largeTestData.add(new ConfigTestPair("null-array", "[]"));
		//largeTestData.add(new ConfigTestPair("null-array-values", "[,,,]"));
		//largeTestData.add(new ConfigTestPair("nested_not_square", "[]"));
	}

	@Test
	public void testConfig() throws Exception {
		Config cf = new Config(new ConfigFile(largeTest));

		for (ConfigTestPair pair : largeTestData) {
			System.out.println(pair.key + ": " + Arrays.toString(cf.getStrings(pair.key)));
			assertEquals(pair.value, Arrays.toString(cf.getStrings(pair.key)));
		}
	}
	
	@Test
	public void testDoubleChild() throws Exception {
		Config cf = new Config(new ConfigFile(largeTest));

		Config grandparent = cf.getChild("grandparent");
		Config parent = grandparent.getChild("parent");
		assertEquals("indeed", parent.getString("child"));
	}
	
//	public static void main(String args[]) {
//		if (args.length > 0) {
//			try {
//				ConfigFile cf = new ConfigFile(args[0]);
//				
//				
//			} catch (IOException ex) {
//				System.out.println("ex: " + ex);
//			}
//		} else {
//			// test saving by printing to console
//			ConfigFile cf = new ConfigFile();
//			cf.setStrings("testing.one", new String [] { "one" });
//			cf.setStrings("testing.two", new String [] { "one", "two" });
//			cf.setStrings("three", new String [] { "one", "two", "three" });
//			try {
//				cf.save(null);
//			} catch (FileNotFoundException e) {
//				System.out.println("ex: " + e);
//			}
//		}
//	}
}
