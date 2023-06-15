package util;

public class FileRequest extends Request {
    public String requester;
    public String fileID;
    public String description;

    public FileRequest(String requester, String fileID, String description) {
        super(RequestType.FILE_REQUEST);
        this.requester = requester;
        this.fileID = fileID;
        this.description = description;
    }
}