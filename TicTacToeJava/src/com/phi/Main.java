package com.phi;

import java.io.IOException;

public class Main {
	public static boolean DEBUG = true;

	public static void main(String[] args) throws IOException, ClassNotFoundException, InterruptedException {
		// write your code here
		//EmptyIcon iconImage = new EmptyIcon();
		if (args[0].equals("server")){
			ServerLoop serverLoop = new ServerLoop();
		}
		else if (args[0].equals("client")){

			ClientTTT clientTTT = new ClientTTT();
		}
		else if (args[0].equals("test")){
			TestClient testClient = new TestClient();
		}

	}
}
