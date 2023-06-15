package server;

import java.io.File;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import util.FileRequest;
import util.Message;
import util.NetworkUtil;
import util.FileInfo;

public class Server {
    private ServerSocket serverSocket;
    private HashMap<String, NetworkUtil> clientMap;
    private List<String> userList;
    private HashMap<String, List<FileInfo>> fileMap;
    private HashMap<String, List<Message>> messageMap;
    private List<FileRequest> fileRequestList;
    private int MAX_BUFFER_SIZE, MIN_CHUNK_SIZE, MAX_CHUNK_SIZE; // kB

    public Server(int MAX_BUFFER_SIZE, int MIN_CHUNK_SIZE, int MAX_CHUNK_SIZE) {
        this.MAX_BUFFER_SIZE = MAX_BUFFER_SIZE;
        this.MIN_CHUNK_SIZE = MIN_CHUNK_SIZE;
        this.MAX_CHUNK_SIZE = MAX_CHUNK_SIZE;
        clientMap = new HashMap<>();
        userList = new ArrayList<>();
        fileMap = new HashMap<>();
        fileRequestList = new ArrayList<>();
        messageMap = new HashMap<>();

        try {
            System.out.println("Server starts...\n");
            serverSocket = new ServerSocket(33333);
            File file = new File("src/storage");
            file.mkdir();
            while (true) {
                serve(serverSocket.accept());
            }
        } catch (Exception e) {
            System.out.println("Server failed to start: " + e + "\n");
        }
    }

    public void serve(Socket clientSocket) throws IOException, ClassNotFoundException {
        NetworkUtil networkUtil = new NetworkUtil(clientSocket);
        String clientName = (String) networkUtil.read();

        if (clientMap.containsKey(clientName)) {
            // user logged in already
            networkUtil.write("User is already logged in!");
            networkUtil.closeConnection();
            return;
        } else {
            System.out.println(clientName + " logged in.");
            clientMap.put(clientName, networkUtil);
            fileMap.put(clientName, new ArrayList<>());
            messageMap.put(clientName, new ArrayList<>());
            if (userList.contains(clientName)) {
                // log user in, but no need to create a new directory
                networkUtil.write("Welcome back, " + clientName + "!");
            } else {
                // log user in, and create a new directory
                userList.add(clientName);
                File file = new File("src/storage/" + clientName);
                if (file.mkdir()) {
                    System.out.println("Directory created for " + clientName);
                } else {
                    System.out.println("Directory already exists for " + clientName);
                }
                networkUtil.write("Welcome, " + clientName + "!");
            }
        }

        new ServerThread(clientName, this, networkUtil);
    }

    public static void main(String[] args) {
        Server server = new Server(20000, 100, 1000);
    }

    public List<String> getUserList(String type) {
        if (type.equals("registered")) {
            return userList;
        } else if (type.equals("active")) {
            return new ArrayList<>(clientMap.keySet());
        } else {
            return null;
        }
    }

    public void makeUserInactive(String username) {
        clientMap.remove(username);
    }

    public List<String> getMyFiles(String username) {
        List<String> myFiles = new ArrayList<>();
        for (FileInfo file : fileMap.get(username)) {
            String s = file.getName();
            if (file.isPrivate()) s += "X";
            else s += "O";
            myFiles.add(s);
        }
        return myFiles;
    }

    public List<String> getSharedFiles() {
        List<String> sharedFiles = new ArrayList<>();
        for (String username : fileMap.keySet()) {
            for (FileInfo file : fileMap.get(username)) {
                if (!file.isPrivate()) {
                    String s = file.getName();
                    s += " (File ID: " + file.getID() + ")O"; // O for public
                    sharedFiles.add(s);
                }
            }
        }
        return sharedFiles;
    }

    public void addFileRequest(FileRequest fileRequest) {
        fileRequestList.add(fileRequest);
    }

    public void broadcastRequest(FileRequest fileRequest) {
        String description = fileRequest.requester + " has requested for a file of following description: (Request ID: " + fileRequest.requestID + ")\n";
        description += fileRequest.description;
        Message m = new Message(fileRequest.requester, "all", true, description);
        for (String username : clientMap.keySet()) { // broadcast to all the connected clients
            messageMap.get(username).add(m);
        }
    }

    public List<String> getMessages(String username) {
        List<String> messages = new ArrayList<>();
        for (Message m : messageMap.get(username)) {
            messages.add(m.msg);
            m.seen = true;
        }
        messageMap.get(username).clear(); // because no option to show read messages, may as well omit the seen field then
        return messages;
    }

    public List<String> getFileRequests() {
        List<String> fileRequests = new ArrayList<>();
        for (FileRequest fileRequest : fileRequestList) {
            String s = "Requested By: " + fileRequest.requester + ", Request ID: " + fileRequest.requestID + "\n";
            s += "Short Description: " + fileRequest.description;
            fileRequests.add(s);
        }
        return fileRequests;
    }
}
