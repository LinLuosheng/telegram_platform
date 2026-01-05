package com.yupi.springbootinit.controller;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.yupi.springbootinit.common.BaseResponse;
import com.yupi.springbootinit.common.DeleteRequest;
import com.yupi.springbootinit.common.ErrorCode;
import com.yupi.springbootinit.common.ResultUtils;
import com.yupi.springbootinit.exception.BusinessException;
import com.yupi.springbootinit.exception.ThrowUtils;
import com.yupi.springbootinit.model.dto.c2Task.C2TaskAddRequest;
import com.yupi.springbootinit.model.dto.c2Task.C2TaskQueryRequest;
import com.yupi.springbootinit.model.dto.c2Task.C2TaskUpdateRequest;
import com.yupi.springbootinit.model.entity.C2Task;
import com.yupi.springbootinit.model.vo.C2TaskVO;
import com.yupi.springbootinit.service.C2TaskService;
import lombok.extern.slf4j.Slf4j;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeanUtils;
import org.springframework.web.bind.annotation.*;

import javax.annotation.Resource;
import javax.servlet.http.HttpServletRequest;
import java.util.UUID;

@RestController
@RequestMapping("/c2Task")
@Slf4j
public class C2TaskController {

    @Resource
    private C2TaskService c2TaskService;

    @PostMapping("/add")
    public BaseResponse<Long> addC2Task(@RequestBody C2TaskAddRequest c2TaskAddRequest, HttpServletRequest request) {
        ThrowUtils.throwIf(c2TaskAddRequest == null, ErrorCode.PARAMS_ERROR);
        C2Task c2Task = new C2Task();
        BeanUtils.copyProperties(c2TaskAddRequest, c2Task);
        c2Task.setTaskId(UUID.randomUUID().toString());
        c2Task.setStatus("pending");
        boolean result = c2TaskService.save(c2Task);
        ThrowUtils.throwIf(!result, ErrorCode.OPERATION_ERROR);
        return ResultUtils.success(c2Task.getId());
    }

    @PostMapping("/list/page/vo")
    public BaseResponse<Page<C2TaskVO>> listC2TaskVOByPage(@RequestBody C2TaskQueryRequest c2TaskQueryRequest,
                                                               HttpServletRequest request) {
        long current = c2TaskQueryRequest.getCurrent();
        long size = c2TaskQueryRequest.getPageSize();
        // 限制爬虫
        ThrowUtils.throwIf(size > 200, ErrorCode.PARAMS_ERROR);
        Page<C2Task> c2TaskPage = c2TaskService.page(new Page<>(current, size),
                c2TaskService.getQueryWrapper(c2TaskQueryRequest));
        return ResultUtils.success(c2TaskService.getC2TaskVOPage(c2TaskPage, request));
    }

    @PostMapping("/update")
    public BaseResponse<Boolean> updateC2Task(@RequestBody C2TaskUpdateRequest c2TaskUpdateRequest,
                                              HttpServletRequest request) {
        if (c2TaskUpdateRequest == null || (c2TaskUpdateRequest.getId() == null && c2TaskUpdateRequest.getTaskId() == null)) {
            throw new BusinessException(ErrorCode.PARAMS_ERROR);
        }
        C2Task c2Task = new C2Task();
        BeanUtils.copyProperties(c2TaskUpdateRequest, c2Task);

        // 如果提供了 taskId 但没有 id，尝试查找 id
        if (c2Task.getId() == null && StringUtils.isNotBlank(c2Task.getTaskId())) {
            C2Task oldTask = c2TaskService.getOne(new QueryWrapper<C2Task>().eq("taskId", c2Task.getTaskId()));
            if (oldTask != null) {
                c2Task.setId(oldTask.getId());
            } else {
                throw new BusinessException(ErrorCode.NOT_FOUND_ERROR);
            }
        }

        boolean result = c2TaskService.updateById(c2Task);
        ThrowUtils.throwIf(!result, ErrorCode.OPERATION_ERROR);
        return ResultUtils.success(true);
    }
    
    @PostMapping("/delete")
    public BaseResponse<Boolean> deleteC2Task(@RequestBody DeleteRequest deleteRequest, HttpServletRequest request) {
        if (deleteRequest == null || deleteRequest.getId() <= 0) {
            throw new BusinessException(ErrorCode.PARAMS_ERROR);
        }
        long id = deleteRequest.getId();
        boolean result = c2TaskService.removeById(id);
        ThrowUtils.throwIf(!result, ErrorCode.OPERATION_ERROR);
        return ResultUtils.success(true);
    }
}
