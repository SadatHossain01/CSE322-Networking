package util;

public class FileUploadInitiationRequest extends Request {
    // this request sends the proposed file name, privacy, file size, (if requested) request id to server for initiation of the upload process
    // fileID is set by server though
    public FileInfo fileInfo;
    public boolean requested;
    public String requestID;

    public FileUploadInitiationRequest(FileInfo fileInfo, boolean requested, String requestID) {
        super(RequestType.UPLOAD_INITIATION);
        this.fileInfo = fileInfo;
        this.requested = requested;
        if (requested) {
            this.requestID = requestID;
        }
    }
}
