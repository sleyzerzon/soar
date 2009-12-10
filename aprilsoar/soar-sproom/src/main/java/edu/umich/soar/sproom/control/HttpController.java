package edu.umich.soar.sproom.control;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArrayList;

import jmat.MathUtil;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import edu.umich.soar.sproom.control.PIDController.Gains;

class HttpController {
	private static final Log logger = LogFactory.getLog(HttpController.class);
	private static final int HTTP_PORT = 8000;
	private static final String INDEX_HTML = "/edu/umich/soar/sproom/control/index.html";
	private static final String ERROR_HTML = "/edu/umich/soar/sproom/control/error.html";
	
	static HttpController newInstance() {
		return new HttpController();
	}
	
	private final String ACTION = "action";
	enum Action {
		postmessage, heading, angvel, linvel, estop, stop, agains, lgains, hgains, soar
	}
	
	private enum Key {
		message, heading, angvel, linvel, pgain, igain, dgain
	}
	
	private final IndexHandler indexHandler = new IndexHandler();

	private HttpController() {
		try {
		    HttpServer server = HttpServer.create(new InetSocketAddress(HTTP_PORT), 0);
		    server.createContext("/", indexHandler);
		    server.createContext("/debug", new DebugHandler());
		    server.start();
		} catch (IOException e) {
			logger.fatal("Error starting http server: " + e.getMessage());
			e.printStackTrace();
			System.exit(1);
		}
		
		logger.info("http server running on port " + HTTP_PORT);
	}
	
	private final List<HttpControllerEventHandler> handlers = new CopyOnWriteArrayList<HttpControllerEventHandler>();
	public void addEventHandler(HttpControllerEventHandler handler) {
		handlers.add(handler);
	}
	
	private void fireEvent(HttpControllerEvent event) {
		for (HttpControllerEventHandler handler : handlers) {
			handler.handleEvent(event);
		}
	}
	
	public void setGains(Gains hgains, Gains agains, Gains lgains) {
		indexHandler.setGains(hgains, agains, lgains);
	}
	
	private class IndexHandler implements HttpHandler {
		public void handle(HttpExchange xchg) throws IOException {
			if (xchg.getRequestMethod().equals("GET")) {
				sendFile(xchg, INDEX_HTML);
			} else {
				handlePost(xchg);
			}
		}
		
		private void sendFile(HttpExchange xchg, String path) throws IOException {
			StringBuffer response = new StringBuffer();

			// I'm sure there is a better way to pipe this stuff!
			BufferedReader br = 
				new BufferedReader(
					new InputStreamReader(
							HttpController.class.getResourceAsStream(path)));

			String line;
			while ((line = br.readLine()) != null) {
				line = performSubstitutions(line);
				response.append(line);
				response.append("\n");
			}

			sendResponse(xchg, response.toString());
		}
		
		Gains hgains;
		Gains agains;
		Gains lgains;
		
		private void setGains(Gains hgains, Gains agains, Gains lgains) {
			this.hgains = hgains;
			this.agains = agains;
			this.lgains = lgains;
		}
		
		private String performSubstitutions(String line) {
			line = line.replaceAll("%hpgain%", Double.toString(hgains.p));
			line = line.replaceAll("%higain%", Double.toString(hgains.i));
			line = line.replaceAll("%hdgain%", Double.toString(hgains.d));

			line = line.replaceAll("%apgain%", Double.toString(agains.p));
			line = line.replaceAll("%aigain%", Double.toString(agains.i));
			line = line.replaceAll("%adgain%", Double.toString(agains.d));

			line = line.replaceAll("%lpgain%", Double.toString(lgains.p));
			line = line.replaceAll("%ligain%", Double.toString(lgains.i));
			line = line.replaceAll("%ldgain%", Double.toString(lgains.d));
			return line;
		}

