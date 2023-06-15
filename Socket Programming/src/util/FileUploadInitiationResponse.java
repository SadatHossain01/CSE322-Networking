package util;

import java.io.Serializable;

public class FileUploadInitiationResponse implements Serializable {
    public long chunkSize;
    public String fileID;
    public boolean isOK;

    public FileUploadInitiationResponse(long chunkSize, String fileID) {
        this.chunkSize = chunkSize;
        this.fileID = fileID;
        isOK = true;
    }

    public FileUploadInitiationResponse(boolean isOK) {
        this.isOK = false;
    }
}
