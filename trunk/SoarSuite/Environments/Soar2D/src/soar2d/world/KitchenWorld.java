package soar2d.world;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;

import soar2d.Direction;
import soar2d.Soar2D;
import soar2d.map.CellObject;
import soar2d.map.GridMap;
import soar2d.map.KitchenMap;
import soar2d.player.MoveInfo;
import soar2d.player.Player;
import soar2d.player.kitchen.Cook;

public class KitchenWorld implements IWorld {

	public void fragPlayer(Player player, GridMap map, PlayersManager players,
			int [] location) {

	}

	public boolean postLoad(GridMap newMap) {
		return true;
	}

	public void putInStartingLocation(Player player, GridMap map,
			PlayersManager players, int [] location) {
	}

	public void reset(GridMap map) {

	}
	
	public boolean update(GridMap _map, PlayersManager players) {
		KitchenMap map = (KitchenMap)_map;
		
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Cook player = (Cook)iter.next();
			MoveInfo move = players.getMove(player);
			
			// for visual world 
			player.resetPointsChanged();
			
			// check for break-out
			if (Soar2D.control.isShuttingDown()) {
				return false;
			}

			if (move.move) {
				// Calculate new location
				int [] oldLocation = players.getLocation(player);
				int [] newLocation = Arrays.copyOf(oldLocation, oldLocation.length);
				Direction.translate(newLocation, move.moveDirection);
				
				// Verify legal move and commit move
				if (map.isInBounds(newLocation) && map.enterable(newLocation)) {
					ArrayList<CellObject> myStuff = map.getAllWithProperty(oldLocation, "smell");

					// verify I can move with food if I need to
					if (move.moveWithObject) {
						// do I have stuff?
						if (myStuff.size() > 0) {
							// do I have a product?
							if (myStuff.get(0).hasProperty("product")) {
								// yes, I have a product, destination cell must be empty
								ArrayList<CellObject> destStuff  = map.getAllWithProperty(newLocation, "smell");
								if (destStuff.size() > 0) {
									// move with object fails!
									player.moveWithObjectFailed();
									move.moveWithObject = false;
									Soar2D.logger.info(player.getName() + ": move-with-object fails because I can't move a product in to a non-empty cell");
								}
							} else {
								// I'm moving stuff, destination cell must not contain a product
								ArrayList<CellObject> destProducts = map.getAllWithProperty(newLocation, "product");
								assert destProducts.size() < 2;
								if (destProducts.size() > 0) {
									// dest does contain a product, move with object fails!
									player.moveWithObjectFailed();
									move.moveWithObject = false;
									Soar2D.logger.info(player.getName() + ": move-with-object fails because I can't move stuff in to a cell with a product");
								}
							}
						} 
					}					
					
					// remove from cell
					map.setPlayer(oldLocation, null);
					players.setLocation(player, newLocation);
					
					// TODO: collisions not handled
					
					map.setPlayer(newLocation, player);

					// move stuff with the player
					if (move.moveWithObject) {
						if (myStuff.size() > 0) {
							map.removeAllWithProperty(oldLocation, "smell");
							
							Iterator<CellObject> stuffIter = myStuff.iterator();
							String stuffNames = "(";
							while (stuffIter.hasNext()) {
								CellObject object = stuffIter.next();
								map.addObjectToCell(newLocation, object);
								stuffNames += object.getName();
								if (stuffIter.hasNext()) {
									stuffNames += ", ";
								}
							}
							stuffNames += ")";
							Soar2D.logger.info(player.getName() + ": moving with: " + stuffNames);
						}
					}
				}
			}
			
			if (move.mix) {
				if (map.isCountertop(players.getLocation(player)) || map.isOven(players.getLocation(player))) {
					ArrayList<CellObject> stuff = map.getAllWithProperty(players.getLocation(player), "smell");
					if (stuff.size() > 1) {
						map.removeAllWithProperty(players.getLocation(player), "smell");
	
						CellObject mixture = map.createObjectByName("mixture");
						int id = idCounter++;
						mixture.setName("mixture-" + id);
						mixture.addProperty("mixture-id", Integer.toString(id));
						mixture.addProperty("shape", "triangle");
	
						Iterator<CellObject> stuffIter;
	
						// ingredients
						stuffIter = stuff.iterator();
						HashSet<Integer> ingredients = new HashSet<Integer>(stuff.size());
						while (stuffIter.hasNext()) {
							CellObject ingredient = stuffIter.next();
							String name = ingredient.getName();
							if (name.startsWith("mixture")) {
								ingredients.addAll(mixtures.get(ingredient.getIntProperty("mixture-id")));
								
							} else if (name.equalsIgnoreCase("butter")) {
								ingredients.add(kIngredientButter);
								
							} else if (name.equalsIgnoreCase("sugar")) {
								ingredients.add(kIngredientSugar);
								
							} else if (name.equalsIgnoreCase("eggs")) {
								ingredients.add(kIngredientEggs);
								
							} else if (name.equalsIgnoreCase("flour")) {
								ingredients.add(kIngredientFlour);
								
							} else if (name.equalsIgnoreCase("cinnamon")) {
								ingredients.add(kIngredientCinnamon);
								
							} else if (name.equalsIgnoreCase("molasses")) {
								ingredients.add(kIngredientMolasses);
								
							} else {
								assert false;
							}
						}
						mixtures.put(id, ingredients);
						
						// texture
						boolean powder = false;
						mixture.addProperty("texture", "solid");
						stuffIter = stuff.iterator();
						while (stuffIter.hasNext()) {
							CellObject ingredient = stuffIter.next();
							if (ingredient.getProperty("texture").equals("liquid")) {
								mixture.addProperty("texture", "liquid");
								break;
								
							} else if (!powder && ingredient.getProperty("texture").equals("powder")) {
								mixture.addProperty("texture", "powder");
								powder = true;
							}
						}
						
						// color
						boolean brown = false;
						boolean yellow = false;
						mixture.addProperty("color", "white");
						stuffIter = stuff.iterator();
						while (stuffIter.hasNext()) {
							CellObject ingredient = stuffIter.next();
							if (ingredient.getProperty("color").equals("black")) {
								mixture.addProperty("color", "black");
								break;
							} else if (!brown) {
								if (ingredient.getProperty("color").equals("brown")) {
									mixture.addProperty("color", "brown");
									brown = true;
								} else if (!yellow && ingredient.getProperty("color").equals("yellow")) {
									mixture.addProperty("color", "yellow");
									yellow = true;
								}
							}
						}
						
						// smell
						stuffIter = stuff.iterator();
						CellObject left = stuffIter.next();
						int newSmell = toSmell(left.getProperty("smell"));
						while (stuffIter.hasNext()) {
							CellObject right = stuffIter.next();
							newSmell = mixSmell(newSmell, toSmell(right.getProperty("smell")));
						}
						switch (newSmell) {
						case kSmellNone:
							mixture.addProperty("smell", "none");
							break;
						case kSmellMild:
							mixture.addProperty("smell", "mild");
							break;
						case kSmellStrong:
							mixture.addProperty("smell", "strong");
							break;
						default:
							assert false;
						}
						
						Soar2D.logger.info(player.getName() + ": New mixture " + printIngredients(ingredients)
								+ ": " + mixture.getProperty("texture") 
								+ "/" + mixture.getProperty("color") 
								+ "/" + mixture.getProperty("smell"));
						
						map.addObjectToCell(players.getLocation(player), mixture);
					} else {
						if (stuff.size() == 1 && stuff.get(0).hasProperty("product")) {
							Soar2D.logger.info(player.getName() + ": can't mix a product");
						} else {
							Soar2D.logger.info(player.getName() + ": can't mix less than 2 things");
						}
					}
				} else {
					Soar2D.logger.info(player.getName() + ": Tried to mix but not at countertop or oven");
				}
			}
			
			if (move.cook) {
				if (map.isOven(players.getLocation(player))) {
					ArrayList<CellObject> stuff = map.getAllWithProperty(players.getLocation(player), "smell");
					if (stuff.size() > 1) {
						Soar2D.logger.info(player.getName() + ": Too many things to cook, mix first");
						
					} else if (stuff.size() == 1) {
						CellObject ingredient = stuff.get(0);
						if (ingredient.hasProperty("product")) {
							Soar2D.logger.info(player.getName() + ": Can't cook a product");
						} else {
							// consume it
							map.removeAllWithProperty(players.getLocation(player), "smell");

							// create something
							Product product = Product.Burned;
							
							if (ingredient.getName().startsWith("mixture")) {
								// get ingredient list
								HashSet<Integer> ingredients = mixtures.get(stuff.get(0).getIntProperty("mixture-id"));
								assert ingredients != null;
								
								if (ingredients.remove(kIngredientButter)) {
									if (ingredients.remove(kIngredientSugar)) {
										if (ingredients.isEmpty()) {
											// toffee
											product = Product.Toffee;
											
										} else if (ingredients.remove(kIngredientEggs)) {
											if (ingredients.remove(kIngredientFlour)) {
												if (ingredients.isEmpty()) {
													// sugar cookies
													product = Product.SugarCookies;
		
												} else if (ingredients.remove(kIngredientCinnamon)) {
													if (ingredients.isEmpty()) {
														// snickerdoodles
														product = Product.Snickerdoodles;
		
													} else if (ingredients.remove(kIngredientMolasses)) {
														if (ingredients.isEmpty()) {
															// molasses cookies
															product = Product.MolassesCookies;
														}
													}										
												}										
											}
										}
									}
								} else if (ingredients.remove(kIngredientSugar)) {
									if (ingredients.remove(kIngredientEggs)) {
										if (ingredients.isEmpty()) {
											// custard
											product = Product.Custard;
										}
									}
								}
							}
							
							switch (product) {
							case Burned:
								map.addObjectByName(players.getLocation(player), "burned");
								Soar2D.logger.info(player.getName() + ": creating burned");
								break;
							case Toffee:
								map.addObjectByName(players.getLocation(player), "toffee");
								Soar2D.logger.info(player.getName() + ": creating toffee");
								break;
							case SugarCookies:
								map.addObjectByName(players.getLocation(player), "sugarcookies");
								Soar2D.logger.info(player.getName() + ": creating sugarcookies");
								break;
							case Snickerdoodles:
								map.addObjectByName(players.getLocation(player), "snickerdoodles");
								Soar2D.logger.info(player.getName() + ": creating snickerdoodles");
								break;
							case MolassesCookies:
								map.addObjectByName(players.getLocation(player), "molassescookies");
								Soar2D.logger.info(player.getName() + ": creating molassescookies");
								break;
							case Custard:
								map.addObjectByName(players.getLocation(player), "custard");
								Soar2D.logger.info(player.getName() + ": creating custard");
								break;
							}
							
						}
					} else {
						Soar2D.logger.info(player.getName() + ": Nothing to cook");
					}
				} else {
					Soar2D.logger.info(player.getName() + ": Tried to cook but not at oven");
				}
			}
			
			if (move.eat) {
				ArrayList<CellObject> stuff = map.getAllWithProperty(players.getLocation(player), "smell");
				if (stuff.size() > 1) {
					Soar2D.logger.info(player.getName() + ": Too many things to eat, mix first");
					
				} else if (stuff.size() == 1) {
					// consume it
					map.removeAllWithProperty(players.getLocation(player), "smell");
					
					// what is it?
					String name = stuff.get(0).getName();
					int points = 0;
					if (name.equals("toffee")) {
						points = 1;
					} else if (name.equals("custard")) {
						points = 1;
					} else if (name.equals("sugarcookies")) {
						points = 2;
					} else if (name.equals("snickerdoodles")) {
						points = 3;
					} else if (name.equals("molassescookies")) {
						points = 4;
					} else if (name.equals("burned")) {
						points = -1;
					}
					
					player.adjustPoints(points, "ate " + name);
					
				} else {
					Soar2D.logger.info(player.getName() + ": Nothing to eat");
				}
			}
		}
		
