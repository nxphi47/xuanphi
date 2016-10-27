package MultiThread;

import java.util.*;
import java.util.concurrent.*;
import java.awt.*;
import javax.swing.*;
import java.awt.event.*;
import java.beans.*;

public class GUIPrimeFinding extends JFrame{
    private JTextField textField;
    private JTextArea textArea;
    private JButton getButton;
    private JButton cancelButton;
    private JLabel statusLabel;
    private JProgressBar bar;

    private JPanel southPanel;

    private PrimeCalculation calculator;

    public GUIPrimeFinding(){
        super("finding prime numbers");
        setLayout(new BorderLayout());

        // declare object
        southPanel = new JPanel();
        southPanel.setLayout(new FlowLayout());

        textField = new JTextField();
        textArea = new JTextArea();
        textArea.setEditable(false);

        getButton = new JButton("Get it!");
        cancelButton = new JButton("Cancel");

        statusLabel = new JLabel("Ready!");

        bar = new JProgressBar();
        bar.setStringPainted(true);

        // setup objects
        add(textField, BorderLayout.NORTH);
        add(new JScrollPane(textArea, ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS,
                ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER), BorderLayout.CENTER);

        southPanel.add(getButton);
        southPanel.add(cancelButton);
        southPanel.add(bar);
        southPanel.add(statusLabel);

        add(southPanel, BorderLayout.SOUTH);

        // setUplistener
        getButton.addActionListener(
            new ActionListener(){
                @Override
                public void actionPerformed(ActionEvent e){
                    bar.setValue(0);
                    textArea.setText("");
                    statusLabel.setText("In progress");

                    int number; // search primes upto this level
                    try{
                        number = Integer.parseInt(textField.getText());
                    }
                    catch (NumberFormatException except){
                        statusLabel.setText("Input format error");
                        return;
                    }
                    if (number <= 1) {
                        statusLabel.setText("input must > 1");
                        return;
                    }
                    // construct the PrimeCalculation SwingWorker
                    calculator = new PrimeCalculation(number, textArea, statusLabel, getButton, cancelButton);
                    calculator.addPropertyChangeListener(
                        new PropertyChangeListener(){
                            @Override
                            public void propertyChange(PropertyChangeEvent e){
                                if (e.getPropertyName().equals("progress")) {
                                    int newVal = (Integer) e.getNewValue();
                                    bar.setValue(newVal);
                                }
                            }
                        }
                    );
                    getButton.setEnabled(false);
                    cancelButton.setEnabled(true);
                    calculator.execute();

                }
            }
        );

        cancelButton.addActionListener(
            new ActionListener(){
                @Override
                public void actionPerformed(ActionEvent e){
                    calculator.cancel(true);
                }
            }
        );


        // done JFrame
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setSize(500,500);
        setVisible(true);
    }
}
