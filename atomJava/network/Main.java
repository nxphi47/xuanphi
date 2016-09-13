package network;

import java.util.*;
import java.net.*;
import java.io.*;
import java.applet.*;
import java.awt.*;
import java.awt.*;
import javax.swing.*;
import javax.swing.event.*;

public class Main{
	public static void main(String[] args) {
		if (args[0].equals("-server")) {
			Server server = new Server();
			server.runServer();
		}
		else if (args[0].equals("-client")) {
			// client on
			// InetAddress =
			String inetAddress = "127.0.0.1"; // localhost
			if (args.length > 1) {
				inetAddress = args[1];
			}
			Client client = new Client(inetAddress);
			client.runClient();
		}
		else{
			System.err.println("Wrong argument input\n");
		}
	}
}
