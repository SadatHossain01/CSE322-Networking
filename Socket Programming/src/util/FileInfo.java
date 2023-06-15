package util;

import java.io.Serializable;

public class FileInfo implements Serializable {
    private String fileName;
    private String fileID;
    private boolean isPrivate;
    private String ownerName;
    private int fileSize;

    public FileInfo(String fileName, boolean isPrivate, String ownerName, int fileSize) {
        this.fileName = fileName;
        this.isPrivate = isPrivate;
        this.ownerName = ownerName;
        this.fileSize = fileSize;
    }

    public void setFileID(String fileID) {
        this.fileID = fileID;
    }

    public String getName() {
        return fileName;
    }

    public boolean isPrivate() {
        return isPrivate;
    }

    public String getID() {
        return fileID;
    }
}
