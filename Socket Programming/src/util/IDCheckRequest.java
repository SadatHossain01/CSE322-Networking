package util;

public class IDCheckRequest extends Request { // this request just asks with the request ID and server replies if the request ID is there or not, then normal upload initiates
    public String requestID;

    public IDCheckRequest(String requestID) {
        super(RequestType.REQUESTED_UPLOAD);
        this.requestID = requestID;
    }
}
