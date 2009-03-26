package org.msoar.sps.sm;

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JComboBox;
import javax.swing.JPanel;

/**
 * A panel with a command entry field and history.
 * 
 * @author ray
 */
public class CommandEntryPanel extends JPanel
{
	private static final long serialVersionUID = 43891091589360728L;

	private final SessionManager sm;
    private final JComboBox field = new JComboBox();
    
    /**
     * Construct the panel with the given debugger
     * 
     * @param debugger
     */
    public CommandEntryPanel(SessionManager sm)
    {
        super(new BorderLayout());
        this.sm = sm;
        
        this.add(field, BorderLayout.CENTER);
        
        field.setEditable(true);
        field.getEditor().addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
                execute();
			}
        });

    }
    
    private void execute()
    {
        final String command = field.getEditor().getItem().toString().trim();
        if(command.length() > 0)
        {
        	sm.executeCommand(command);
            addCommand(command);
        }
    }
    
    private void addCommand(String command)
    {
        for(int i = 0; i < field.getItemCount(); ++i)
        {
            if(command.equals(field.getItemAt(i)))
            {
                field.removeItemAt(i);
                break;
            }
        }
        field.insertItemAt(command, 0);
        field.setSelectedItem("");
    }}
