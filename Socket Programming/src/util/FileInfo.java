package util;

import java.io.Serializable;

public class FileInfo implements Serializable {
    public String fileName;
    public String fileID;
    public boolean isPrivate;
    public String ownerName;
    public long fileSize; // in bytes

    public FileInfo(String fileName, boolean isPrivate, String ownerName, long fileSize) {
        this.fileName = fileName;
        this.isPrivate = isPrivate;
        this.ownerName = ownerName;
        this.fileSize = fileSize;
    }
}
