package com.example.intelligentsmokealarmsystem.utils;

import com.itfitness.mqttlibrary.MQTTHelper;

public class Common {
    //    public static String Port = "6002";
//    public static String Sever = "tcp://183.230.40.39" + ":" + Port;
    public static String Port = "1883";
    public static String Sever = "tcp://iot-06z00axdhgfk24n.mqtt.iothub.aliyuncs.com" + ":" + Port;

    public static String ReceiveTopic = "/broadcast/h9sjf6crkxG/test2";
    public static String PushTopic = "/broadcast/h9sjf6crkxG/test1";
    public static String DriveID = "h9sjf6crkxG.smartapp|securemode=2,signmethod=hmacsha256,timestamp=1709628535797|";
    public static String DriveName = "smartapp&h9sjf6crkxG";
    public static String DrivePassword = "82f66f0e8319836ec8887477a429f4d2062d503531db8b09718c137eb082abdd";
    public static String Drive2ID = "1181073142";
    public static String api_key = "EtpuIH6FIU3XjavzoDRPSdYAkT8=";
    public static boolean DeviceOnline = false;
    public static String LatestOnlineDate = "离线";
    public static MQTTHelper mqttHelper = null;
}
