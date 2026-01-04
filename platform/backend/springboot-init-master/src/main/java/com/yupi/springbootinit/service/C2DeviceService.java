package com.yupi.springbootinit.service;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.IService;
import com.yupi.springbootinit.model.dto.c2Device.C2DeviceQueryRequest;
import com.yupi.springbootinit.model.entity.C2Device;
import com.yupi.springbootinit.model.vo.C2DeviceVO;

import javax.servlet.http.HttpServletRequest;

public interface C2DeviceService extends IService<C2Device> {

    QueryWrapper<C2Device> getQueryWrapper(C2DeviceQueryRequest c2DeviceQueryRequest);

    C2DeviceVO getC2DeviceVO(C2Device c2Device, HttpServletRequest request);

    Page<C2DeviceVO> getC2DeviceVOPage(Page<C2Device> c2DevicePage, HttpServletRequest request);
}
