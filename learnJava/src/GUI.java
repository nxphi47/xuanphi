import javax.swing.*;
import javax.swing.border.Border;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import java.awt.*;
import java.awt.Font;
import java.awt.event.*;

/**
 * Created by nxphi on 7/26/16.
 */
public class GUI extends JFrame{
	private JLabel label1;
	private JLabel label2;
	private JLabel label3;
	// for text field function
	private JTextField textField1;
	private JTextField textField2;
	private JTextField textField3;
	private JPasswordField passwordField;
	// for add buttons
	private JButton plainButton;
	private JButton fancyButton;
	// for add checkbox
	private JTextField textCheckboxNRadio;
	private JCheckBox boldCheckbox;
	private JCheckBox italicCheckbox;
	// for add radio
	private Font fontBold;
	private Font fontItalic;
	private Font fontPlain;
	private Font fontBoth;
	private JRadioButton radioBold;
	private JRadioButton radioItalic;
	private JRadioButton radioBoth;
	private JRadioButton radioPlain;
	private ButtonGroup radioGroup;
	// for combobox
	private JComboBox<Icon> comboBoxImage;
	private JLabel comboLabel;
	private static final String[] comboImageNames= {"bug.png", "bug1.png", "bug2.png"};
	private Icon[] comboIcon = {
		new ImageIcon(comboImageNames[0]),
		new ImageIcon(comboImageNames[1]),
		new ImageIcon(comboImageNames[2])
	};
	// for JList
	private JList<String> jListColor;
	private static final String[] listColorNames = {"Red", "Green", "Blue", "Cyan", "Black", "White"};
	private Color[] listColor = {Color.RED, Color.green, Color.blue, Color.cyan, Color.black, Color.WHITE};

	// for Key event
	private String lineKey1 = "";
	private String lineKey2 = "";
	private String lineKey3 = "";
	private JTextArea textAreaKey;

	// layout grid
	private JButton[] gridButtons;
	private static final String[] gridButtonNames = {
		"one", "two", "three", "four", "five", "six"
	};
	private boolean toggle;
	private GridLayout grid1;
	private GridLayout grid2;

	// for Graphics2D
	private ShapePanel shapePanel;


	// expensive task runnable on the event dispatch thread
	private BackgroundCalculation fibonacciCalculator;
	private JLabel fiboResultLabel;
	private JTextField fiboTextField;


	public GUI(){
		super("Testing JFrame");

		// main operation

		addFibonacciCalculation();
		// after setting
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setSize(500,500);
		setVisible(true);
	}

