package client;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.SocketTimeoutException;
import java.util.List;
import java.util.Scanner;

import util.*;

import static java.lang.System.exit;

public class Client {
    private static NetworkUtil networkUtil;
    private static String clientName;
    private static Scanner scanner = new Scanner(System.in);

    public Client(String serverAddress, int serverPort) {
        File file = new File("src/client/to_upload");
        file.mkdir();

        try {
            System.out.print("Enter your username: ");
            Scanner scanner = new Scanner(System.in);
            clientName = scanner.nextLine();

            networkUtil = new NetworkUtil(serverAddress, serverPort);
            networkUtil.write(clientName);

            String response = (String) networkUtil.read();
            System.out.println(response);

            String[] tokens = response.split(" ");
            if (!tokens[0].contains("Welcome")) {
                exit(0);
            }
        } catch (Exception e) {
            System.out.println(e);
        }
    }

    private static void showUsers(List<String> users, String userType) {
        System.out.println("List of " + userType + " Users:");
        for (String user : users) {
            System.out.println(user);
        }
    }

    public static void main(String[] args) throws IOException, ClassNotFoundException {
        String serverAddress = "127.0.0.1";
        int serverPort = 33333;
        Client client = new Client(serverAddress, serverPort);

        while (true) {
            System.out.println("Menu:");
            System.out.println("1. Show Registered Users"); // distinguish the ones that are online
            System.out.println("2. Show Active Users");
            System.out.println("3. Show My Files"); // distinguish private and public files
            System.out.println("4. Show Shared Files"); // public files of other users
            System.out.println("5. Request a File");
            System.out.println("6. Show Unread Messages");
            System.out.println("7. Show File Requests");
            System.out.println("8. Upload A File"); // ask here if it is a requested file
            System.out.println("9. Download A File");
            System.out.println("10. Log Out\n");

            int choice = scanner.nextInt();

            if (choice == 1) {
                // show all registered users
                networkUtil.write(new Request(RequestType.REGISTERED_USERLIST));
                Object o = networkUtil.read();
                if (o instanceof SendableList) {
                    SendableList sendableList = (SendableList) o;
                    sendableList.showUsers("Registered");
                }
            } else if (choice == 2) {
                // show all active users
                networkUtil.write(new Request(RequestType.ACTIVE_USERLIST));
                Object o = networkUtil.read();
                if (o instanceof SendableList) {
                    SendableList sendableList = (SendableList) o;
                    sendableList.showUsers("Active");
                }
            } else if (choice == 3) {
                // show my files
                networkUtil.write(new Request(RequestType.MY_FILES));
                Object o = networkUtil.read();
                if (o instanceof SendableList) {
                    SendableList sendableList = (SendableList) o;
                    sendableList.showFiles("Your");
                }
            } else if (choice == 4) {
                // show shared files
                networkUtil.write(new Request(RequestType.SHARED_FILES));
                Object o = networkUtil.read();
                if (o instanceof SendableList) {
                    SendableList sendableList = (SendableList) o;
                    sendableList.showFiles("Shared");
                }
            } else if (choice == 5) {
                // request a file
                scanner.nextLine();
                System.out.print("Enter Request ID: ");
                String fileID = scanner.next();
                scanner.nextLine();
//                If you call the scanner.nextLine() method after any of the other scanner.nextWhatever() methods, the program will skip that call.
                System.out.println("Give a short description of the file: ");
                String description = scanner.nextLine();
                networkUtil.write(new FileRequest(clientName, fileID, description));
            } else if (choice == 6) {
                // show unread messages
                networkUtil.write(new Request(RequestType.MESSAGES));
                Object o = networkUtil.read();
                if (o instanceof SendableList) {
                    SendableList sendableList = (SendableList) o;
                    sendableList.showMessages();
                }
            } else if (choice == 7) {
                // show file requests
                networkUtil.write(new Request(RequestType.SHOW_FILE_REQUESTS));
                Object o = networkUtil.read();
                if (o instanceof SendableList) {
                    SendableList sendableList = (SendableList) o;
                    sendableList.showFileRequests();
                }
            } else if (choice == 8) {
                // upload a file
                System.out.print("Do you want to upload a requested file? (y/n): ");
                String answer = scanner.next().toLowerCase();
                if (answer.charAt(0) == 'y') {
                    System.out.print("Enter request ID for the file: ");
                    String requestID = scanner.next();
                    networkUtil.write(new IDCheckRequest(requestID));
                    String response = (String) networkUtil.read();
                    if (response.charAt(0) == 'y') {
                        initiateFileUpload(true, requestID);
                    } else {
                        System.out.println("There is no upload request with this ID.");
                    }
                } else {
                    initiateFileUpload(false, "123");
                }
            } else if (choice == 9) {
                // download a file
            } else if (choice == 10) {
                // log out
                networkUtil.write(new Request(RequestType.LOGOUT));
                String response = (String) networkUtil.read();
                if (response.equals("ok")) System.out.println("Logged out successfully.");
                exit(0);
            } else {
                System.out.println("Invalid choice!");
            }
        }
    }

