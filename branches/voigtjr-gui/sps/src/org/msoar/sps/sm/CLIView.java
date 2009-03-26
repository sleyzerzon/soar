package org.msoar.sps.sm;

import java.awt.BorderLayout;
import java.awt.Font;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.Writer;

import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.SwingUtilities;

import org.flexdock.docking.DockingConstants;
import org.flexdock.view.View;

public class CLIView extends View
{
	private static final long serialVersionUID = 1961356824471119682L;

	private final SessionManager sm;
    
    private JTextArea outputWindow = new JTextArea();
    private Writer outputWriter = new Writer()
    {
        private StringBuilder buffer = new StringBuilder();
        
        @Override
        public void close() throws IOException
        {
        }

        @Override
        synchronized public void flush() throws IOException
        {
            final String output = buffer.toString();
            buffer = new StringBuilder();

            if(output.length() > 0)
            {
                SwingUtilities.invokeLater(new Runnable() {
                    public void run() {
                        outputWindow.append(output);
                        outputWindow.setCaretPosition(outputWindow.getDocument().getLength());
                    }
                });
            }
        }

        @Override
        synchronized public void write(char[] cbuf, int off, int len) throws IOException
        {
            buffer.append(cbuf, off, len);
        }
    };

    public CLIView(SessionManager sm)
    {
        super("sm", "Session Manager");
        this.sm = sm;
        
        outputWindow.setFont(new Font("Monospaced", Font.PLAIN, 12));
        
        outputWindow.addMouseListener(new MouseAdapter() {

            public void mousePressed(MouseEvent e) { mouseReleased(e); }

            public void mouseReleased(MouseEvent e)
            {
            }});
        this.sm.setWriter(new PrintWriter(outputWriter, true));
        
        this.addAction(DockingConstants.PIN_ACTION);

        JPanel p = new JPanel(new BorderLayout());
        //p.add(new RunControlPanel(debugger), BorderLayout.NORTH);
        p.add(new JScrollPane(outputWindow), BorderLayout.CENTER);
        
        p.add(new CommandEntryPanel(this.sm), BorderLayout.SOUTH);
        this.setContentPane(p);
    }
}
