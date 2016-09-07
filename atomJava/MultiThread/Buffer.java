package MultiThread;
public interface Buffer{
    public void set(int x) throws InterruptedException;
    public int get() throws InterruptedException;
}
