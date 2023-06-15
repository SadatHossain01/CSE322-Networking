package util;

public class Message {
    public boolean seen = false;
    public String sender;
    public String receiver;
    public String msg;
    public boolean isRequest;

    public Message(String sender, String receiver, boolean isRequest, String msg) {
        this.sender = sender;
        this.receiver = receiver;
        this.msg = msg;
        this.isRequest = isRequest;
//        System.out.println(msg);
    }
}
