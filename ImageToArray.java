
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
        int n = args.length;
        for (int i = 0; i < n; i++) {
            String entry = args[i];
        
            String imagePath = "/Users/yufeihong/Desktop/ece243/243Project/utility/numbers/" + entry;
            String name = entry.substring(0, entry.lastIndexOf("."));
            try {
                BufferedImage image = ImageIO.read(new File(imagePath));

                int width = image.getWidth();
                int height = image.getHeight();
                System.out.println("size : " + width + " * " + height + "\n");
                 if (width > 320 || height > 240) {
                     System.out.println("图片过大");
                     return;
                 }
                FileWriter writer = new FileWriter("/Users/yufeihong/Desktop/ece243/243Project/utility/numbers/" + name + ".h");

                writer.write("#ifndef " + name + "_H\n#define " + name + "_H\n\n");
                writer.write("const unsigned short " + name + "[240][320] = {\n");

                for (int y = 0; y < 240; y++) {
                    writer.write("    {");
                    for (int x = 0; x < 320; x++) {
                        if (x >= width || y >= height) {
                            writer.write(String.format("0x0000"));
                            if (x < 320 - 1) writer.write(", ");
                            continue;
                        }
                        int rgb = image.getRGB(x, y);
                        int red = (rgb >> 16) & 0xFF;
                        int green = (rgb >> 8) & 0xFF;
                        int blue = rgb & 0xFF;
                        int threshold_white = 200;
                        int threshold_black = 50;
                        int pixel565 = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
                        if (red >= threshold_white && (green >= threshold_white && blue >= threshold_white)) {
                            pixel565 = 0xffff;
                        }
                        if (red <= threshold_black && green <= threshold_black && blue <= threshold_black) {
                            pixel565 = 0x0000;
                        }
                        writer.write(String.format("0x%04X", pixel565));
                        if (x < 320 - 1) writer.write(", ");
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
}
