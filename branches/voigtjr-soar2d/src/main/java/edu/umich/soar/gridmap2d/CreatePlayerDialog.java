package edu.umich.soar.gridmap2d;

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.io.File;

import javax.swing.AbstractAction;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.filechooser.FileFilter;
import javax.swing.text.PlainDocument;

import edu.umich.soar.gridmap2d.config.PlayerConfig;
import edu.umich.soar.gridmap2d.core.Simulation;

public class CreatePlayerDialog extends JDialog {

	private static final long serialVersionUID = 8723092727211860088L;
	private final JTextField prod = new JTextField(40);
	private final JCheckBox spawn = new JCheckBox();
	private final JFrame frame;
	private static final String HUMAN = "<human>";
	
	CreatePlayerDialog(final JFrame frame, final Simulation sim) {
		super(frame, "Create Player", true);
		this.frame = frame;

		JPanel p = new JPanel(new BorderLayout());

		JPanel tl = new JPanel(new BorderLayout());
        tl.add(new JLabel("Productions:"), BorderLayout.WEST);
        prod.setDocument(new PlainDocument());
        prod.setText(HUMAN);
        tl.add(prod, BorderLayout.EAST);

		JPanel tr = new JPanel(new BorderLayout());
        final JButton soar = new JButton(new AbstractAction("Soar") {
			private static final long serialVersionUID = -2830685716905346567L;

			@Override
        	public void actionPerformed(ActionEvent e) {
        		choose();
        	}
        });
        tr.add(soar, BorderLayout.WEST);

        final JButton human = new JButton(new AbstractAction("Human") {
			private static final long serialVersionUID = 2856333655057412415L;

			@Override
        	public void actionPerformed(ActionEvent e) {
        		prod.setText(HUMAN);
        	}
        });
        tr.add(human, BorderLayout.EAST);

        JPanel top = new JPanel(new BorderLayout());
        top.add(tl, BorderLayout.WEST);
        top.add(tr, BorderLayout.EAST);
        
        p.add(top, BorderLayout.NORTH);
        
        JPanel bottom = new JPanel(new BorderLayout());
        
        spawn.setText("Spawn Debuggers");
        spawn.setSelected(sim.getCogArch().debug());
        bottom.add(spawn, BorderLayout.WEST);
        
        final JButton create = new JButton(new AbstractAction("Create Player") {
			private static final long serialVersionUID = -2830685716905346567L;

			@Override
        	public void actionPerformed(ActionEvent e) {
		        PlayerConfig playerConfig = new PlayerConfig();
		        if (prod.getText().isEmpty() || !prod.getText().equals(HUMAN)) {
		        	playerConfig.productions = prod.getText();
		        }
		        sim.getCogArch().setDebug(spawn.isSelected());
		        sim.createPlayer(playerConfig);
		        dispose();
        	}
        });
        bottom.add(create, BorderLayout.EAST);
 
        p.add(bottom, BorderLayout.SOUTH);

        setContentPane(p);
        pack();
	}
	
	private void choose() {
        JFileChooser chooser = new JFileChooser(System.getProperty("user.dir", "."));
        chooser.setFileFilter(new FileFilter() {

            @Override
            public boolean accept(File f)
            {
            	// TODO: case insensitive extension test
                return f.isDirectory() || f.isFile() && f.getAbsolutePath().endsWith("soar");
            }

            @Override
            public String getDescription()
            {
                return "Soar Files (*.soar)";
            }});
        
        if(JFileChooser.CANCEL_OPTION == chooser.showOpenDialog(frame))
        {
            return;
        }
        
        prod.setText(chooser.getSelectedFile().getAbsolutePath());
	}
}
