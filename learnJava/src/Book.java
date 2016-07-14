/**
 * Created by phi on 14/07/16.
 */
public enum Book {
    JHTP("Java how to programe", "2012"),
    CHTP("C how to program", "2007"),
    IW3HTP("Internet and world wide web", "2008");

    private final String title;
    private final String year;

    Book(String tt, String yy){
        title = tt;
        year = yy;
    }

    public String getTitle(){
        return title;
    }
    public String getYear(){
        return year;
    }
}