		private void handlePost(HttpExchange xchg) throws IOException {
			BufferedReader br = new BufferedReader(new InputStreamReader(xchg.getRequestBody()));
			char[] buf = new char[256];
			int len;
			StringBuffer body = new StringBuffer();
			while ((len = br.read(buf)) != -1) {
				body.append(buf, 0, len);
			}

			Map<String, String> properties = new HashMap<String, String>();
			String[] all = body.toString().split("&");
			for (String each : all) {
				String[] pair = each.split("=");
				if (pair.length > 2) {
					logger.error("Too many tokens: " + each);
					sendFile(xchg, ERROR_HTML);
					return;
				} else if (pair.length == 2) {
					properties.put(pair[0], URLDecoder.decode(pair[1], "UTF-8"));
				} else if (pair.length == 1) {
					properties.put(pair[0], null);
				} else {
					logger.error("No tokens");
					sendFile(xchg, ERROR_HTML);
					return;
				}
			}
			
			if (properties.size() == 0 || !properties.containsKey(ACTION) 
					|| properties.get(ACTION) == null) {
				logger.error("No action specified");
				sendFile(xchg, ERROR_HTML);
				return;
			}
			
			logger.trace(properties.get(ACTION));
			if (properties.get(ACTION).equals(Action.postmessage.name())) {
				actionPostMessage(xchg, properties);
			} else if (properties.get(ACTION).equals(Action.heading.name())) {
				actionHeading(xchg, properties);
			} else if (properties.get(ACTION).equals(Action.angvel.name())) {
				actionAngvel(xchg, properties);
			} else if (properties.get(ACTION).equals(Action.linvel.name())) {
				actionLinvel(xchg, properties);
			} else if (properties.get(ACTION).equals(Action.estop.name())) {
				actionEstop(xchg);
			} else if (properties.get(ACTION).equals(Action.stop.name())) {
				actionStop(xchg);
			} else if (properties.get(ACTION).equals(Action.agains.name())) {
				actionAGains(xchg, properties);
			} else if (properties.get(ACTION).equals(Action.lgains.name())) {
				actionLGains(xchg, properties);
			} else if (properties.get(ACTION).equals(Action.hgains.name())) {
				actionHGains(xchg, properties);
			} else if (properties.get(ACTION).equals(Action.soar.name())) {
				actionSoar(xchg);
			} else {
				logger.error("Unknown action: " + properties.get(ACTION));
				sendFile(xchg, ERROR_HTML);
				return;
			}
		}
		
		private void actionPostMessage(HttpExchange xchg, Map<String, String> properties) throws IOException {
			String message = properties.get(Key.message.name());
			if (message == null) {
				sendFile(xchg, ERROR_HTML);
				return;
			}
			
			List<String> tokens = Arrays.asList(message.split(" "));
			
			Iterator<String> iter = tokens.iterator();
			while (iter.hasNext()) {
				String token = iter.next();
				if (token.length() == 0) {
					iter.remove();
				}
			}

			fireEvent(new HttpControllerEvent.MessageChanged(tokens));
			
		    StringBuffer response = new StringBuffer();
		    if (tokens.size() > 0) {
			    response.append("Sent: ");
			    response.append(Arrays.toString(tokens.toArray(new String[tokens.size()])));
			    response.append("\n");
		    } else {
		    	response.append("Cleared all messages.\n");
		    }
			logger.debug(response);
			
		    sendResponse(xchg, response.toString());
		}
		
		private void actionHeading(HttpExchange xchg, Map<String, String> properties) throws IOException {
			String headingString = properties.get(Key.heading.name());
			if (headingString == null) {
				sendFile(xchg, INDEX_HTML);
				return;
			}
			
			try {
				double yaw = Math.toRadians(Double.parseDouble(headingString));
				yaw = MathUtil.mod2pi(yaw);
				DifferentialDriveCommand ddc = DifferentialDriveCommand.newHeadingCommand(yaw);
				fireEvent(new HttpControllerEvent.DDCChanged(ddc));
				logger.debug(ddc);
				sendFile(xchg, INDEX_HTML);
			} catch (NumberFormatException e) {
				sendResponse(xchg, "Invalid number");
				return;
			}
		}
		
		private void actionAngvel(HttpExchange xchg, Map<String, String> properties) throws IOException {
			String angvelString = properties.get(Key.angvel.name());
			if (angvelString == null) {
				sendFile(xchg, INDEX_HTML);
				return;
			}
			
			try {
				double angvel = Math.toRadians(Double.parseDouble(angvelString));
				DifferentialDriveCommand ddc = DifferentialDriveCommand.newAngularVelocityCommand(angvel);
				fireEvent(new HttpControllerEvent.DDCChanged(ddc));
				logger.debug(ddc);
				sendFile(xchg, INDEX_HTML);
			} catch (NumberFormatException e) {
				sendResponse(xchg, "Invalid number");
				return;
			}
		}
		
