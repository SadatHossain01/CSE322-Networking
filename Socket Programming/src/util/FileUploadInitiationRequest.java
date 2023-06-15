package util;

import util.Request;

public class FileUploadInitiationRequest extends Request {
    public FileInfo fileInfo;
    public boolean requested;
    public String requestID;

    public FileUploadInitiationRequest(FileInfo fileInfo, boolean requested, String requestID) {
        super(RequestType.UPLOAD);
        this.fileInfo = fileInfo;
        this.requested = requested;
        if (requested) {
            this.requestID = requestID;
        }
    }
}
