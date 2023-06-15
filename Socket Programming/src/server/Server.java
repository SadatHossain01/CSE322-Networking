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
import util.SendableFile;

public class Server {
    private ServerSocket serverSocket;
    private HashMap<String, NetworkUtil> clientMap;
    private List<String> userList;
    private HashMap<String, List<SendableFile>> fileMap;
    private HashMap<String, List<Message>> messageMap;
    private List<FileRequest> fileRequestList;

    public Server() {
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
        Server server = new Server();
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
        for (SendableFile file : fileMap.get(username)) {
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
            for (SendableFile file : fileMap.get(username)) {
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
        String description = fileRequest.requester + " has requested for a file of following description: (Request ID: " + fileRequest.fileID + ")\n";
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
}
