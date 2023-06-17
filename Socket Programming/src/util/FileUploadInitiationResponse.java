package util;

import java.io.Serializable;

public class FileUploadInitiationResponse implements Serializable {
    public int chunkSize;
    public String fileID;
    public boolean isOK;

    public FileUploadInitiationResponse(int chunkSize, String fileID) {
        this.chunkSize = chunkSize;
        this.fileID = fileID;
        isOK = true;
    }

    public FileUploadInitiationResponse(boolean isOK) {
        this.isOK = false;
    }
}
