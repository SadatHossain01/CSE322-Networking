package util;

public class FileRequest extends Request {
    public String requester;
    public String requestID;
    public String description;

    public FileRequest(String requester, String requestID, String description) {
        super(RequestType.FILE_REQUEST);
        this.requester = requester;
        this.requestID = requestID;
        this.description = description;
    }
}