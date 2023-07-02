package util;

public class MatchFileRequestID extends Request {
    // this request just asks with the file_request ID and server replies if the file_request ID is there or not, then normal upload initiates
    public String requestID;

    public MatchFileRequestID(String requestID) {
        super(RequestType.REQUESTED_UPLOAD_CROSSCHECK);
        this.requestID = requestID;
    }
}