		private void actionLinvel(HttpExchange xchg, Map<String, String> properties) throws IOException {
			String linvelString = properties.get(Key.linvel.name());
			if (linvelString == null) {
				sendFile(xchg, INDEX_HTML);
				return;
			}
			
			try {
				double linvel = Double.parseDouble(linvelString);
				DifferentialDriveCommand ddc = DifferentialDriveCommand.newLinearVelocityCommand(linvel);
				fireEvent(new HttpControllerEvent.DDCChanged(ddc));
				logger.debug(ddc);
				sendFile(xchg, INDEX_HTML);
			} catch (NumberFormatException e) {
				sendResponse(xchg, "Invalid number");
				return;
			}
		}
		
		private void actionEstop(HttpExchange xchg) throws IOException {
			DifferentialDriveCommand ddc = DifferentialDriveCommand.newEStopCommand();
			fireEvent(new HttpControllerEvent.DDCChanged(ddc));
			sendFile(xchg, INDEX_HTML);
		}

		private void actionStop(HttpExchange xchg) throws IOException {
			DifferentialDriveCommand ddc = DifferentialDriveCommand.newVelocityCommand(0, 0);
			fireEvent(new HttpControllerEvent.DDCChanged(ddc));
			sendFile(xchg, INDEX_HTML);
		}

		private void actionSoar(HttpExchange xchg) throws IOException {
			fireEvent(new HttpControllerEvent.SoarChanged());
			sendFile(xchg, INDEX_HTML);
		}

		private double parseDefault(String value) {
			double out = 0;
			try {
				out = Double.parseDouble(value);
			} catch (NullPointerException ignored) {
				// ignored, use 0
			}
			return out;
		}
		
		private void actionAGains(HttpExchange xchg, Map<String, String> properties) throws IOException {
			double p = 0;
			double i = 0;
			double d = 0;
			

			try {
				p = parseDefault(properties.get(Key.pgain.name()));
				i = parseDefault(properties.get(Key.igain.name()));
				d = parseDefault(properties.get(Key.dgain.name()));
			} catch (NumberFormatException e) {
				sendResponse(xchg, "Invalid number");
				return;
			}

			agains = new Gains(p, i, d);
			fireEvent(new HttpControllerEvent.GainsChanged(hgains, agains, lgains));
			sendFile(xchg, INDEX_HTML);
		}
		
		private void actionLGains(HttpExchange xchg, Map<String, String> properties) throws IOException {
			double p = 0;
			double i = 0;
			double d = 0;

			try {
				p = parseDefault(properties.get(Key.pgain.name()));
				i = parseDefault(properties.get(Key.igain.name()));
				d = parseDefault(properties.get(Key.dgain.name()));
			} catch (NumberFormatException e) {
				sendResponse(xchg, "Invalid number");
				return;
			}
			
			lgains = new Gains(p, i, d);
			fireEvent(new HttpControllerEvent.GainsChanged(hgains, agains, lgains));
			sendFile(xchg, INDEX_HTML);
		}
		
		private void actionHGains(HttpExchange xchg, Map<String, String> properties) throws IOException {
			double p = 0;
			double i = 0;
			double d = 0;

			try {
				p = parseDefault(properties.get(Key.pgain.name()));
				i = parseDefault(properties.get(Key.igain.name()));
				d = parseDefault(properties.get(Key.dgain.name()));
			} catch (NumberFormatException e) {
				sendResponse(xchg, "Invalid number");
				return;
			}
			
			hgains = new Gains(p, i, d);
			fireEvent(new HttpControllerEvent.GainsChanged(hgains, agains, lgains));
			sendFile(xchg, INDEX_HTML);
		}
		
	}
	
	private class DebugHandler implements HttpHandler {
		public void handle(HttpExchange xchg) throws IOException {
			Headers headers = xchg.getRequestHeaders();
			Set<Map.Entry<String, List<String>>> entries = headers.entrySet();

			StringBuffer response = new StringBuffer();
			for (Map.Entry<String, List<String>> entry : entries) {
				response.append(entry.toString() + "\n");
			}
			response.append("\n\n");
			
			BufferedReader br = new BufferedReader(new InputStreamReader(xchg.getRequestBody()));
			char[] buf = new char[256];
			int len;
			while ((len = br.read(buf)) != -1) {
				response.append(buf, 0, len);
			}

			sendResponse(xchg, response.toString());
		}
	}
	
	private void sendResponse(HttpExchange xchg, String response) throws IOException {
	    xchg.sendResponseHeaders(200, response.length());
	    OutputStream os = xchg.getResponseBody();
	    os.write(response.toString().getBytes());
	    os.close();
	}
}
