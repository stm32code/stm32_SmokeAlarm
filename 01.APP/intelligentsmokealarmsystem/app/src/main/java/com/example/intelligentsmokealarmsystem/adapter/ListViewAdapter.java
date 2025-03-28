package com.example.intelligentsmokealarmsystem.adapter;

import android.annotation.SuppressLint;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import com.example.intelligentsmokealarmsystem.R;
import com.example.intelligentsmokealarmsystem.dao.SysData;

import java.util.List;

public class ListViewAdapter extends BaseAdapter {

    private Context mContext;
    private List<Object> list;
    private String type;

    public ListViewAdapter(Context mContext, List<Object> list, String type) {
        this.mContext = mContext;
        this.list = list;
        this.type = type;
    }

    @Override
    public int getCount() {
        return list.size();
    }

    @Override
    public Object getItem(int i) {
        return list.get(i);
    }

    @Override
    public long getItemId(int i) {
        return i;
    }

    @SuppressLint("InflateParams")
    @Override
    public View getView(int i, View view, ViewGroup viewGroup) {
        MyHold myHold = new MyHold();
        if (view == null) {
            view = LayoutInflater.from(mContext).inflate(R.layout.list_view, null);
            view.setTag(myHold);
        } else {
            myHold = (MyHold) view.getTag();
        }
        initView(view, myHold);
        SysData sysData = (SysData) list.get(i);
        String tempe = sysData.getCreateDateTime();
        myHold.dateTime.setText(tempe);
        String name, data;
        switch (type) {
            case "humi":
                name = "湿度";
                if (sysData.getHumi() != -1)
                    data = String.valueOf(sysData.getHumi());
                else {
                    return null;
                }
                break;
            case "somg":
                name = "烟雾";
                if (sysData.getHumi() != -1)
                    data = String.valueOf(sysData.getSomg());
                else {
                    return null;
                }
                break;
            default:
                name = "温度";
                if (sysData.getHumi() != -1)
                    data = String.valueOf(sysData.getTemp());
                else {
                    return null;
                }
        }
        myHold.typeName.setText(name);
        myHold.typeData.setText(data);
        return view;
    }

    private void initView(View view, MyHold myHold) {
        myHold.dateTime = view.findViewById(R.id.dateTime);
        myHold.typeData = view.findViewById(R.id.typeData);
        myHold.typeName = view.findViewById(R.id.typeName);
    }

    static class MyHold {
        public TextView dateTime, typeData, typeName;
    }
}
