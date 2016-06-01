import javax.swing.*;

/**
 * Created by phi on 31/05/16.
 */
abstract public class Mode {
    protected boolean enabled;
    protected JTextArea text;
    protected JButton[] buttons;
    protected boolean executing;
    abstract public void execute();
    abstract public void handlingButton();

    private Account userAccount = null;
    public Account getUserAccount() {
        return userAccount;
    }

    public void setUserAccount(Account user) {
        if (getUserAccount() != null){
            getUserAccount().setAuthorized(false);
        }
        if (user == null){
            System.err.printf("%s: input user account = null\n", getClass());
            return;
        }
        this.userAccount = user;
        getUserAccount().setAuthorized(true);
    }

    private Mode superMode;

    public Mode(JTextArea text, JButton[] buttons){
        setText(text);
        setButtons(buttons);
        setEnabled(false);
        setExecuting(false);
        setSuperMode(null);
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

    public Mode getSuperMode() {
        return superMode;
    }

    public void setSuperMode(Mode superMode) {
        this.superMode = superMode;
    }
}
