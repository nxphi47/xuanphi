package com.phi.game;

import com.phi.game.control.TestController;
import com.phi.game.gui.GameGUIForm;

/**
 * Created by nxphi on 9/14/16.
 */
public class StartGame {
	public static void main(String[] args) {
		GameGUIForm form = new GameGUIForm(new TestController(), "hello wordl");
	}
}