    private static void initiateFileUpload(boolean isRequested, String requestID) throws IOException, ClassNotFoundException {
        scanner.nextLine();
        System.out.print("Enter file name: ");
        String fileName = scanner.nextLine();


        File file = new File("src/client/to_upload/" + fileName);
        if (!file.exists()) {
            System.out.println("File does not exist!");
            return;
        }

        long fileSize = (long) file.length();

        boolean isPublic = false;
        if (!isRequested) {
            System.out.print("Do you want to keep the file public? (y/n): ");
            String answer = scanner.next().toLowerCase();
            if (answer.charAt(0) == 'y') {
                isPublic = true;
            }
        }

        FileInfo fileInfo = new FileInfo(fileName, !isPublic, clientName, fileSize);
        FileUploadInitiationRequest req = new FileUploadInitiationRequest(fileInfo, isRequested, requestID);
        networkUtil.write(req);

        Object response = networkUtil.read();

        if (response instanceof FileUploadInitiationResponse) {
            FileUploadInitiationResponse fileUploadInitiationResponse = (FileUploadInitiationResponse) response;
            if (fileUploadInitiationResponse.isOK) {
                System.out.println("File upload initiated successfully.");
                uploadFile(fileUploadInitiationResponse.chunkSize, fileUploadInitiationResponse.fileID, file, req);
            } else {
                System.out.println("File upload initiation failed.");
            }
        }
    }

    private static void uploadFile(long chunkSize, String fileID, File file, FileUploadInitiationRequest req) throws IOException, ClassNotFoundException {
        FileInputStream fileInputStream = new FileInputStream(file);

        byte[] buffer = new byte[(int) chunkSize];
        int chunk_number = 0;
        int read_bytes = 0;

        while (true) {
            read_bytes = fileInputStream.read(buffer);
            if (read_bytes == -1) break;

//            if (chunk_number % 500 == 0)
//                System.out.println("Uploading chunk " + chunk_number + " of " + req.fileInfo.fileName);

            chunk_number++;

//            System.out.println("Read " + read_bytes);
            networkUtil.write(buffer, 0, read_bytes);
//            System.out.println("Done Writing One Chunk");

            // tries to read ok here
            try {
                String msg = (String) networkUtil.read();
                if (!msg.equals("ok")) {
                    System.out.println("Did not receive acknowledgement message from server.");
                    break;
                }
            } catch (SocketTimeoutException e) {
                System.out.println("Timeout in receiving acknowledgement message from server.");
//                networkUtil.write("timeout");
                break;
            }
        }

        fileInputStream.close();

        networkUtil.write("done"); // final confirmation
        String msg = (String) networkUtil.read();
        System.out.println("Response from Server: " + msg);
    }
}
