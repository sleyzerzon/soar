package edu.umich.soar.gridmap2d.map;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.gridmap2d.core.Direction;
import edu.umich.soar.gridmap2d.core.Names;

public class EaterCommands {
	private static Log logger = LogFactory.getLog(EaterCommands.class);

	private static void parseError(Tokenizer t, String msg) {
		logger.error("Command script parse error: " + msg);
		logger.error("Near line " + t.lineNumber + ": " + t.line);
	}
	
	/**
	 * @param path
	 * @return
	 * 
	 * @throws IOException If there is a problem finding or reading the script specified by path
	 */
	public static List<EaterCommand> loadScript(String path) throws IOException {
		Tokenizer t = new Tokenizer(path);
		List<EaterCommand> commands = new ArrayList<EaterCommand>();
		
		while (true) {
			if (!t.hasNext())
				return commands;

			if (!t.consume("(")) {
				parseError(t, "Expected ( got " + t.peek());
			}
			
			EaterCommand.Builder builder = new EaterCommand.Builder();
			
			String tok = t.next();
			if (tok.equals(Names.kJumpID)) {
				builder.jump();
			} else if (tok.equals(Names.kMoveID)) {
				if (!t.consume(":")) {
					parseError(t, "Expected :");
					return commands;
				}
				builder.move(Direction.parse(t.next()));
			} else if (tok.equals(Names.kDontEatID)) {
				builder.dontEat();
			} else if (tok.equals(Names.kOpenID)) {
				builder.open();
			}
			
			if (!t.consume(")")) {
				parseError(t, "Expected ) got " + t.peek());
			}
			
			commands.add(builder.build());
		}
	}
	
	private static class Tokenizer {
		BufferedReader ins;

		// tokens belonging to the current line
		String line;
		int lineNumber = 0;
		Queue<String> tokens = new LinkedList<String>();

		/**
		 * @param path
		 * 
		 * @throws IOException If there is a problem finding or reading the files specified by path
		 */
		public Tokenizer(String path) throws IOException {
			ins = new BufferedReader(new FileReader(path));
		}

		// doesn't support string literals spread across multiple lines.
		void tokenizeLine(String line) {
			String TOKSTOP = "():";

			String tok = "";
			boolean in_string = false;

			for (int pos = 0; pos < line.length(); pos++) {
				char c = line.charAt(pos);

				if (in_string) {
					// in a string literal
					if (c == '\"') {
						// end of string.
						tokens.add(tok);
						in_string = false;
						tok = "";
						continue;
					}

					tok += c;

				} else {
					// NOT in a string literal

					// strip spaces when not in a string literal
					if (Character.isWhitespace(c))
						continue;

					// starting a string literal
					if (c == '\"' && tok.length() == 0) {
						in_string = true;
						continue;
					}

					// does this character end a token?
					if (TOKSTOP.indexOf(c) < 0) {
						// nope, add it to our token so far
						tok += c;
						continue;
					}

					// produce (up to) two new tokens: the accumulated token
					// which has just ended, and a token corresponding to the
					// new character.
					tok = tok.trim();
					if (tok.length() > 0) {
						tokens.add(tok);
						tok = "";
					}

					if (c == '#')
						return;

					// add this terminator character
					tok = "" + c;
					tok = tok.trim();
					if (tok.length() > 0) {
						tokens.add(tok);
						tok = "";
					}
				}
			}

			tok = tok.trim();
			if (tok.length() > 0)
				tokens.add(tok);

		}

		/**
		 * @return
		 * 
		 * @throws IOException If there is a problem reading the script file
		 */
		public boolean hasNext() throws IOException {
			while (true) {
				if (tokens.size() > 0)
					return true;

				line = ins.readLine();
				lineNumber++;
				if (line == null)
					return false;

				tokenizeLine(line);
			}
		}

		// If the next token is s, consume it.
		/**
		 * @param s
		 * @return
		 * @throws IOException If there is a problem reading the script file
		 */
		public boolean consume(String s) throws IOException {
			if (peek().equals(s)) {
				next();
				return true;
			}
			return false;
		}

		/**
		 * @return
		 * @throws IOException If there is a problem reading the script file
		 */
		public String peek() throws IOException {
			if (!hasNext())
				return null;

			return tokens.peek();
		}

		/**
		 * @return
		 * @throws IOException If there is a problem reading the script file
		 */
		public String next() throws IOException {
			if (!hasNext())
				return null;

			String tok = tokens.poll();
			return tok;
		}
	}

}
