package com.example.intelligentsmokealarmsystem.bean;

public class Send {
    private int cmd;
    private DataDTO data;

    @Override
    public String toString() {
        return "Send{" +
                "cmd=" + cmd +
                ", data=" + data +
                '}';
    }

    public int getCmd() {
        return cmd;
    }

    public void setCmd(int cmd) {
        this.cmd = cmd;
    }

    public DataDTO getData() {
        return data;
    }

    public void setData(DataDTO data) {
        this.data = data;
    }
}
