/* File: EaterControl.java
 * Jul 12, 2004
 */

package eaters;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

import javax.swing.JOptionPane;

import eatersMap.EatersSquare;
import eatersMap.EatersWall;
import eatersMap.NormalFood;
import eatersMap.BonusFood;
import eatersMap.EatersEmpty;


import sml.Kernel;
import sml.smlSystemEventId;
import sml.smlUpdateEventId;

import base.SimulationControl;

public class EaterControl extends SimulationControl {
	
	EatersSettings m_Settings = null;	
	private int foodCount = 0;
	
	public EaterControl(EatersSettings settings) {
		super(settings);
		
		m_Settings = settings;
		logger.log("Eaters command line: " + m_Settings.toString() + "\n");		
		
		initSoar();
		generateBasePath(m_Settings.FOLDER_NAME);
		
		if (m_Settings.map == null) {
			generateMap();
		} else {
			loadMap(new File(m_Settings.map));
		}		
	}
	
	/**
	 * Generates the map to be used by EaterControl, filling every #BonusFoodColumns
	 * column with bonusfood (or at least the integer), and creating walls randomly
	 * but with probability defined by Probability. No walls are created diagonal to
	 * any others, but walls that will be adjacent to others have probability defined
	 * by BetterProbability of being created.
	 */
	private void generateMap() {
		int width = m_Settings.MAP_WIDTH;
		int height = m_Settings.MAP_HEIGHT;
		
		EatersSquare wall = new EatersWall();

		for (int x = 0; x < width; x++) {
			map[x][0] = map[x][height - 1] = wall;
		}
		for (int y = 1; y < height - 1; y++) {
			map[0][y] = map[width - 1][y] = wall;
		}
		
		// Clear old map
		NormalFood normal = new NormalFood(m_Settings.NORMAL_FOOD_WORTH);
		for (int x = 1; x < width - 1; x++) {
			for (int y = 1; y < height - 1; y++) {
				map[x][y] = normal;
			}
		}
		
		double probability = m_Settings.PROBABILITY;
		for (int x = 2; x < width - 2; x++) {
			for (int y = 2; y < height - 2; y++) {
				if (!(map[x + 1][y + 1] instanceof EatersWall)
						&& !(map[x - 1][y - 1] instanceof EatersWall)
						&& !(map[x + 1][y - 1] instanceof EatersWall)
						&& !(map[x - 1][y + 1] instanceof EatersWall)) {
					if ((map[x + 1][y] instanceof EatersWall)
							|| (map[x][y + 1] instanceof EatersWall)
							|| (map[x - 1][y] instanceof EatersWall)
							|| (map[x][y - 1] instanceof EatersWall)) {
						probability = m_Settings.BETTER_PROBABILITY;
					}
					if (random.nextDouble() < probability) {
						map[x][y] = wall;
					}
					probability = m_Settings.PROBABILITY;
				}
			}
		}
		fillFoods();
	}	
	
	private void fillFoods() {
		NormalFood normal = new NormalFood(m_Settings.NORMAL_FOOD_WORTH);
		BonusFood bonus = new BonusFood(m_Settings.BONUS_FOOD_WORTH);
		
		foodCount = 0;
		for (int x = 1; x < m_Settings.MAP_WIDTH - 1; x++) {
			for (int y = 1; y < m_Settings.MAP_HEIGHT - 1; y++) {
				if (!(map[x][y] instanceof EatersWall)) {
					if (x % m_Settings.BONUS_FOOD_COLUMNS == 0) {
						map[x][y] = bonus;
					} else {
						map[x][y] = normal;
					}
					++foodCount;
				}
			}
		}
	}

	public int getFoodCount() {
		return foodCount;
	}

	public void setFoodCount(int foodCount) {
		this.foodCount = foodCount;
	}
	
	public void loadMap(File file) {
		BufferedReader bIn = null;
		
		try {
			bIn = new BufferedReader(new FileReader(file));
		} catch (FileNotFoundException f) {
			logger.log("File not found: " + file.getName());
			System.exit(1);
		}
		
		EatersSquare empty = new EatersEmpty(0);
		EatersSquare normal = new NormalFood(m_Settings.NORMAL_FOOD_WORTH);
		EatersSquare bonus = new BonusFood(m_Settings.BONUS_FOOD_WORTH);
		EatersSquare wall = new EatersWall();
		foodCount = 0;
		
		// TODO: better input error handling
		try {
			for (int y = 0; y < m_Settings.MAP_HEIGHT; y++) {
				String s = bIn.readLine();
				for (int x = 0; x < s.length(); x++) {
					int type = s.charAt(x) - '0';
					switch (type) {
					case (EatersSquare.EMPTY):
						map[x][y] = empty;
						break;
					case (EatersSquare.NORMAL_FOOD):
						map[x][y] = normal;
						break;
					case (EatersSquare.WALL):
						map[x][y] = wall;
						break;
					case (EatersSquare.BONUS_FOOD):
						map[x][y] = bonus;
						break;
					default:
						map[x][y] = empty;
						break;
					}
					if (type == EatersSquare.NORMAL_FOOD || type == EatersSquare.BONUS_FOOD) {
						++foodCount;
					}
				}
			}
		} catch (IOException e) {
			logger.log("IOException when loading map: " + e.getMessage());
			System.exit(1);
		} catch (Exception e) {
			logger.log("Exception when loading map: " + e.getMessage());
			System.exit(1);
		}
		
		try {
			bIn.close();
		} catch (IOException ignored) {
		}
		
		// TODO: events
		//fireNewMapNotification(null);
	}

}