	// for fibonacci calculation
	public void addFibonacciCalculation(){
		getContentPane().removeAll();
		setLayout(new FlowLayout());

		fiboTextField = new JTextField(20);
		fiboResultLabel = new JLabel("enter the number to cal");
		fiboTextField.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				fiboResultLabel.setText("please wait a moment");
				BackgroundCalculation calculation = new BackgroundCalculation(Integer.parseInt(fiboTextField.getText()), fiboResultLabel);
				calculation.execute();

			}
		});

		add(fiboTextField);
		add(fiboResultLabel);
	}

	// for JSlider demonstration
	private class OvalPanel extends JPanel{
		private int diameter = 10;
		@Override
		public void paintComponent(Graphics g){
			super.paintComponent(g);
			g.fillOval(10, 10, diameter, diameter);
		}

		public void setDiameter(int diameter){
			this.diameter = (diameter >= 0)?diameter:10;
			repaint();
		}

		public Dimension getPreferredDimension(){
			return new Dimension(200,200);
		}

		public Dimension getMinimumSize(){
			return getPreferredSize();
		}
	}
	private JSlider slider;
	private OvalPanel ovalPanel;

	public void addJSlider(){
		getContentPane().removeAll();
		ovalPanel = new OvalPanel();
		ovalPanel.setBackground(Color.YELLOW);

		slider = new JSlider(SwingConstants.HORIZONTAL, 0, 200, 10);
		slider.setMajorTickSpacing(10); //create tick every 10
		slider.setPaintTicks(true); //paint the ticks on the sliders

		slider.addChangeListener(new ChangeListener() {
			@Override
			public void stateChanged(ChangeEvent e) {
				ovalPanel.setDiameter(slider.getValue());
			}
		});

		add(ovalPanel, BorderLayout.CENTER);
		add(slider, BorderLayout.SOUTH);
	}

	//colorsss
	private class PaintPanel extends JPanel{
		@Override
		protected void paintComponent(Graphics g) {
			super.paintComponent(g);
			g.setColor(Color.RED);
			g.drawLine(5,30, 380, 30);

			g.setColor(Color.BLUE);
			g.drawRect(5, 40, 90, 55);
			g.fillRect(100, 40, 90, 55);

			g.setColor(Color.CYAN);
			g.fillRoundRect(195, 40, 90, 55, 50, 20);

		}
	}

	public void addShapePanel(){
		getContentPane().removeAll();
		shapePanel = new ShapePanel();
		add(shapePanel, BorderLayout.CENTER);
	}

	// color
	private PaintPanel paintColor;
	private Color color;
	private JButton colorChooserButton;

	public void addColorPresentation(){
		getContentPane().removeAll();
		paintColor = new PaintPanel();
		color = Color.WHITE;

		// new Color choose
		colorChooserButton = new JButton("Choose new Color");
		colorChooserButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				color = JColorChooser.showDialog(GUI.this, "choose a color", color);
				if (color == null){
					color = Color.WHITE;
				}
				paintColor.setBackground(color);
			}
		});

		add(paintColor, BorderLayout.CENTER);
		add(colorChooserButton, BorderLayout.SOUTH);
	}

	public void addTextField(){
		getContentPane().removeAll();
		setLayout(new FlowLayout());

		textField1 = new JTextField(10);
		add(textField1);

		textField2 = new JTextField(20);
		textField2.setBackground(Color.GREEN);
		add(textField2);

		textField3 = new JTextField("Uneditable", 16);
		textField3.setEditable(false);
		add(textField3);

		passwordField = new JPasswordField("Hidden text");
		add(passwordField);

		TextFieldHandler handler = new TextFieldHandler();
		textField1.addActionListener(handler);
		textField2.addActionListener(handler);
		textField3.addActionListener(handler);
		passwordField.addActionListener(handler);
	}

	// when text field is entered
	private class TextFieldHandler implements ActionListener{
		@Override
		public void actionPerformed(ActionEvent event){
			String string = "";
			if (event.getSource() == textField1){
				string = String.format("Text field1: %s", event.getActionCommand());
			}
			else if (event.getSource() == textField2){
				string = String.format("text field 2: %s", event.getActionCommand());
			}
			else if (event.getSource() == textField3){
				string = String.format("text field 3: %s", event.getActionCommand());
			}
			else if (event.getSource() == passwordField){
				string = String.format("pass: %s", event.getActionCommand());
			}

			JOptionPane.showMessageDialog(null, string, "Infor", JOptionPane.INFORMATION_MESSAGE);
		}
	}

	public void addLabels(){
		// remove everything, to be set at every method for new frame
		getContentPane().removeAll();

		setLayout(new FlowLayout());
		label1 = new JLabel("Label text");
		label1.setToolTipText("Labels");
		add(label1);

		Icon bugIcon = null;
		try {
			/* the following is only for jar file output
			InputStream inImage = getClass().getClassLoader().getResourceAsStream("bug.png");
			byte[] arr = new byte[100000];
			int resultREad = inImage.read(arr);
			bugIcon = new ImageIcon(arr);
			*/
			bugIcon = new ImageIcon("bug.png");
		} catch (NullPointerException e) {
			System.err.println("Null pointer for open bug.png");
			System.err.println(getClass().getResource("")); //URL of .class folder
			//JOptionPane.showMessageDialog(this,getClass().getResource(""), "file open error", JOptionPane.ERROR_MESSAGE);
			System.exit(1);
		} catch (Exception e){
			e.printStackTrace();
		}

		label2 = new JLabel("Label text + icon", bugIcon, SwingConstants.LEFT);
		label2.setToolTipText("Label2");
		add(label2);

		label3 = new JLabel();
		label3.setText("Icon+ text at bottom");
		label3.setIcon(bugIcon);
		label3.setHorizontalTextPosition(SwingConstants.CENTER);
		label3.setVerticalTextPosition(SwingConstants.BOTTOM);
		label3.setToolTipText("Label3");
		add(label3);


		// repaint
		getContentPane().repaint();
	}

	public void addButtons(){
		getContentPane().removeAll();
		setLayout(new FlowLayout());

		plainButton = new JButton("PLain button");
		add(plainButton);

		Icon bugIcon = null;
		try{
			bugIcon = new ImageIcon("bug.png");
		}
		catch (NullPointerException nullpointer){
			System.err.println("Not found bug.png or pointer not created");
			JOptionPane.showMessageDialog(this, "bug.png not found", "Error", JOptionPane.ERROR_MESSAGE);

		}
		catch (Exception e){
			e.printStackTrace();
		}

		fancyButton = new JButton("fancy button", bugIcon);
		fancyButton.setRolloverIcon(bugIcon);
		add(fancyButton);

		class buttonHandler implements ActionListener{
			@Override
			public void actionPerformed(ActionEvent e) {
				JOptionPane.showMessageDialog(GUI.this, String.format("You pressed: %s", e.getActionCommand()));
			}
		}

		plainButton.addActionListener(new buttonHandler());
		fancyButton.addActionListener(new buttonHandler());
	}

	// add checkbox
	public void addCheckboxes(){
		getContentPane().removeAll();
		setLayout(new FlowLayout());

		textCheckboxNRadio = new JTextField("Hello from the other side", 24);
		textCheckboxNRadio.setFont(new Font("Serif", Font.PLAIN, 14)); // style, type, size
		add(textCheckboxNRadio);

		boldCheckbox = new JCheckBox("Bold");
		italicCheckbox = new JCheckBox("Italic");
		add(boldCheckbox);
		add(italicCheckbox);

		// handling
		class CheckboxHanlder implements ItemListener{
			@Override
			public void itemStateChanged(ItemEvent e){
				Font font = null;
				if (boldCheckbox.isSelected() && italicCheckbox.isSelected()){
					font = new Font("Serif", Font.BOLD + Font.ITALIC, 14);
				}
				else if (boldCheckbox.isSelected()){
					font = new Font("Serif", Font.BOLD, 14);
				}
				else if (italicCheckbox.isSelected()){
					font = new Font("Serif", Font.ITALIC, 14);
				}
				else {
					font = new Font("Serif", Font.PLAIN, 14);
				}
				textCheckboxNRadio.setFont(font);
			}
		}
		// register handler
		boldCheckbox.addItemListener(new CheckboxHanlder());
		italicCheckbox.addItemListener(new CheckboxHanlder());

	}

	public void addRadio(){
		getContentPane().removeAll();
		setLayout(new FlowLayout());

		fontBold = new Font("Serif", Font.BOLD, 14);
		fontItalic = new Font("Serif", Font.ITALIC, 14);
		fontBoth = new Font("Serif", Font.ITALIC + Font.BOLD, 14);
		fontPlain = new Font("Serif", Font.PLAIN, 14);

		textCheckboxNRadio = new JTextField("See style change herer", 25);
		add(textCheckboxNRadio);

		radioBold = new JRadioButton("Bold", false);
		radioBoth = new JRadioButton("Bold and Italic", false);
		radioItalic = new JRadioButton("Italic", false);
		radioPlain = new JRadioButton("plain", true);
		radioGroup = new ButtonGroup();
		radioGroup.add(radioBold);
		radioGroup.add(radioBoth);
		radioGroup.add(radioItalic);
		radioGroup.add(radioPlain);
		add(radioPlain);
		add(radioBold);
		add(radioItalic);
		add(radioBoth);

		class radioHanlder implements ItemListener{
			private Font font;
			public radioHanlder(Font f){
				font = f;
			}
			@Override
			public void itemStateChanged(ItemEvent e) {
				textCheckboxNRadio.setFont(font);
			}
		}

		radioItalic.addItemListener(new radioHanlder(fontItalic));
		radioBold.addItemListener(new radioHanlder(fontBold));
		radioBoth.addItemListener(new radioHanlder(fontBoth));
		radioPlain.addItemListener(new radioHanlder(fontPlain));
	}

	// combobox
	public void addComboBox(){
		getContentPane().removeAll();
		setLayout(new FlowLayout());

		comboBoxImage = new JComboBox<>(comboIcon);
		comboBoxImage.setMaximumRowCount(2);
		comboBoxImage.addItemListener(new ItemListener() {
			@Override
			public void itemStateChanged(ItemEvent e) {
				if (e.getStateChange() == ItemEvent.SELECTED){
					comboLabel.setIcon(comboIcon[comboBoxImage.getSelectedIndex()]);
				}
			}
		});

		add(comboBoxImage);
		comboLabel = new JLabel(comboIcon[0]);
		add(comboLabel);
	}

	// grid layout
	public void addGridLayout(){
		getContentPane().removeAll();
		toggle = true;
		grid1 = new GridLayout(2,3,5,5);// 2x3 5 gap
		grid2 = new GridLayout(3,2);
		setLayout(grid1);

		gridButtons = new JButton[gridButtonNames.length];

		class GridButtonHandler implements ActionListener {
			@Override
			public void actionPerformed(ActionEvent e) {
				if (toggle){
					GUI.this.setLayout(grid1);
				}
				else {
					GUI.this.setLayout(grid2);
				}
				toggle = !toggle;
				getContentPane().validate();
			}
		}

		GridButtonHandler gridButtonHandler = new GridButtonHandler();

		for (int i = 0; i < gridButtonNames.length; i++){
			gridButtons[i] = new JButton(gridButtonNames[i]);
			gridButtons[i].addActionListener(gridButtonHandler);
			add(gridButtons[i]);
		}
	}

	// JList
	public void addListColor(){
		getContentPane().removeAll();
		setLayout(new FlowLayout());

		jListColor = new JList<>(listColorNames);
		jListColor.setVisibleRowCount(3);
		jListColor.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

		add(new JScrollPane(jListColor));
		jListColor.addListSelectionListener(new ListSelectionListener() {
			@Override
			public void valueChanged(ListSelectionEvent e) {
				getContentPane().setBackground(listColor[jListColor.getSelectedIndex()]);
			}
		});
	}

	// set of function for key event
	public void addKeyEvent(){
		getContentPane().removeAll();
		setLayout(new FlowLayout());

		textAreaKey = new JTextArea(10,15);
		textAreaKey.setText("press any key");
		textAreaKey.setEditable(false);
		textAreaKey.setDisabledTextColor(Color.black);
		add(textAreaKey);
		addKeyListener(new KeyListener() {
			@Override
			public void keyTyped(KeyEvent e) {
				lineKey1 = String.format("Key typed: %s", KeyEvent.getKeyText(e.getKeyCode()));
				setLine2and3(e);
			}

			@Override
			public void keyPressed(KeyEvent e) {
				lineKey1 = String.format("Key pressed: %s", KeyEvent.getKeyText(e.getKeyCode()));
				setLine2and3(e);
			}

			@Override
			public void keyReleased(KeyEvent e) {
				lineKey1 = String.format("Key released: %s", KeyEvent.getKeyText(e.getKeyCode()));
				setLine2and3(e);
			}
		});
	}
	public void setLine2and3(KeyEvent e){
		lineKey2 = String.format("This is %s action key", e.isActionKey()?"an":"not an");
		String temp = KeyEvent.getKeyModifiersText(e.getModifiers());
		lineKey3 = String.format("modified key pressed: %s", (temp.equals(""))?"none":temp);
		textAreaKey.setText(String.format("%s\n%s\n%s", lineKey1, lineKey2, lineKey3));
	}


	public void test(){
		System.out.printf(String.valueOf(getClass().getResource("bug.png")));
	}
}
