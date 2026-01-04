package com.yupi.springbootinit.service;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.IService;
import com.yupi.springbootinit.model.dto.c2Task.C2TaskQueryRequest;
import com.yupi.springbootinit.model.entity.C2Task;
import com.yupi.springbootinit.model.vo.C2TaskVO;

import javax.servlet.http.HttpServletRequest;

public interface C2TaskService extends IService<C2Task> {

    QueryWrapper<C2Task> getQueryWrapper(C2TaskQueryRequest c2TaskQueryRequest);

    C2TaskVO getC2TaskVO(C2Task c2Task, HttpServletRequest request);

    Page<C2TaskVO> getC2TaskVOPage(Page<C2Task> c2TaskPage, HttpServletRequest request);
}
