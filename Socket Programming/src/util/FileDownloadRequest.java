package util;

public class FileDownloadRequest extends Request {
    public String fileID;

    public FileDownloadRequest(String fileID) {
        super(RequestType.DOWNLOAD_REQUEST);
        this.fileID = fileID;
    }
}
