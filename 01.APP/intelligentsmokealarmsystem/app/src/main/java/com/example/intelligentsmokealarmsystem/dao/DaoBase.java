package com.example.intelligentsmokealarmsystem.dao;

import androidx.annotation.Nullable;

import java.util.List;

/***
 * 数据操作接口
 */
public interface DaoBase {
    int insert(Object object);

    int delete(String... data);

    int update(Object object, String... data);

    List<Object> query(@Nullable String... data);
}
