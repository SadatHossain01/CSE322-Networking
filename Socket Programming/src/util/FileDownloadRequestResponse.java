package util;

public class FileDownloadRequestResponse extends Request {
    public boolean isAccepted;
    public String fileName;
    public int chunkSize;
    public int fileSize;

    public FileDownloadRequestResponse(boolean isAccepted) {
        super(RequestType.DOWNLOAD_REQUEST_RESPONSE);
        this.isAccepted = isAccepted;
    }

    public FileDownloadRequestResponse(boolean isAccepted, String fileName, int chunkSize, int fileSize) {
        super(RequestType.DOWNLOAD_REQUEST_RESPONSE);
        this.isAccepted = isAccepted;
        this.fileName = fileName;
        this.chunkSize = chunkSize;
        this.fileSize = fileSize;
    }
}
