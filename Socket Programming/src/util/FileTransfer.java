package util;

import java.io.Serializable;

public class FileTransfer implements Serializable {
    public byte[] chunk;
    public String fileID;

    public FileTransfer(byte[] chunk, String fileID) {
        this.chunk = chunk;
        this.fileID = fileID;
    }
}
