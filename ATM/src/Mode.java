import javax.swing.*;

/**
 * Created by phi on 31/05/16.
 */
abstract public class Mode {
    private boolean enabled;
    private JTextArea text;
    private JButton[] buttons;
    private boolean executing;
    abstract public void execute();

    public Mode(JTextArea text, JButton[] buttons){
        setText(text);
        setButtons(buttons);
        setEnabled(false);
        setExecuting(false);
    }
    public boolean isEnabled() {
        return enabled;
    }

    public void setEnabled(boolean enabled) {
        this.enabled = enabled;
    }

    public JTextArea getText() {
        return text;
    }

    public void setText(JTextArea text) {
        this.text = text;
    }

    public JButton[] getButtons() {
        return buttons;
    }

    public void setButtons(JButton[] buttons) {
        this.buttons = buttons;
    }

    public boolean isExecuting() {
        return executing;
    }

    public void setExecuting(boolean executing) {
        this.executing = executing;
    }
}
