package com.example.intelligentsmokealarmsystem;

import static com.example.intelligentsmokealarmsystem.utils.TimeCycle.compareDateTime;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.app.DatePickerDialog;
import android.app.TimePickerDialog;
import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.DatePicker;
import android.widget.TextView;

import com.example.intelligentsmokealarmsystem.adapter.ListViewAdapter;
import com.example.intelligentsmokealarmsystem.dao.SysDataDao;
import com.example.intelligentsmokealarmsystem.databinding.ActivityRecordBinding;
import com.example.intelligentsmokealarmsystem.utils.MToast;
import com.gyf.immersionbar.ImmersionBar;

import java.util.Calendar;
import java.util.List;
import java.util.Objects;

public class RecordActivity extends AppCompatActivity {
    private ActivityRecordBinding binding;
    private SysDataDao dao;
    private ListViewAdapter adapter;
    private List<Object> objects = null;
    private String type = "temp";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        binding = ActivityRecordBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        dao = new SysDataDao(this);
        initView();
    }

    private void initView() {
        setSupportActionBar(binding.toolbar);
        binding.toolbarLayout.setTitle("记录");
        ImmersionBar.with(this).init();
        Objects.requireNonNull(getSupportActionBar()).setDisplayHomeAsUpEnabled(true);//添加默认的返回图标
        getSupportActionBar().setHomeButtonEnabled(true); //设置返回键可用
        eventManager();
        objects = dao.query();
        initListView("temp", objects);
    }

    private void initListView(String type, List<Object> objects) {
        this.type = type;
        if (objects != null && objects.size() > 0) {
            showEmptyView(false);
            adapter = new ListViewAdapter(this, objects, type);
            binding.recordListView.setAdapter(adapter);
        } else {
            showEmptyView(true);
        }
    }

    /***
     * 事件监听
     */
    private void eventManager() {
        binding.startBtn.setOnClickListener(view -> {
            showDateTimeDialog(binding.startDateTime, true);
        });

        binding.endBtn.setOnClickListener(view -> {
            showDateTimeDialog(binding.endDateTime, true);
        });

    }


    /**
     * 显示日期弹窗
     *
     * @param view   TextView
     * @param setMax 是否设置日期最大值为当前
     */
    private void showDateTimeDialog(TextView view, boolean setMax) {
        //获取当前系统时间
        Calendar currentTime = Calendar.getInstance();
        DatePickerDialog datePickerDialog = new DatePickerDialog(this, new DatePickerDialog.OnDateSetListener() {
            @Override
            public void onDateSet(DatePicker x_, int year, int month, int day) {
                new TimePickerDialog(RecordActivity.this, (_1, hourOfDay, minute) -> {
                    view.setText(String.format("%4d-%02d-%02d %02d:%02d:00", year, month + 1, day, hourOfDay, minute));
                    if (view == binding.endDateTime && !binding.startDateTime.getText().toString().equals("开始时间")) {
                        if (compareDateTime(binding.endDateTime.getText().toString(), binding.startDateTime.getText().toString()) > 0) {
                            objects = dao.query(binding.startDateTime.getText().toString(), binding.endDateTime.getText().toString());
                            initListView(type, objects);
                        } else {
                            MToast.mToast(RecordActivity.this, "结束时间必须大于开始时间");
                        }
                    }
                }, currentTime.get(Calendar.HOUR_OF_DAY), currentTime.get(Calendar.MINUTE), true).show();
            }
        }, currentTime.get(Calendar.YEAR), currentTime.get(Calendar.MONTH), currentTime.get(Calendar.DAY_OF_MONTH));
        if (setMax) {
            // 设置最大日期值为当前日期
            datePickerDialog.getDatePicker().setMaxDate(currentTime.getTimeInMillis());
        }
        datePickerDialog.show();
    }

    /***
     * 是否显示数据空视图
     * @param isShow boolean
     */
    private void showEmptyView(boolean isShow) {
        binding.nullView.setVisibility(isShow ? View.VISIBLE : View.GONE);
        binding.recordListView.setVisibility(isShow ? View.GONE : View.VISIBLE);
    }

    //填充右上角目录
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.record_menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == R.id.tempView) {
            initListView("temp", objects);
        } else if (item.getItemId() == R.id.humiView) {
            initListView("humi", objects);
        } else if (item.getItemId() == R.id.somgView) {
            initListView("somg", objects);
        } else if (item.getItemId() == android.R.id.home) {
            finish();
        }
        return super.onOptionsItemSelected(item);
    }
}