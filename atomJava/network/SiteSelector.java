package network;

import java.util.*;
import java.net.*;
import java.applet.*;
import java.awt.*;
import javax.swing.*;
import javax.swing.event.*;

public class SiteSelector extends JApplet{
	private HashMap<String, URL> sites;
	private ArrayList<String> siteNames;
	private JList chooser;
	public SiteSelector(){

	}

	public void init(){
		sites = new HashMap<String, URL>();
		siteNames = new ArrayList<>();

		// obtain the URL list
		ObtainURLfromHTML();

		chooser = new JList(siteNames.toArray());
		chooser.addListSelectionListener(
		new ListSelectionListener(){
			@Override
			public void valueChanged(ListSelectionEvent e){
				// get the site name
				Object obj = chooser.getSelectedValue();

				// get the corresponding URL
				URL newDoc = sites.get(obj);

				// get the applet Context
				AppletContext browser = getAppletContext();

				// open the things
				browser.showDocument(newDoc);

			}
		}
		);

		// add the applet
		add(new JLabel("choose a web site"), BorderLayout.NORTH);
		add(new JScrollPane(chooser), BorderLayout.CENTER);
	}

	private void ObtainURLfromHTML(){
		String title;
		String location;
		URL url;

		int count = 0;
		title = getParameter("title" + count);
		while(title != null){
			location = getParameter("location" + count);

			try{
				url = new URL(location);
				sites.put(title, url);
				siteNames.add(title);
			}
			catch (MalformedURLException urlExcept){
				urlExcept.printStackTrace();
			}

			count++;
			title = getParameter("title" + count);
		}
	}
}
