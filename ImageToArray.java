
import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

public class ImageToArray {

    public static void main(String[] args) {

        if (args.length < 1) {
            System.out.println("path/to/ur/image");
            return;
        }

        String imagePath = args[0];
        String name = imagePath.substring(imagePath.lastIndexOf("/") + 1, imagePath.length() - 4);
        try {
            BufferedImage image = ImageIO.read(new File(imagePath));

            int width = image.getWidth();
            int height = image.getHeight();
            System.out.println("size : " + width + " * " + height + "\n");
            // if (width != 300 || height != 240) {
            //     System.out.println("非240*320");
            //     return;
            // }
            FileWriter writer = new FileWriter("/Users/yufeihong/Desktop/ece243/243Project/" + name + ".h");

            writer.write("#ifndef " + name + "_H\n#define " + name + "_H\n\n");
            writer.write("const unsigned short " + name+ "_image_pixels[240][320] = {\n");

            for (int y = 0; y < height; y++) {
                writer.write("    {");
                for (int x = 0; x < width; x++) {
                    int rgb = image.getRGB(x, y);
                    int red = (rgb >> 16) & 0xFF;
                    int green = (rgb >> 8) & 0xFF;
                    int blue = rgb & 0xFF;
                    int pixel565 = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
                    writer.write(String.format("0x%04X", pixel565));
                    if (x < width - 1) writer.write(", ");
                }
                writer.write("},\n");
            }
            writer.write("};\n\n#endif // "+ name + "_H\n");
            writer.close();

        } catch (IOException e) {
            System.out.println(e.getMessage());
        }
    }
}
