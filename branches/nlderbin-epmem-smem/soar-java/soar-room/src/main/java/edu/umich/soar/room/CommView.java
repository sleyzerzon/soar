package edu.umich.soar.room;

import java.awt.BorderLayout;
import java.io.IOException;
import java.io.Writer;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;

import org.flexdock.docking.DockingConstants;

import edu.umich.soar.robot.SendMessages;
import edu.umich.soar.robot.SendMessagesInterface;
import edu.umich.soar.room.core.Simulation;
import edu.umich.soar.room.core.events.PlayerAddedEvent;
import edu.umich.soar.room.core.events.PlayerRemovedEvent;
import edu.umich.soar.room.events.SimEvent;
import edu.umich.soar.room.events.SimEventListener;
import edu.umich.soar.room.map.Robot;

public class CommView extends AbstractAgentView implements SimEventListener {
	
	private static final long serialVersionUID = -1676542480621263207L;
	private final Simulation sim;
	private final JTextField commInput = new JTextField();
	private final SendMessagesInterface sendMessages;
	private final JTextArea commOutput = new JTextArea();
	private final JComboBox commDestination = new JComboBox();
	private static final String DESTINATION_ALL = "-ALL-";
	private static final String OPERATOR = "operator";
	
    private final Writer outputWriter = new Writer()
    {
        private StringBuilder buffer = new StringBuilder();
        private AtomicBoolean flushing = new AtomicBoolean(false);
        
        @Override
        public void close() throws IOException
        {
        }

        @Override
        synchronized public void flush() throws IOException
        {
            // If there's already a runnable headed for the UI thread, don't send another
        	if (!flushing.compareAndSet(false, true)) {
        		return;
        	}
            
            SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                    synchronized(outputWriter) // synchronized on outer.this like the flush() method
                    {
                    	commOutput.append(buffer.toString());
                    	commOutput.setCaretPosition(commOutput.getDocument().getLength());
                        buffer.setLength(0);
                        flushing.set(false);
                    }
                }
            });
        }

        @Override
        synchronized public void write(char[] cbuf, int off, int len) throws IOException
        {
            buffer.append(cbuf, off, len);
        }
    };

    public CommView(Adaptable app) {
        super("commView", "Communication");
        addAction(DockingConstants.PIN_ACTION);
        addAction(DockingConstants.CLOSE_ACTION);

        this.sim = Adaptables.adapt(app, Simulation.class);
        this.sim.getEvents().addListener(PlayerAddedEvent.class, this);
        this.sim.getEvents().addListener(PlayerRemovedEvent.class, this);

        this.sendMessages = Adaptables.adapt(app, SendMessagesInterface.class);
        
        final JPanel p = new JPanel(new BorderLayout());
        p.setBorder(BorderFactory.createTitledBorder("Direct Communication"));
        
        refresh();
        p.add(commDestination, BorderLayout.NORTH);
        
        commOutput.setEditable(false);
        commOutput.setRows(4);
        commOutput.setLineWrap(true);
        commOutput.setWrapStyleWord(true);
        this.sim.getWorld().setCommWriter(outputWriter);
        this.sim.getWorld().setCommOperator(OPERATOR);
        p.add(new JScrollPane(commOutput), BorderLayout.CENTER);
        
        commInput.addKeyListener(new java.awt.event.KeyAdapter() {
			public void keyTyped(java.awt.event.KeyEvent e) {
				if (e.getKeyChar() == '\n') {
					e.consume();
					sendMessage();
				}
			}
		});

        p.add(commInput, BorderLayout.SOUTH);
        final JButton commSend = new JButton("Send");
        commSend.addActionListener(new java.awt.event.ActionListener() {
			public void actionPerformed(java.awt.event.ActionEvent e) {
				sendMessage();
			}
        });
        p.add(commSend, BorderLayout.EAST);
        setContentPane(p);
    }
    
    private void sendMessage() {
    	Object selectedObject = commDestination.getSelectedItem();
    	String dest = null;
    	if (selectedObject != null) {
    		if (selectedObject instanceof Robot) {
    			dest = selectedObject.toString();
    		}
    	}
    	List<String> tokens = SendMessages.toTokens(commInput.getText());
    	if (!tokens.isEmpty()) {
    		sendMessages.sendMessage(OPERATOR, dest, tokens);
    		commInput.setText("");
    	}
    }

	@Override
	public void refresh() {
	}

	@Override
	public void onEvent(SimEvent event) {
		commDestination.removeAllItems();
        commDestination.addItem(DESTINATION_ALL);
        for (Robot robot : sim.getWorld().getPlayers()) {
        	commDestination.addItem(robot);
        }
	}
}
