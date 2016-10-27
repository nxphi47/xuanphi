import javax.swing.*;
import java.awt.*;
import java.awt.geom.Arc2D;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Rectangle2D;
import java.awt.geom.RoundRectangle2D;
import java.awt.image.BufferedImage;

/**
 * Created by nxphi on 9/1/16.
 */
public class ShapePanel extends JPanel{
	@Override
	public void paintComponent(Graphics g){
		super.paintComponent(g);

		Graphics2D graphics2D = (Graphics2D) g;
		graphics2D.setPaint(new GradientPaint(5, 30, Color.CYAN, 35, 100, Color.RED, true));

		graphics2D.fill(new Ellipse2D.Double(5, 30, 65, 100));

		graphics2D.setPaint(Color.RED);
		graphics2D.setStroke(new BasicStroke(20.0f));
		graphics2D.draw(new Rectangle2D.Double(80,30,65, 100));

		BufferedImage bufferedBackground = new BufferedImage(10, 10, BufferedImage.TYPE_INT_RGB);

		Graphics2D gg = bufferedBackground.createGraphics();

		gg.setColor(Color.YELLOW);
		gg.fillRect(0,0, 10, 10);
		gg.setColor(Color.BLACK);
		gg.fillRect(1,1,6,6);
		gg.setColor(Color.BLUE);
		gg.fillRect(1,1,3,3);
		gg.setColor(Color.RED);
		gg.fillRect(4,4,3,3);

		// paint the buffered background on the JFrame
		graphics2D.setPaint(new TexturePaint(bufferedBackground, new Rectangle(10,10)));
		graphics2D.fill(new RoundRectangle2D.Double(155,30, 75, 100, 50, 50));

		//draw 2D pie arc
		graphics2D.setPaint(Color.WHITE);
		graphics2D.setStroke(new BasicStroke(6.0f));
		graphics2D.draw(new Arc2D.Double(240, 30, 75, 100,0,270, Arc2D.PIE));

	}
}
