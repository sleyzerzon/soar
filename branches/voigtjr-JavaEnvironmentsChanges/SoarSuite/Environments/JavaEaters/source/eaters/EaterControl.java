package eaters;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

import eatersMap.EatersSquare;
import eatersMap.EatersWall;
import eatersMap.NormalFood;
import eatersMap.BonusFood;
import eatersMap.EatersEmpty;

import base.SimulationControl;

public class EaterControl extends SimulationControl {
	
	EatersSettings m_Settings = null;	
	private int foodCount = 0;
	
	public EaterControl(EatersSettings settings) {
		super(settings);
		
		m_Settings = settings;
		logger.log(m_Settings.toString());		
		
		initSoar();
		generateBasePath(m_Settings.FOLDER_NAME);
		
		if (m_Settings.map == null) {
			//generateMap();
		} else {
			loadMap(new File(m_Settings.map));
		}		
	}
	
	public EatersSettings getSettings() {
		return m_Settings;
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
						//map[x][y] = empty;
						break;
					case (EatersSquare.NORMAL_FOOD):
						//map[x][y] = normal;
						break;
					case (EatersSquare.WALL):
						//map[x][y] = wall;
						break;
					case (EatersSquare.BONUS_FOOD):
						//map[x][y] = bonus;
						break;
					default:
						//map[x][y] = empty;
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