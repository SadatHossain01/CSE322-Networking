package util;

import java.io.Serializable;
import java.util.List;

public class SendableList implements Serializable {
    private List<String> list;

    public SendableList(List<String> list) {
        this.list = list;
    }

    public void showUsers(String type) {
        System.out.println("List of " + type + " Users:");
        int i = 1;
        for (String user : list) {
            System.out.println(i + ". " + user);
            i++;
        }
        System.out.println();
    }

    public void showFiles(String type) {
        System.out.println("List of " + type + " Files:");

        int publicCount = 0;
        for (String f : list) {
            if (f.charAt(f.length() - 1) == 'O') {
                publicCount++;
            }
        }

        int i = 1;
        if (publicCount > 0) {
            System.out.println("Public Files:");
            for (String file : list) {
                int len = file.length();
                if (file.charAt(len - 1) == 'X') continue;
                System.out.println(i + ". " + file.substring(0, len - 2));
                i++;
            }
        } else {
            if (type.equals("Your")) System.out.println("No public files to show.");
            else System.out.println("No shared files to show.");
        }
        if (!type.equals("Your")) return; // because shared files are not private

        if (list.size() - publicCount > 0) {
            System.out.println("Private Files:");
            i = 1;
            for (String file : list) {
                int len = file.length();
                if (file.charAt(len - 1) == 'X') {
                    System.out.println(i + ". " + file.substring(0, len - 2));
                    i++;
                }
            }
        } else {
            System.out.println("No private files to show.");
        }

        System.out.println();
    }

    public void showMessages() {
        System.out.println("List of Unread Messages:");
        if (list.size() == 0) {
            System.out.println("No unread messages to show.");
        }
        else {
            int i = 1;
            for (String message : list) {
                System.out.println(i + ". ");
                System.out.println(message);
                i++;
            }
        }
        System.out.println();
    }
}
