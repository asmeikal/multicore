package homework_seconda_implementazione;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;

public class Master
{
    private ArrayList<byte[]> hashes;
    private int length;
    private GeneratorFactory gf;
    private AllStringGenerator asg;
    private WorkerThread[] workerThreads;
    private HashMap<byte[], byte[]> passwordMap;


    public Master(ArrayList<byte[]> hashes, int n_thread, int length) {
        this.hashes = hashes;
        this.workerThreads = new WorkerThread[n_thread];
        this.length = length;
        this.gf = new GeneratorFactory("abcdefghijklmnopqrstuvwxyz0123456789".getBytes(), this.length);
        this.asg = gf.getASG();
        this.passwordMap = new HashMap<byte[], byte[]>();
    }


    public void work() throws InterruptedException {
        MD5 md5 = new MD5();

        // controllare le prime parole in sequenziale
        byte[] word;
        for (int i = 1; i <= this.length; ++i) {
            word = new byte[i];
            StringGenerator sg = this.gf.getSG(i);
            while (sg.hasWords()) {
                sg.getNextWord(word,0);
                byte[] hash = md5.hash(word);
                for (byte[] h : hashes) {
                    if (Arrays.equals(h, hash)) {
                        this.foundPassword(hash, word);
                    }
                }
            }
        }

        // il master fa lavorare i thread

        for (int i = 0; i < this.workerThreads.length; i++) {
            workerThreads[i] = new WorkerThread(this, this.asg, gf.getSG());
            workerThreads[i].start();
        }
        for (int i=0; i < workerThreads.length; i++) {
            workerThreads[i].join();
        }

        System.out.println(this.passwordMap);

    }

    synchronized public void foundPassword(byte[] hash, byte[] pass)
    {
        ArrayList<byte[]> arr = new ArrayList<byte[]>();
        
        for (byte[] el : this.hashes) {
            if (!Arrays.equals(hash, el)) {
                arr.add(el);
            }
        }
        this.hashes = arr;
        this.passwordMap.put(hash,pass);
        
    }

    synchronized public ArrayList<byte[]> getPasswords() {
        ArrayList<byte[]> copy = new ArrayList<byte[]>();
        copy.addAll(this.hashes);
        return copy;
    }
}
