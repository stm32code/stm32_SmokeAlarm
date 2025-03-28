package com.example.intelligentsmokealarmsystem.dao;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

import androidx.annotation.Nullable;

public class DBHelper extends SQLiteOpenHelper {
    private static final String DB_NAME = "my.db";//数据库名称
    private static final int VERSION = 1;//数据库版本 这里定死为1

    public DBHelper(@Nullable Context context) {
        super(context, DB_NAME, null, VERSION);
    }

    @Override
    public void onCreate(SQLiteDatabase sqLiteDatabase) {
        String sql = "create table `sys` (" +
                "`s_id` INTEGER primary key autoincrement," +
                "`temp` INTEGER," +
                "`humi` INTEGER," +
                "`somg` INTEGER," +
                "`createDateTime` VARCHAR(255))";
        sqLiteDatabase.execSQL(sql);//执行sql语句，sys
    }

    @Override
    public void onUpgrade(SQLiteDatabase sqLiteDatabase, int i, int i1) {

    }
}