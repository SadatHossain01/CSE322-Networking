package util;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;

public class NetworkUtil {
    private Socket socket;
    private ObjectOutputStream oos;
    private ObjectInputStream ois;

    public NetworkUtil(String s, int port) throws IOException {
        this.socket = new Socket(s, port);
        oos = new ObjectOutputStream(socket.getOutputStream());
        ois = new ObjectInputStream(socket.getInputStream());
    }

    public NetworkUtil(Socket s) throws IOException {
        this.socket = s;
        oos = new ObjectOutputStream(socket.getOutputStream());
        ois = new ObjectInputStream(socket.getInputStream());
    }

    public Object read() throws IOException, ClassNotFoundException {
        return ois.readUnshared();
    }

    public void write(Object o) throws IOException {
        oos.writeUnshared(o);
        flush();
    }

    public void closeConnection() throws IOException {
        ois.close();
        oos.close();
    }

    public int read(byte[] buf, int off, int len) throws IOException {
//        buf - the buffer into which the data is read
//        off - the start offset in the destination array buf
//        len - the maximum number of bytes read
//        https://docs.oracle.com/en/java/javase/17/docs/api/java.base/java/io/ObjectInputStream.html#read(byte%5B%5D,int,int)
        return ois.read(buf, off, len);
    }

    public void write(byte[] buffer, int off, int len) throws IOException {
        oos.write(buffer, off, len);
        flush();
    }

    public void flush() throws IOException {
        oos.flush();
    }
}

