import org.omg.PortableInterceptor.ACTIVE;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.Formatter;
import java.util.Scanner;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.atomic.DoubleAccumulator;

/**
 * Created by phi on 31/05/16.
 */
public class DataBase {
    private ArrayList<Account> data; // further develop, sort the number and use binarysearch
    private static int count;
    private File file;
    private String fileName;
    private Scanner input;
    private Formatter output;

    public DataBase(String fileName){
        data = new ArrayList<Account>();

        this.fileName = fileName;
        file = new File(this.fileName);
        try {
            input = new Scanner(file);
        } catch (FileNotFoundException e) {
            System.err.println("Cannot open the database file");
            System.exit(1);
        }
        inputFile();
    }

    public void inputFile(){
        // account = 1234 [gnuyen xuan phi] {123456} 456.2
        if (input == null){
            try {
                input = new Scanner(file);
            } catch (FileNotFoundException e) {
                System.err.println("Canot open the file to input");
            }
        }

        try {
            while (input.hasNext()){
                String info = input.nextLine();
                int number = Integer.parseInt(info.substring(0, info.indexOf('[')));
                String name = info.substring(info.indexOf('[') + 1, info.indexOf(']'));
                String pass = info.substring(info.indexOf('{') + 1, info.indexOf('}'));
                double bal = Double.parseDouble(info.substring(info.indexOf('}') + 1));
                Account newAcc = new Account(number, name, pass, bal);
                data.add(newAcc);
                count++;
            }
        } catch (Exception e){
            System.err.printf("Error when input the file %s", e);
            System.exit(1);
        }
    }

    public void outputFile(){
        if (input != null){
            input.close();
        }
        try {
            output = new Formatter(this.fileName);
        } catch (FileNotFoundException e) {
            System.err.println("Cannot output/open the file to output");
        }

        // starting outputing
        try {
            for (int i = 0; i < data.size(); i++){
                Account acc = data.get(i);
                output.format("%-6d[%s] {%s} %f\n", acc.getNumber(), acc.getName(), acc.getBalance(),
                        acc.getBalance());

            }
        } catch (IndexOutOfBoundsException e1){
            System.err.println("Arraylist out of bound when output file");
        } catch (Exception e2){
            System.err.println("Error when output the file");
            System.exit(1);
        }
    }

    public String getDataBase(){
        if (data == null){
            inputFile();
        }
        StringBuilder database = new StringBuilder();
        database.ensureCapacity(10000000);
        String info;
        try {
            for (int i = 0; i < data.size(); i++){
                Account acc = data.get(i);
                info = String.format("%-6d[%s] {%s} %f\n", acc.getNumber(), acc.getName(), acc.getPassword(),
                        acc.getBalance());
                database.append(info);
            }
        } catch (IndexOutOfBoundsException e){
            System.err.println("arraylist out of bound when get the database");
            System.exit(1);
        }

        return database.toString();
    }

    public Account getAccount(int num, String name){
        for (Account aData : data) {
            if (aData.getNumber() == num && aData.getName().equals(name)) {
                return aData;
            }
        }
        return null;
    }
    public Account getAccount(int num){
        for (Account acc: data){
            if (acc.getNumber() == num){
                return acc;
            }
        }
        return null;
    }
}
