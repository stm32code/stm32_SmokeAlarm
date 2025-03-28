package com.example.intelligentsmokealarmsystem;

import static com.example.intelligentsmokealarmsystem.utils.Common.DeviceOnline;
import static com.example.intelligentsmokealarmsystem.utils.Common.PushTopic;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.SeekBar;

import com.blankj.utilcode.util.LogUtils;
import com.example.intelligentsmokealarmsystem.bean.DataDTO;
import com.example.intelligentsmokealarmsystem.bean.Receive;
import com.example.intelligentsmokealarmsystem.bean.Send;
import com.example.intelligentsmokealarmsystem.dao.SysData;
import com.example.intelligentsmokealarmsystem.dao.SysDataDao;
import com.example.intelligentsmokealarmsystem.databinding.ActivityMainBinding;
import com.example.intelligentsmokealarmsystem.utils.BeatingAnimation;
import com.example.intelligentsmokealarmsystem.utils.Common;
import com.example.intelligentsmokealarmsystem.utils.DeviceIsOnline;
import com.example.intelligentsmokealarmsystem.utils.MToast;
import com.example.intelligentsmokealarmsystem.utils.TimeCycle;
import com.google.gson.Gson;
import com.gyf.immersionbar.ImmersionBar;
import com.itfitness.mqttlibrary.MQTTHelper;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

import pub.devrel.easypermissions.AppSettingsDialog;
import pub.devrel.easypermissions.EasyPermissions;

