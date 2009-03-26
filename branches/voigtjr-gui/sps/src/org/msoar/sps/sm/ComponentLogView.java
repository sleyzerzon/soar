package org.msoar.sps.sm;

import java.awt.Font;
import java.io.IOException;
import java.io.Writer;

import javax.swing.JTextArea;
import javax.swing.SwingUtilities;

import org.flexdock.view.View;

class ComponentLogView extends View {

	private static final long serialVersionUID = -2648030870011109891L;

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

    public ComponentLogView(SessionManager sm) {
		super("log", "Log");
		
		this.sm = sm;
		
        outputWindow.setFont(new Font("Monospaced", Font.PLAIN, 12));
        
	}
}
