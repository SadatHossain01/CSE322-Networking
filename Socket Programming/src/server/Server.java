package server;

import java.io.File;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import util.*;

public class Server {
    private ServerSocket serverSocket;
    private HashMap<String, NetworkUtil> clientMap;
    private List<String> userList;
    private HashMap<String, List<FileInfo>> fileMap;
    private HashMap<String, List<UserMessage>> messageMap;
    private List<FileRequest> fileRequestList;
    private int FILE_COUNT;
    public long CUR_BUFFER_SIZE, MAX_BUFFER_SIZE, MIN_CHUNK_SIZE, MAX_CHUNK_SIZE; // in bytes

    public Server(long MAX_BUFFER_SIZE, long MIN_CHUNK_SIZE, long MAX_CHUNK_SIZE) {
        this.MAX_BUFFER_SIZE = MAX_BUFFER_SIZE;
        this.MIN_CHUNK_SIZE = MIN_CHUNK_SIZE;
        this.MAX_CHUNK_SIZE = MAX_CHUNK_SIZE;
        this.CUR_BUFFER_SIZE = 0;
        clientMap = new HashMap<>();
        userList = new ArrayList<>();
        fileMap = new HashMap<>();
        fileRequestList = new ArrayList<>();
        messageMap = new HashMap<>();

        try {
            System.out.println("Server started...");
            serverSocket = new ServerSocket(33333);
            File file = new File("src/storage");
            file.mkdir();
            while (true) {
                serve(serverSocket.accept());
            }
        } catch (Exception e) {
            System.out.println("Server failed to start: " + e);
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
        Server server = new Server(1000000000, 1000, 1000);
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
            String s = file.fileName;
            if (file.isPrivate) s += "X";
            else s += "O";
            myFiles.add(s);
        }
        return myFiles;
    }

    public List<String> getSharedFiles() {
        List<String> sharedFiles = new ArrayList<>();
        for (String username : fileMap.keySet()) {
            for (FileInfo file : fileMap.get(username)) {
                if (!file.isPrivate) {
                    String s = file.fileName;
                    s += " (File ID: " + file.fileID + ")O"; // O for public
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
        UserMessage m = new UserMessage(fileRequest.requester, "all", true, description);
        for (String username : clientMap.keySet()) { // broadcast to all the connected clients
            messageMap.get(username).add(m);
        }
    }

    public List<String> getMessages(String username) {
        List<String> messages = new ArrayList<>();
        for (UserMessage m : messageMap.get(username)) {
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

    public boolean checkRequestID(String requestID) {
        for (FileRequest fileRequest : fileRequestList) {
            if (fileRequest.requestID.equals(requestID)) {
                return true;
            }
        }
        return false;
    }

    public void addFile(FileUploadInitiationRequest req, String fileID) {
        req.fileInfo.fileID = fileID;
        fileMap.get(req.fileInfo.ownerName).add(req.fileInfo);
        FILE_COUNT++;
        if (req.requested) {
            // send message to the person that requested the file
            String req_id = req.requestID;

            for (FileRequest fileRequest : fileRequestList) {
                if (fileRequest.requestID.equals(req_id)) {
                    UserMessage m = new UserMessage(req.fileInfo.ownerName, fileRequest.requester, false, "File " + req.fileInfo.fileName + " has been uploaded by " + req.fileInfo.ownerName + " (File ID: " + fileID + ")");
                    messageMap.get(fileRequest.requester).add(m);
                    break;
                }
            }
        }
    }

    public String generateFileID() {
        return FILE_COUNT + 1 + "";
    }
}
