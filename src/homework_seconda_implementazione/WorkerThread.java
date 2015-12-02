package homework_seconda_implementazione;


public class WorkerThread
{
    // stessi metodi, reimplementare allocando meno oggetti possibili
    
    private Master master;
    private AllStringGenerator asg;
    private StringGenerator sg;
    private MD5 hasher;

    public WorkerThread (Master m, AllStringGenerator asg, StringGenerator sg) {
        this.master = m;
        this.asg = asg;
        this.sg = sg;
        this.hasher = new MD5();
    }

        public void run()
    {
        HashSet<byte[]> passwords = this.master.getPasswords();
        byte[] root;
        byte[] word;
        while (!passwords.isEmpty()) {
            root = this.asg.getNextWord();
            word = Arrays.copyOf(root,root.length + this.sg.length);
            while (this.sg.hasWords()) {
                sg.getNextWord(word,root.length);
                byte[] hash = this.hasher.hash(word);
                for (byte[] password : passwords) {
                    if (Arrays.equals(password,hash)) {
                        this.master.foundPassword(hash, word);
                    }
                }
            }
            this.sg.reset();
            passwords = this.master.getPasswords();
        }
    }
}
