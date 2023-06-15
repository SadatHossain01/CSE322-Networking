package server;

import util.*;

public class ServerThread implements Runnable {
    private Thread thr;
    private NetworkUtil networkUtil;
    private String username;
    private Server server;

    public ServerThread(String username, Server server, NetworkUtil networkUtil) {
        this.networkUtil = networkUtil;
        this.username = username;
        this.server = server;
        this.thr = new Thread(this);
        thr.start();
    }

    @Override
    public void run() {
        try {
            while (true) {
                Object o = networkUtil.read();
                if (o instanceof Request) {
                    RequestType requestType = ((Request) o).requestType;

                    if (requestType == RequestType.REGISTERED_USERLIST) {
                        System.out.println("Sending registered user list to " + username);
                        networkUtil.write(new SendableList(server.getUserList("registered")));
                    } else if (requestType == RequestType.ACTIVE_USERLIST) {
                        System.out.println("Sending active user list to " + username);
                        networkUtil.write(new SendableList(server.getUserList("active")));
                    } else if (requestType == RequestType.MY_FILES) {
                        System.out.println("Sending personal file list to " + username);
                        networkUtil.write(new SendableList(server.getMyFiles(username)));
                    } else if (requestType == RequestType.SHARED_FILES) {
                        System.out.println("Sending shared file list to " + username);
                        networkUtil.write(new SendableList(server.getSharedFiles()));
                    } else if (requestType == RequestType.FILE_REQUEST) {
                        System.out.println(username + " requested a file of ID: " + ((FileRequest) o).requestID);
                        FileRequest fileRequest = (FileRequest) o;
                        server.addFileRequest(fileRequest);
                        server.broadcastRequest(fileRequest);
                    } else if (requestType == RequestType.MESSAGES) {
                        System.out.println("Showing messages to " + username);
                        networkUtil.write(new SendableList(server.getMessages(username)));
                    }
                    else if (requestType == RequestType.SHOW_FILE_REQUESTS) {
                        System.out.println("Showing file requests to " + username);
                        networkUtil.write(new SendableList(server.getFileRequests()));
                    }
                    else if (requestType == RequestType.UPLOAD) {
                    }
                    else if (requestType == RequestType.DOWNLOAD) {
                    }
                    else if (requestType == RequestType.LOGOUT) {
                        server.makeUserInactive(username);
                        networkUtil.write("ok");
                        System.out.println(username + " logged out.");
                        networkUtil.closeConnection();
                        break;
                    }
                }
            }
        } catch (Exception e) {
//            System.out.println(e);
            server.makeUserInactive(username);
            System.out.println(username + " got disconnected.");
        } finally {
            try {
                networkUtil.closeConnection();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
}
