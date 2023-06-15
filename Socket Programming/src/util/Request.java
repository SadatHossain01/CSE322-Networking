package util;

import java.io.Serializable;

public class Request implements Serializable {
    public RequestType requestType;

    public Request(RequestType requestType) {
        this.requestType = requestType;
    }
}
