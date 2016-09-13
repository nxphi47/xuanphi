package network;

import java.util.*;
import java.net.*;
import java.applet.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import javax.swing.*;
import javax.swing.event.*;

public class ReadServerFile extends JFrame{
	private JTextField text;
	private JEditorPane pane;

	public ReadServerFile(){
		super("Test server reader");

		// adding component
		text = new JTextField();
		pane = new JEditorPane();
		pane.setEditable(false);
		add(text, BorderLayout.NORTH);
		add(new JScrollPane(pane), BorderLayout.CENTER);

		// configure components
		text.addActionListener(
			new ActionListener(){
				@Override
				public void actionPerformed(ActionEvent e){
					getPage(e.getActionCommand());
				}
			}
		);

		// when hyperlink in the pane is clicked
		pane.addHyperlinkListener(
			new HyperlinkListener(){
				@Override
				public void hyperlinkUpdate(HyperlinkEvent e){
					// if the event type is clicked, ENTERED if hovered, and EXITED
					if (e.getEventType() == HyperlinkEvent.EventType.ACTIVATED) {
						getPage(e.getURL().toString());
					}
				}
			}
		);

		// setup final for JFrame
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setSize(500,500);
		setVisible(true);
	}

	public void getPage(String location){
		try{
			pane.setPage(location);
			text.setText(location);
		}
		catch (IOException io){
			// MalformedURLException is throw as a subclass of IOException
			JOptionPane.showMessageDialog(this, "Error, url not found", "BAD URL",
				JOptionPane.ERROR_MESSAGE);
		}
	}
}
