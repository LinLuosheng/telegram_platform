package com.yupi.springbootinit.controller;

import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.yupi.springbootinit.common.BaseResponse;
import com.yupi.springbootinit.common.DeleteRequest;
import com.yupi.springbootinit.common.ErrorCode;
import com.yupi.springbootinit.common.ResultUtils;
import com.yupi.springbootinit.exception.BusinessException;
import com.yupi.springbootinit.exception.ThrowUtils;
import com.yupi.springbootinit.model.dto.c2Device.C2DeviceQueryRequest;
import com.yupi.springbootinit.model.entity.C2Device;
import com.yupi.springbootinit.model.vo.C2DeviceVO;
import com.yupi.springbootinit.service.C2DeviceService;
import lombok.extern.slf4j.Slf4j;
import org.springframework.web.bind.annotation.*;

import javax.annotation.Resource;
import javax.servlet.http.HttpServletRequest;

@RestController
@RequestMapping("/c2Device")
@Slf4j
public class C2DeviceController {

    @Resource
    private C2DeviceService c2DeviceService;

    @PostMapping("/list/page/vo")
    public BaseResponse<Page<C2DeviceVO>> listC2DeviceVOByPage(@RequestBody C2DeviceQueryRequest c2DeviceQueryRequest,
                                                               HttpServletRequest request) {
        long current = c2DeviceQueryRequest.getCurrent();
        long size = c2DeviceQueryRequest.getPageSize();
        ThrowUtils.throwIf(size > 20, ErrorCode.PARAMS_ERROR);
        Page<C2Device> c2DevicePage = c2DeviceService.page(new Page<>(current, size),
                c2DeviceService.getQueryWrapper(c2DeviceQueryRequest));
        return ResultUtils.success(c2DeviceService.getC2DeviceVOPage(c2DevicePage, request));
    }

    @PostMapping("/update/heartbeat")
    public BaseResponse<Boolean> updateHeartbeat(@RequestBody C2Device c2Device, HttpServletRequest request) {
        if (c2Device == null || c2Device.getId() == null || c2Device.getHeartbeatInterval() == null) {
            throw new BusinessException(ErrorCode.PARAMS_ERROR);
        }
        C2Device updateDevice = new C2Device();
        updateDevice.setId(c2Device.getId());
        updateDevice.setHeartbeatInterval(c2Device.getHeartbeatInterval());
        boolean result = c2DeviceService.updateById(updateDevice);
        ThrowUtils.throwIf(!result, ErrorCode.OPERATION_ERROR);
        return ResultUtils.success(true);
    }
    
    @PostMapping("/delete")
    public BaseResponse<Boolean> deleteC2Device(@RequestBody DeleteRequest deleteRequest, HttpServletRequest request) {
        if (deleteRequest == null || deleteRequest.getId() <= 0) {
            throw new BusinessException(ErrorCode.PARAMS_ERROR);
        }
        long id = deleteRequest.getId();
        boolean result = c2DeviceService.removeById(id);
        ThrowUtils.throwIf(!result, ErrorCode.OPERATION_ERROR);
        return ResultUtils.success(true);
    }
}
