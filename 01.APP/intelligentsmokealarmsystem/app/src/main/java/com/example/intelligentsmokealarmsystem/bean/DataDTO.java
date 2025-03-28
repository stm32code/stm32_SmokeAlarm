package com.example.intelligentsmokealarmsystem.bean;

public class DataDTO {
    private Integer flage;
    private Integer temp_v;
    private Integer humi_v;
    private Integer somg_v;
    private Integer beep;

    @Override
    public String toString() {
        return "DataDTO{" +
                "flage=" + flage +
                ", temp_v=" + temp_v +
                ", humi_v=" + humi_v +
                ", somg_v=" + somg_v +
                ", beep=" + beep +
                '}';
    }
    // Getter and Setter methods for fields

    public Integer getFlage() {
        return flage;
    }

    public void setFlage(Integer flage) {
        this.flage = flage;
    }

    public Integer getTemp_v() {
        return temp_v;
    }

    public void setTemp_v(Integer temp_v) {
        this.temp_v = temp_v;
    }

    public Integer getHumi_v() {
        return humi_v;
    }

    public void setHumi_v(Integer humi_v) {
        this.humi_v = humi_v;
    }

    public Integer getSomg_v() {
        return somg_v;
    }

    public void setSomg_v(Integer somg_v) {
        this.somg_v = somg_v;
    }

    public Integer getBeep() {
        return beep;
    }

    public void setBeep(Integer beep) {
        this.beep = beep;
    }
}
