package server;

import util.*;

import java.io.*;
import java.net.SocketTimeoutException;
import java.util.Random;

public class ServerThread implements Runnable {
    private Thread thr;
    private NetworkUtil networkUtil;
    private String username;
    private Server server;
    private boolean isConnected;
    Random random = new Random();

    public ServerThread(String username, Server server, NetworkUtil networkUtil) {
        this.networkUtil = networkUtil;
        this.username = username;
        this.server = server;
        this.thr = new Thread(this);
        isConnected = true;
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
                        FileRequest fileRequest = (FileRequest) o;
                        fileRequest.requestID = server.generateRequestID();
                        System.out.println(username + " requested a file, request ID: " + fileRequest.requestID);
                        server.addFileRequest(fileRequest);
                        server.broadcastRequest(fileRequest);
                    } else if (requestType == RequestType.MESSAGES) {
                        System.out.println("Showing messages to " + username);
                        networkUtil.write(new SendableList(server.getMessages(username)));
                    } else if (requestType == RequestType.SHOW_FILE_REQUESTS) {
                        System.out.println("Showing file requests to " + username);
                        networkUtil.write(new SendableList(server.getFileRequests()));
                    } else if (requestType == RequestType.REQUESTED_UPLOAD) {
                        String requestID = ((IDCheckRequest) o).requestID;
                        boolean accepted = server.checkRequestID(requestID);
                        if (accepted) {
                            System.out.println("Go on with upload from " + username + " for request ID: " + requestID);
                            networkUtil.write("yes");
                        } else {
                            System.out.println("Rejected upload request from " + username + " for no match with any request ID");
                            networkUtil.write("no");
                        }
                    } else if (requestType == RequestType.UPLOAD) {
                        long fileSize = ((FileUploadInitiationRequest) o).fileInfo.fileSize;
                        if (fileSize + server.CUR_BUFFER_SIZE > server.MAX_BUFFER_SIZE) {
                            networkUtil.write(new FileUploadInitiationResponse(false));
                            System.out.println("Rejected upload request from " + username + " for exceeding buffer size");
                        } else {
                            long chunkSize = generateRandomNumber(server.MIN_CHUNK_SIZE, server.MAX_CHUNK_SIZE);
                            String fileID = server.generateFileID();
                            networkUtil.write(new FileUploadInitiationResponse(chunkSize, fileID));
                            boolean success = receiveFile((FileUploadInitiationRequest) o, chunkSize, fileID);
                            if (success) {
                                System.out.println("Successfully received file from " + username);
                                networkUtil.write("successful upload");
                                server.addFile((FileUploadInitiationRequest) o, fileID);
                            } else {
                                System.out.println("Failed to receive file from " + username);
                                networkUtil.write("failed upload");
                            }
                        }
                    } else if (requestType == RequestType.DOWNLOAD_REQUEST) {
                        FileDownloadRequest fileDownloadRequest = (FileDownloadRequest) o;
                        String fileID = fileDownloadRequest.fileID;
                        FileInfo fileInfo = server.checkFileAvailability(fileID);
                        if (fileInfo == null) {
                            networkUtil.write(new FileDownloadRequestResponse(false));
                            System.out.println("Rejected download request from " + username + " for no match with any file ID");
                        }
                        else {
                            networkUtil.write(new FileDownloadRequestResponse(true, fileInfo.fileName, (int)server.MAX_CHUNK_SIZE, (int)fileInfo.fileSize));
                            System.out.println("Accepted download request from " + username + " for file ID: " + fileID);
                            sendFile(fileInfo);
                        }
                    } else if (requestType == RequestType.LOGOUT) {
                        server.makeUserInactive(username);
                        networkUtil.write("ok");
                        System.out.println(username + " logged out.");
                        isConnected = false;
                        networkUtil.closeConnection();
                        break;
                    }
                }
            }
        } catch (Exception e) {
            System.out.println(e);
            server.makeUserInactive(username);
            isConnected = false;
            System.out.println(username + " got disconnected.");
        } finally {
            try {
                networkUtil.closeConnection();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    private long generateRandomNumber(long a, long b) {
        long aa = random.nextLong() % (b - a + 1);
        while (aa < 0) aa += (b - a + 1);
        long rep = aa + a;
        return rep;
    }

    private boolean receiveFile(FileUploadInitiationRequest fileDetail, long chunkSize, String fileID) throws IOException, ClassNotFoundException {
        FileInfo fileInfo = fileDetail.fileInfo;

        System.out.println("Receiving file " + fileInfo.fileName + " from " + username);
        FileOutputStream fileOutputStream = new FileOutputStream("src/storage/" + username + "/" + fileInfo.fileName);
        System.out.println("File Output Stream Opened");

        try {
            int fileSize = (int) fileInfo.fileSize;
            byte[] buffer = new byte[(int) chunkSize];
            server.CUR_BUFFER_SIZE += chunkSize;

            int chunk_number = 0;
            System.out.println("File Size: " + fileSize + " bytes");

            while (fileSize > 0 && isConnected) {
                int read_bytes;
                try {
//                    System.out.println("Want to read a chunk");
                    if (isConnected)
                        read_bytes = networkUtil.read(buffer, 0, Math.min(buffer.length, fileSize));
                    else
                        throw new SocketTimeoutException();
//                    System.out.println("Read a chunk");
                } catch (SocketTimeoutException e) {
                    server.CUR_BUFFER_SIZE -= chunkSize;
                    fileOutputStream.close();
                    System.out.println("File upload from " + username + " failed due to timeout.");
                    return false;
                }

                if (read_bytes == -1) break; // -1 is returned when the end of the stream is reached.

//                if (chunk_number % 500 == 0)
//                System.out.println("Chunk number " + chunk_number + " received from " + username);
                chunk_number++;

                fileOutputStream.write(buffer, 0, read_bytes);

                fileSize -= read_bytes;

                networkUtil.write("ok");
            }

            server.CUR_BUFFER_SIZE -= chunkSize;
            fileOutputStream.close();
            System.out.println("File Output Stream Closed");

        } catch (Exception e) {
            e.printStackTrace();
            server.CUR_BUFFER_SIZE -= chunkSize;
            fileOutputStream.close();
            System.out.println("File Output Stream Closed");
        }

        String final_msg = (String) networkUtil.read();
        if (final_msg.equals("done")) {
            File file = new File("src/storage/" + username + "/" + fileInfo.fileName);
            if (file.length() != fileInfo.fileSize) {
                System.out.println("File Size Mismatch found");
                file.delete();
                return false;
            }
            return true;
        } else {
            return false;
        }
    }

    private void sendFile(FileInfo fileInfo) throws IOException {
        FileInputStream fileInputStream = null;
        try {
            fileInputStream = new FileInputStream("src/storage/" + fileInfo.ownerName + "/" + fileInfo.fileName);
        } catch (FileNotFoundException e) {
            System.out.println("File not found");
            return;
        }

        byte[] buffer = new byte[(int) server.MAX_CHUNK_SIZE];
        int chunk_number = 0, read_bytes = 0;

        while (true) {
            read_bytes = fileInputStream.read(buffer);
            if (read_bytes == -1) break;

            chunk_number++;

            networkUtil.write(buffer, 0, read_bytes);

            // no wait for message from client
        }

        fileInputStream.close();
        networkUtil.write("done");
    }
}