public class MainActivity extends AppCompatActivity implements EasyPermissions.PermissionCallbacks {
    private ActivityMainBinding binding;
    private boolean isDebugView = false;//是否显示debug界面
    private final List<String> arrayList = new ArrayList<String>();// debug消息数据
    private ArrayAdapter adapter = null; // debug消息适配器
    //    private boolean onlineFlag = false; //是否在线标识
    private SysDataDao dao;
    private int receiveNum = 0;//接受消息计数器 接收3次存一次

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        dao = new SysDataDao(this);
        initView();
//        getPermission();
        mqttConfig();
        isOnline();
    }

    /***
     * 初始化视图
     */
    private void initView() {
        setSupportActionBar(binding.toolbar);
        binding.toolbarLayout.setTitle(getTitle());
        ImmersionBar.with(this).init();
        debugView();
        warringLayout(false, "");
        eventManager();
        debugView();
    }

    /***
     *事件监听
     */
    private void eventManager() {
        binding.modeSwitch.setOnClickListener(view -> {
            if (Common.mqttHelper.getConnected()) {
                if (binding.modeSwitch.isChecked()) {
                    MToast.mToast(this, "手动模式开");
                } else {
                    MToast.mToast(this, "手动模式关");
                }
                sendMessage(1, binding.modeSwitch.isChecked() ? "1" : "0");
            } else {
                binding.modeSwitch.setChecked(!binding.modeSwitch.isChecked());
                MToast.mToast(this, "请先建立连接");
            }

        });
        binding.beepButton.setOnClickListener(view -> {
            if (Common.mqttHelper.getConnected()) {
                binding.beepButton.setSelected(!binding.beepButton.isSelected());
                sendMessage(3, binding.beepButton.isSelected() ? "1" : "0");
            } else {
                MToast.mToast(this, "请先建立连接");
            }
        });
        binding.somgSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            //滑块在变化
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                binding.somgSeekText.setText(String.valueOf(i));
            }

            //开始
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            //结束
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                binding.somgSeekText.setText(String.valueOf(seekBar.getProgress()));
                if (Common.mqttHelper.getConnected()) {
                    sendMessage(2, binding.tempSeekText.getText().toString(), binding.humiSeekText.getText().toString(), String.valueOf(seekBar.getProgress()));
                } else {
                    MToast.mToast(MainActivity.this, "请先建立连接");
                }
            }
        });
        binding.tempSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            //滑块在变化
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                binding.tempSeekText.setText(String.valueOf(i));
            }

            //开始
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            //结束
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                binding.tempSeekText.setText(String.valueOf(seekBar.getProgress()));
                if (Common.mqttHelper.getConnected()) {
                    sendMessage(2, String.valueOf(seekBar.getProgress()), binding.humiSeekText.getText().toString(), binding.somgSeekText.getText().toString());
                } else {
                    MToast.mToast(MainActivity.this, "请先建立连接");
                }
            }
        });
        binding.humiSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            //滑块在变化
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                binding.humiSeekText.setText(String.valueOf(i));
            }

            //开始
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            //结束
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                binding.humiSeekText.setText(String.valueOf(seekBar.getProgress()));
                if (Common.mqttHelper.getConnected()) {
                    sendMessage(2, binding.tempSeekText.getText().toString(), String.valueOf(seekBar.getProgress()), binding.somgSeekText.getText().toString());
                } else {
                    MToast.mToast(MainActivity.this, "请先建立连接");
                }
            }
        });
    }

    /**
     * @brief debug界面的初始化
     */
    private void debugView() {
        adapter = new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, arrayList);
        binding.debugView.setAdapter(adapter);
    }

    /***
     * mqtt配置
     */
    private void mqttConfig() {
        Common.mqttHelper = new MQTTHelper(this, Common.Sever, Common.DriveID, Common.DriveName, Common.DrivePassword, true, 30, 60);

        try {
            Common.mqttHelper.connect(Common.ReceiveTopic, 1, true, new MqttCallback() {
                @Override
                public void connectionLost(Throwable cause) {

                }

                @Override
                public void messageArrived(String topic, MqttMessage message) {
                    //收到消息
                    Receive data = new Gson().fromJson(message.toString(), Receive.class);
                    LogUtils.eTag("接收到消息", message.getPayload() != null ? new String(message.getPayload()) : "");
//                    onlineFlag = true;
                    binding.online.setText("在线");
                    debugViewData(2, message.getPayload() != null ? new String(message.getPayload()) : "");
                    System.out.println(data);
                    analysisOfData(data);
                }

                @Override
                public void deliveryComplete(IMqttDeliveryToken token) {

                }
            });
        } catch (MqttException e) {
            e.printStackTrace();
            MToast.mToast(this, "MQTT连接错误");
        }
    }

    /**
     * 数据处理
     *
     * @param data
     */
    private void analysisOfData(Receive data) {
        try {
            boolean isInsert = true;
            SysData sysData = new SysData();
            receiveNum++;
            if (data.getHumi() != null) {
                String humi = String.format("%.0f", Float.parseFloat(data.getHumi()));
                binding.humiText.setText(humi);
                sysData.setHumi(Integer.parseInt(humi));
            } else {
                isInsert = false;
                sysData.setHumi(-1);
            }
            if (data.getHumi_v() != null) {
                binding.humiSeekBar.setProgress(Integer.parseInt(data.getHumi_v()));
                binding.humiSeekText.setText(String.valueOf(binding.humiSeekBar.getProgress()));
            }
            if (data.getTemp() != null) {
                String temp = String.format("%.0f", Float.parseFloat(data.getTemp()));
                binding.tempText.setText(temp);
                sysData.setTemp(Integer.parseInt(temp));
            } else {
                isInsert = false;
                sysData.setTemp(-1);
            }
            if (data.getTemp_v() != null) {
                binding.tempSeekBar.setProgress(Integer.parseInt(data.getTemp_v()));
                binding.tempSeekText.setText(String.valueOf(binding.tempSeekBar.getProgress()));
            }
            if (data.getSomg() != null) {
                String somg = String.format("%.0f", Float.parseFloat(data.getSomg()));
                binding.somgText.setText(somg);
                sysData.setSomg(Integer.parseInt(somg));
            } else {
                isInsert = false;
                sysData.setSomg(-1);
            }
            if (data.getSomg_v() != null) {
                binding.somgSeekBar.setProgress(Integer.parseInt(data.getSomg_v()));
                binding.somgSeekText.setText(String.valueOf(binding.somgSeekBar.getProgress()));
            }
            if (data.getFire() != null) {
                binding.fireText.setText(data.getFire().equals("1") ? "检测到火焰" : "正常");
                warringLayout(!binding.fireText.getText().equals("正常"), "检测到火焰");
            }
            if (data.getFlage() != null) {
                binding.modeSwitch.setChecked(data.getFlage().equals("1"));
            }
            if (data.getBeep() != null) {
                binding.beepButton.setSelected(data.getBeep().equals("1"));
            }
            if (isInsert && receiveNum == 3) {
                receiveNum = 0;
                dao.insert(sysData);
            }
        } catch (Exception e) {
            e.printStackTrace();
            MToast.mToast(this, "数据失败");
        }
    }

    /**
     * @brief 动态获取权限
     */
    private void getPermission() {

        List<String> perms = new ArrayList<>();
        perms.add(Manifest.permission.ACCESS_WIFI_STATE);
        perms.add(Manifest.permission.CHANGE_WIFI_STATE);
        perms.add(Manifest.permission.INTERNET);
        perms.add(Manifest.permission.ACCESS_NETWORK_STATE);
        perms.add(Manifest.permission.READ_PHONE_STATE);
        perms.add(Manifest.permission.WAKE_LOCK);
        perms.add(Manifest.permission.WRITE_EXTERNAL_STORAGE);
        if (!EasyPermissions.hasPermissions(this, perms.toArray(new String[0]))) {
            //请求权限
            EasyPermissions.requestPermissions(this, "这是必要的权限", 100, perms.toArray(new String[0]));
        }

    }

    /***
     * 判断硬件是否在线
     */
    /***
     * 判断硬件是否在线
     */
    private void isOnline() {
        new Thread(() -> {
            int i = 0;
            try {
                while (true) {
                    if (i > 3) {
                        i = 0;
                        runOnUiThread(() -> binding.online.setText(DeviceOnline ? "在线" : "离线"));
                        DeviceOnline = false;
                    }
                    i++;
                    Thread.sleep(15000);

                }
            } catch (Exception e) {
                e.printStackTrace();
            }

        }).start();

    }

    /**
     * 发送数据
     *
     * @param cmd
     * @param message
     */
    private void sendMessage(int cmd, String... message) {
        if (Common.mqttHelper.getConnected() && Common.mqttHelper.getSubscription()) {
            String str = "";
            switch (cmd) {
                case 1:
                    DataDTO dto = new DataDTO();
                    dto.setFlage(Integer.parseInt(message[0]));
                    Send send = new Send();
                    send.setCmd(cmd);
                    send.setData(dto);
                    str = new Gson().toJson(send);
                    break;
                case 2:
                    DataDTO dto1 = new DataDTO();
                    dto1.setTemp_v(Integer.parseInt(message[0]));
                    dto1.setHumi_v(Integer.parseInt(message[1]));
                    dto1.setSomg_v(Integer.parseInt(message[2]));
                    Send send1 = new Send();
                    send1.setCmd(cmd);
                    send1.setData(dto1);
                    str = new Gson().toJson(send1);
                    break;
                case 3:
                    DataDTO dto2 = new DataDTO();
                    dto2.setBeep(Integer.parseInt(message[0]));
                    Send send2 = new Send();
                    send2.setCmd(cmd);
                    send2.setData(dto2);
                    str = new Gson().toJson(send2);
                    break;
            }

            Common.mqttHelper.publish(PushTopic, str, 1);

            debugViewData(1, str);
        }
    }

    /**
     * @param visibility 是否显示
     * @param str        显示内容
     * @brief 显示警告弹窗和设置弹窗内容
     */
    private void warringLayout(boolean visibility, String str) {
        if (visibility) {
            binding.warringLayout.setVisibility(View.VISIBLE);
            binding.warringText.setText(str);
            new BeatingAnimation().onAnimation(binding.warringImage);
        } else {
            binding.warringLayout.setVisibility(View.GONE);
        }
    }

    /**
     * @param str  如果为 1 添加发送数据到界面   为 2 添加接受消息到界面
     * @param data 数据字符串
     * @brief debug界面数据添加
     */
    private void debugViewData(int str, String data) {
        if (arrayList.size() >= 255) {
            arrayList.clear();
        }
        runOnUiThread(() -> {
            switch (str) {
                case 1: //发送的消息
                    arrayList.add("目标主题:" + Common.ReceiveTopic + " \n时间:" + TimeCycle.getDateTime() + "\n发送消息:" + data);
                    break;
                case 2:
                    arrayList.add("来自主题:" + Common.ReceiveTopic + " \n时间:" + TimeCycle.getDateTime() + "\n接到消息:" + data);
                    break;
            }
            // 在添加新数据之后调用以下方法，滚动到列表底部
            binding.debugView.post(() -> {
                binding.debugView.setSelection(adapter != null ? adapter.getCount() - 1 : 0);
            });
            if (adapter != null) {
                adapter.notifyDataSetChanged();
            }
        });
    }

    //权限申请回调
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        //将申请权限结果的回调交由easypermissions处理
        EasyPermissions.onRequestPermissionsResult(requestCode, permissions, grantResults, this);
    }

    // 授权（拒绝也算）
    @Override
    public void onPermissionsGranted(int requestCode, @NonNull List<String> perms) {
//        getPermission();
    }

    @Override
    public void onPermissionsDenied(int requestCode, @NonNull List<String> perms) {
        /**
         * 若是在权限弹窗中，用户勾选了'不在提示'，且拒绝权限。
         * 这时候，需要跳转到设置界面去，让用户手动开启。
         */
        if (EasyPermissions.somePermissionPermanentlyDenied(this, perms)) {
            new AppSettingsDialog.Builder(this).build().show();
        }
    }

    //填充右上角目录
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_scrolling, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        if (item.getItemId() == R.id.setDebugView) {
            isDebugView = !isDebugView;
            binding.debugView.setVisibility(isDebugView ? View.VISIBLE : View.GONE);
        } else if (item.getItemId() == R.id.setHistoryView) {
            startActivity(new Intent(this, RecordActivity.class));
        } else {
            super.onOptionsItemSelected(item);
        }
        return true;
    }
}