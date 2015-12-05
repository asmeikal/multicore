package homework_seconda_implementazione;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

public class Main {

    public static void main(String[] args) throws IOException, InterruptedException {

        Path path = Paths.get(args[0]);
        if (!Files.isRegularFile(path) || !path.toString().endsWith("txt")) {
            System.out.println("Il file non è regolare oppure non è un file di testo");
            return;
        }

        final String alphabet = "abcdefghijklmnopqrstuvwxyz0123456789";
        int processors = Runtime.getRuntime().availableProcessors();
        List<String> list = Files.readAllLines(path);
        ArrayList<String> passwords = new ArrayList<String>(list);

        Master master = new Master(passwords, alphabet, processors, 2);
        Map<String,String> map = master.work();

        for (String key : map.keySet()) {
            System.out.println(key + " = MD5("+map.get(key)+")");
        }
    }
}
