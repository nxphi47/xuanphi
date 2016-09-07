
import javax.script.ScriptException;
import javax.swing.*;
import java.util.*;
import java.io.*;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

class Main {
	public static FileSequential app;

	public static void main(String[] args) throws ScriptException {
		//PaintAppForm appForm = new PaintAppForm();

		GUIBouncing app = new GUIBouncing();


		BouncingThreading thread = new BouncingThreading(app.getBallList(), app.getPanel());

		ExecutorService service = Executors.newCachedThreadPool();
		service.execute(thread);
		service.shutdown();


        /*
        BouncingSwingWorker worker = new BouncingSwingWorker(app.getBallList(), app.getPanel());
        worker.execute();
        */
		/*
		Properties props = new Properties();

		props.setProperty("color", "blue");
		props.setProperty("width", "100%");
		props.setProperty("height", "200%");

		printProperties(props);
		saveProperties(props);

		Properties newProps = new Properties();
		loadProperties(newProps);
		printProperties(newProps);
		*/

	}

	private static void saveProperties(Properties props){
		try{
			FileOutputStream outputStream = new FileOutputStream("props.dat");
			props.store(outputStream, "Samples properties");
			outputStream.close();
			System.out.printf("after saving");
			// print the properties
		}
		catch (IOException io){
			System.err.println("Error in writing the properties");
			io.printStackTrace();
		}
	}

	private static void loadProperties(Properties props){
		try {
			FileInputStream inputStream = new FileInputStream("props.dat");
			props.load(inputStream);
			inputStream.close();
			System.out.printf("After ");
			//print it
		}
		catch (IOException io){
			System.err.println("Error in reading the file");
			io.printStackTrace();
		}
	}

	private static void printProperties(Properties props){
		Set<Object> keys = props.keySet();
		for (Object key: keys){
			System.out.printf("%-10s%10s\n", key, props.getProperty((String) key));
		}
		System.out.printf("\n");
	}

	private static void createMap(Map<String, Integer> map){
		Scanner scanner = new Scanner(System.in);
		System.out.printf("enter string: ");
		String input = scanner.nextLine();

		String[] tokens = input.split(" ");

		for (String x: tokens){
			String word = x.toLowerCase();

			if (map.containsKey(word)){
				map.put(word, map.get(word) + 1);
			}
			else {
				map.put(word, 1);
			}
		}
	}

	private static void displayMap(Map<String, Integer> map){
		Set<String> keys = map.keySet();
		TreeSet<String> sortedKeys = new TreeSet<>(keys);

		System.out.printf("\nMap contains: \nKey\t\tValue\n");

		for (String key: sortedKeys){
			System.out.printf("%-10s%10s\n", key, map.get(key));

		}
	}

	public static void testArrayList(){
		ArrayList<Integer> arrayList = new ArrayList<>(100);
		arrayList.add(2);
		arrayList.add(3);
		arrayList.add(6);
		System.out.printf(String.valueOf(arrayList.get(arrayList.size() - 1)));
		System.out.printf(String.valueOf(arrayList.get(arrayList.size() - 2)));
	}

	public static void write(){
		app.openWrite();
		app.addRecords();
		app.closeFileWrite();
	}

	public static void read(){
		app.openRead();
		app.readRecords();
		app.closeFileRead();
	}


	public static void analyzePath(){
		/*

		File name = new File(path);
		if (name.exists()){
			System.out.printf("%s%s\n%s\n%s\n%s\n%s\n%s\n%s",
					name.getName(), " exits",
					(name.isFile() ? "is a file": " not a file"),
					(name.isDirectory() ? " is directory": " not a directory"),
					(name.isAbsolute() ? " is absolute": " not absolute"),
					(name.getPath()),
					(name.getAbsolutePath()),
					(name.getParent())
			);

			if (name.isDirectory()) {
				String[] list = name.list();
				System.out.printf("\nContainer: \n");
				for (String x: list) {
					System.out.printf("%s\n", x);
				}
			}
		}
		else {
			System.out.printf("Not exists");
		}
		*/
		FileManipulate app = new FileManipulate();
		app.openToWrite();
		app.addRecords();
		app.closeFileOpen();

	}
}
