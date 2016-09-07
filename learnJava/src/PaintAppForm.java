import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

/**
 * Created by nxphi on 8/29/16.
 */
public class PaintAppForm extends JFrame{
	private JPanel mainPanel;
	private JPanel northPanel;
	private JPanel centerPanel;
	private JToolBar controlToolBar;
	private JButton newPaintButton;
	private JButton savePaintButton;
	private JButton clearPaintButton;
	private JRadioButton pencilRadioButton;
	private JRadioButton eraseRadioButton;
	private JComboBox colorComboBox;
	private JComboBox shapeComboBox;
	private JLabel mousePositionLabel;

	// customized JComponent
	private PaintRealTimePanel paintRealTimePanel;

	public PaintAppForm(){
		super("Paint application");
		setContentPane(mainPanel);
		pack();
		// customize operation

		setupPaintPanel();
		setupControlPanelMain();

		// done operation
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setSize(800, 600);
		setVisible(true);
	}

	public void setupPaintPanel(){
		paintRealTimePanel = new PaintRealTimePanel();
		paintRealTimePanel.setMouseInfoLabel(mousePositionLabel);
		paintRealTimePanel.addInfoLabelListener();
		paintRealTimePanel.addPolygonDrawListener();
		centerPanel.add(paintRealTimePanel, BorderLayout.CENTER);
	}

	// setup the control panel
	public void setupControlPanelMain(){
		// setup the new button
		// setup save button
		// setup clearPaint button
		clearPaintButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				paintRealTimePanel.clearPaint();
			}
		});
		// setup pencil and erase group

		// setup color combobox
		// setup shape combobox
	}

	public void setupPencilAndErase(){

	}
}