		updatePlayers(false, map, players);

		map.updateObjects(null);
		
		map.spawnBasics();

		// do not reset after this frame
		return false;
	}
	
	private String printIngredients(HashSet<Integer> ingredients) {
		Iterator<Integer> iter = ingredients.iterator();
		String output = "(";
		while (iter.hasNext()) {
			final int ingredient = iter.next().intValue();
			if (ingredient == kIngredientButter) {
				output += "butter";
			} else if (ingredient == kIngredientSugar) {
				output += "sugar";
			} else if (ingredient == kIngredientEggs) {
				output += "eggs";
			} else if (ingredient == kIngredientFlour) {
				output += "flour";
			} else if (ingredient == kIngredientCinnamon) {
				output += "cinnamon";
			} else if (ingredient == kIngredientMolasses) {
				output += "molasses";
			} else {
				assert false;
			}
			
			if (iter.hasNext()) {
				output += ", ";
			}
		}
		output += ")";
		return output;
	}
	
	enum Product { Toffee, Custard, SugarCookies, Snickerdoodles, MolassesCookies, Burned };
	
	int idCounter = 0;
	final Integer kIngredientButter = 0;
	final Integer kIngredientSugar = 1;
	final Integer kIngredientEggs = 2;
	final Integer kIngredientFlour = 3;
	final Integer kIngredientCinnamon = 4;
	final Integer kIngredientMolasses = 5;
	
	HashMap< Integer, HashSet<Integer> > mixtures = new HashMap< Integer, HashSet<Integer> >();
	
	final int kSmellNone = 0;
	final int kSmellMild = 1;
	final int kSmellStrong = 2;

	private int toSmell(String smell) {
		if (smell.equalsIgnoreCase("strong")) {
			return kSmellStrong;
		}
		if (smell.equalsIgnoreCase("mild")) {
			return kSmellMild;
		}
		return kSmellNone;
	}
	
	private int mixSmell(int left, int right) {
		
		if (left > right) {
			int temp = left;
			left = right;
			right = temp;
		}

		// left <= right
		if (left == kSmellNone) {
			if (right == kSmellNone) {
				return kSmellNone;
			}
			return kSmellMild;
		} else {
			return kSmellStrong;
		}
	}
	
	public void updatePlayers(boolean playersChanged, GridMap map, PlayersManager players) {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			player.update(players.getLocation(player));
		}
	}
	
	public int getMinimumAvailableLocations() {
		return 1;
	}

	public void resetPlayer(GridMap map, Player player, PlayersManager players, boolean resetDuringRun) {
		player.reset();
	}

	public GridMap newMap() {
		return new KitchenMap();
	}
}
