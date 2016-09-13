package com.phi.icon;

import javax.swing.*;
import java.awt.*;
import java.awt.datatransfer.StringSelection;
import java.net.URL;

/**
 * Created by nxphi on 9/12/16.
 */
public class EmptyIcon extends ImageIcon{
	private ImageIcon icon;
	private final String pathName = "iconImage/EmptyIcon.png";
	private final URL url = this.getClass().getResource(pathName);
	public EmptyIcon() {
		super();

	}

	public ImageIcon get(){
		return icon;
	}
}
