import java.io.*;
import java.net.*;
import java.util.Arrays;

class OracleNode {
    static int[][] cost = new int[26][26];
    static int n = 0;

    static int ON_PORT = 6000;
    
    static Socket[]           vnSock = new Socket[26];
    static DataInputStream[]  vnIn   = new DataInputStream[26];
    static DataOutputStream[] vnOut  = new DataOutputStream[26];
    static byte[][]           vnIp   = new byte[26][4];
    static int[]              vnPort = new int[26];

    static String ipToString(byte[] ip) {
        return (ip[0] & 0xFF) + "." + (ip[1] & 0xFF) + "."
             + (ip[2] & 0xFF) + "." + (ip[3] & 0xFF);
    }
    static int[][] copyCost() {
        int[][] copy = new int[26][26];

        for (int i = 0; i < 26; i++) {
            System.arraycopy(cost[i], 0, copy[i], 0, 26);
        }

        return copy;
    }

    static void restoreCost(int[][] oldCost) {
        for (int i = 0; i < 26; i++) {
            System.arraycopy(oldCost[i], 0, cost[i], 0, 26);
        }
    }
    static boolean readConnect(int i) throws IOException{
        vnIn[i].readFully(vnIp[i]);
        vnPort[i] = vnIn[i].readUnsignedShort();
        vnIn[i].skipNBytes(2);
        return true;
    }
    static void acceptVNs(ServerSocket server , String configPath) throws IOException{
        int i = 0;
        long lastCheck = System.currentTimeMillis();
        
        while(true){
            if (System.currentTimeMillis() - lastCheck >= 1000){
                checkConfigChanges(configPath, i);
                lastCheck = System.currentTimeMillis();
            }
            Socket s;

            try {
                s = server.accept();
            } catch (SocketTimeoutException e) {
                continue;
            }

            if (i >= n) {
                System.out.println("Rejecting extra connection");
                s.close();
                continue;
            }
            try{
                vnSock[i] = s;
                vnIn[i]   = new DataInputStream(s.getInputStream());
                vnOut[i]  = new DataOutputStream(s.getOutputStream());
                if (!readConnect(i)) {
                    s.close();
                    continue;  
                }
            } catch (IOException e) {
                System.err.println("Client dropped before CONNECT: " + e.getMessage());
                s.close();
                continue;
            }
            System.out.println((char)('A' + i) + " = " + ipToString(vnIp[i]) + ":" + vnPort[i] + "(TCP from " + s.getInetAddress().getHostAddress() + ":" + s.getPort() + ")");
            i++;
            if (i == n) {
                System.out.println("All " + n + " nodes connected.");

                for (int j = 0; j < n; j++) {
                    sendLinkState(j);
                }
            }
        }
    }
    static void checkConfigChanges(String path, int connected) throws IOException {
        int oldN = n;
        int[][] oldCost = copyCost();

        if (!parsefile(path)) {
            n = oldN;
            restoreCost(oldCost);
            System.err.println("Could not reload config file");
            return;
        }

        if (n != oldN) {
            System.err.println(
                "Changing the number of nodes is not supported at runtime"
            );

            n = oldN;
            restoreCost(oldCost);
            return;
        }

        if (!Arrays.deepEquals(oldCost, cost)) {
            System.out.println("Configuration changed!");
            printMatrix();

            if (connected == n) {
                for (int j = 0; j < n; j++) {
                    sendLinkState(j);
                }
            } else {
                System.out.println(
                    "Waiting for all nodes before sending LINK-STATE"
                );
            }
        }
    }
    static boolean parsefile(String path){
        int row = 0;
        try (BufferedReader br = new BufferedReader(new FileReader(path))) {
            String line;
            while ((line = br.readLine()) != null) {
                line = line.trim();
                if (line.isEmpty() || line.startsWith("#")) continue;

                String[] words = line.split("\\s+");
                if (row == 0) {
                    n = words.length + 1;
                    for (int i = 0; i < 26; i++)
                        for (int k = 0; k < 26; k++)
                            cost[i][k] = (i == k) ? 0 : -1;
                }
                for (int j = 0; j < words.length; j++) {
                    int c = Integer.parseInt(words[j]);
                    int k = row + 1 + j;

                    cost[row][k] = c;
                    cost[k][row] = c;
                }
                row++;
            }
        } catch (IOException e) {
            System.err.println("Error: " + e.getMessage());
            return false;
        }

        return true;
    }

    static void printMatrix() {
        System.out.print("   ");
        for (int k = 0; k < n; k++) System.out.printf("%4c", (char)('A' + k));
        System.out.println();
        for (int i = 0; i < n; i++) {
            System.out.printf("%-3c", (char)('A' + i));
            for (int k = 0; k < n; k++) System.out.printf("%4d", cost[i][k]);
            System.out.println();
        }
    }
    static void sendLinkState(int id) throws IOException {

        int count = 1;

        for (int j = 0; j < n; j++) {
            if (j != id && cost[id][j] != -1) {
                count++;
            }
        }

        DataOutputStream out = vnOut[id];

        if (vnSock[id] == null || vnSock[id].isClosed() || out == null) {
            throw new IOException(
                "No active connection for node " + (char) ('A' + id));
        }

        out.writeInt(count);

        out.writeByte('A' + id);    
        out.write(new byte[3]);     
        out.write(vnIp[id]);        
        out.writeShort(vnPort[id]); 
        out.write(new byte[2]);
        out.writeInt(0);            

        for (int j = 0; j < n; j++) {

            if (j == id || cost[id][j] == -1) {
                continue;
            }

            out.writeByte('A' + j);
            out.write(new byte[3]);
            out.write(vnIp[j]);
            out.writeShort(vnPort[j]);
            out.write(new byte[2]);
            out.writeInt(cost[id][j]);
        }

        out.flush();

        System.out.println(
            "Sent LINK-STATE to " + (char) ('A' + id)
            + " with " + count + " tuples");
    }

    public static void main(String[] args) throws IOException{
        if(args.length != 1){
            System.out.println("need a config file:");
            return;
        }
        if (!parsefile(args[0])) System.exit(1);
        printMatrix();

        ServerSocket server = new ServerSocket();
        server.bind(new InetSocketAddress(ON_PORT));
        server.setSoTimeout(500);
        System.out.println("Listening on port " + ON_PORT + ", waiting for " + n + " nodes...");
        acceptVNs(server,args[0]);
    }

}
