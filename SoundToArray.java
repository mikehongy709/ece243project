import javax.sound.sampled.*;
import java.io.*;

public class SoundToArray {
    public static void main(String[] args) {
        if (args.length != 1) {
            System.out.println("Usage: java SoundToArray <file>");
        }

        String path = args[0];
        String filePath = "/Users/yufeihong/Desktop/ece243/243Project/audio/" + path; 
        String name = path.substring(0, path.lastIndexOf("."));

        try {
            File audioFile = new File(filePath);
            AudioInputStream audioInputStream = AudioSystem.getAudioInputStream(audioFile);
            AudioFormat format = audioInputStream.getFormat();

            // 确保音频格式是 32-bit PCM
            int formatSize = format.getSampleSizeInBits() / 8;
            System.out.println(formatSize);

            byte[] audioBytes = convertAudioToBytes(audioInputStream);
            int[] audioData = bytesToInts(audioBytes, format, formatSize);

            FileWriter writer = new FileWriter("/Users/yufeihong/Desktop/ece243/243Project/audio/" + name + ".txt");
            writer.write("int " + name + "[] = { \n");
            for (int i = 0; i < audioData.length; i++) {
                if (i == audioData.length - 1) {
                    writer.write(String.format("0x%08X", audioData[i]));
                    writer.write("}; \n\n");
                } else {
                    if ((i + 1) % 4 == 0) {
                        writer.write(String.format("0x%08X", audioData[i]));
                        writer.write(", \n");
                    } else {
                        writer.write(String.format("0x%08X", audioData[i]));
                        writer.write(", ");
                    }
                }
            }
            writer.write("int " + name + "_size = " + audioData.length + ";");
            writer.close();
//            for (int i = 0; i < audioData.length; i++) {
//                //System.out.println(audioData[i]);
//                //System.out.println("\n");
//            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    // 读取音频数据到 byte[] 数组
    public static byte[] convertAudioToBytes(AudioInputStream audioInputStream) throws IOException {
        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
        byte[] buffer = new byte[1024];  // 创建一个缓冲区来存储每帧的数据
        int bytesRead;  // 存储每次读取的字节数
        while ((bytesRead = audioInputStream.read(buffer)) != -1) {  // 每次读取一帧数据
            byteArrayOutputStream.write(buffer, 0, bytesRead);  // 将读取到的数据写入 ByteArrayOutputStream
        }
        return byteArrayOutputStream.toByteArray();
    }
    public static int[] bytesToInts(byte[] audioBytes, AudioFormat format, int sampleSize) {
    int sampleCount = audioBytes.length / sampleSize;
    int[] audioData = new int[sampleCount];

    

    for (int i = 0; i < sampleCount; i++) {
        int sampleStart = i * sampleSize;
        int sampleValue = 0;
        if (sampleSize == 4) {
            sampleValue = ((audioBytes[sampleStart + 3] << 24) |
                    ((audioBytes[sampleStart + 2] & 0xFF) << 16) |
                    ((audioBytes[sampleStart + 1] & 0xFF) << 8)  |
                    (audioBytes[sampleStart] & 0xFF));
        } else if (sampleSize == 2) {
            sampleValue = ((audioBytes[sampleStart + 1] & 0xFF) << 8)  | (audioBytes[sampleStart] & 0xFF);;
        }

        audioData[i] = sampleValue;
    }
    return audioData;
}
}
