import javax.swing.*;
import java.awt.*;

/**
 * Created by phi on 31/05/16.
 */
public class Keypad extends JPanel {
    private JButton[] buttons;
    private static final String[] buttonNames = {
            "1", "2", "3", "4", "5", "6", "7", "8", "9", ".", "0", "#",
            "^", "OK", "v", "Cancel",
            "Back"
    };
    private JPanel panelNumber;

    private JPanel panelControl;
    private JPanel panelWthBack;

    private void setPanelNumber(){
        panelNumber = new JPanel(new GridLayout(4,3,5,5));
        for (int i = 0; i < 12; i++){
            panelNumber.add(buttons[i]);
        }
    }
    private void setPanelWthBack(){
        panelWthBack = new JPanel(new GridLayout(2,2,5,5));
        for (int i =12; i < buttonNames.length - 1; i++){
            panelWthBack.add(buttons[i]);
        }
    }
    private void setPanelControl(){
        panelControl = new JPanel(new GridLayout(2,1,5,5));
        panelControl.add(panelWthBack);
        panelControl.add(buttons[buttonNames.length - 1]);
    }
    private void setPanel(){
        setLayout(new GridLayout(1,2,15,15));
        add(panelNumber);
        add(panelControl);
    }

    public Keypad(){
        buttons = new JButton[buttonNames.length];
        for (int i = 0; i < buttonNames.length; i++){
            buttons[i] = new JButton(buttonNames[i]);
        }

        setPanelWthBack();
        setPanelControl();
        setPanelNumber();
        setPanel();
    }

    public JButton[] getButtons(){
        return buttons;
    }

}
