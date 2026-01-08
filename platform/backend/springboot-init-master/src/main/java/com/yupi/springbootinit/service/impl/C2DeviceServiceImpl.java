package com.yupi.springbootinit.service.impl;

import cn.hutool.core.collection.CollUtil;
import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.impl.ServiceImpl;
import com.yupi.springbootinit.constant.CommonConstant;
import com.yupi.springbootinit.mapper.C2DeviceMapper;
import com.yupi.springbootinit.mapper.C2TaskMapper;
import com.yupi.springbootinit.model.dto.c2Device.C2DeviceQueryRequest;
import com.yupi.springbootinit.model.entity.C2Device;
import com.yupi.springbootinit.model.entity.C2Task;
import com.yupi.springbootinit.model.vo.C2DeviceVO;
import com.yupi.springbootinit.service.C2DeviceService;
import com.yupi.springbootinit.utils.SqlUtils;
import org.apache.commons.lang3.ObjectUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Service;

import javax.annotation.Resource;
import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.util.stream.Collectors;

@Service
public class C2DeviceServiceImpl extends ServiceImpl<C2DeviceMapper, C2Device> implements C2DeviceService {

    @Resource
    private C2TaskMapper c2TaskMapper;

    @Resource
    private C2DeviceMapper c2DeviceMapper;

    @Override
    public void restoreAll() {
        c2DeviceMapper.restoreAll();
    }

    @Override
    public QueryWrapper<C2Device> getQueryWrapper(C2DeviceQueryRequest c2DeviceQueryRequest) {
        QueryWrapper<C2Device> queryWrapper = new QueryWrapper<>();
        if (c2DeviceQueryRequest == null) {
            return queryWrapper;
        }
        Long id = c2DeviceQueryRequest.getId();
        String searchText = c2DeviceQueryRequest.getSearchText();
        String ip = c2DeviceQueryRequest.getIp();
        String os = c2DeviceQueryRequest.getOs();
        String sortField = c2DeviceQueryRequest.getSortField();
        String sortOrder = c2DeviceQueryRequest.getSortOrder();

        if (StringUtils.isNotBlank(searchText)) {
            queryWrapper.and(qw -> qw.like("internalIp", searchText).or().like("externalIp", searchText).or().like("hostName", searchText));
        }
        queryWrapper.like(StringUtils.isNotBlank(ip), "externalIp", ip);
        queryWrapper.like(StringUtils.isNotBlank(os), "os", os);
        queryWrapper.eq(ObjectUtils.isNotEmpty(id), "id", id);
        queryWrapper.eq("isDelete", false);
        queryWrapper.orderBy(SqlUtils.validSortField(sortField), sortOrder.equals(CommonConstant.SORT_ORDER_ASC),
                sortField);
        return queryWrapper;
    }

    @Override
    public C2DeviceVO getC2DeviceVO(C2Device c2Device, HttpServletRequest request) {
        C2DeviceVO c2DeviceVO = C2DeviceVO.objToVo(c2Device);
        populateTaskData(c2DeviceVO);
        return c2DeviceVO;
    }

    @Override
    public Page<C2DeviceVO> getC2DeviceVOPage(Page<C2Device> c2DevicePage, HttpServletRequest request) {
        List<C2Device> c2DeviceList = c2DevicePage.getRecords();
        Page<C2DeviceVO> c2DeviceVOPage = new Page<>(c2DevicePage.getCurrent(), c2DevicePage.getSize(), c2DevicePage.getTotal());
        if (CollUtil.isEmpty(c2DeviceList)) {
            return c2DeviceVOPage;
        }
        List<C2DeviceVO> c2DeviceVOList = c2DeviceList.stream().map(c2Device -> {
            C2DeviceVO c2DeviceVO = C2DeviceVO.objToVo(c2Device);
            populateTaskData(c2DeviceVO);
            return c2DeviceVO;
        }).collect(Collectors.toList());
        c2DeviceVOPage.setRecords(c2DeviceVOList);
        return c2DeviceVOPage;
    }

    private void populateTaskData(C2DeviceVO c2DeviceVO) {
        if (c2DeviceVO == null || c2DeviceVO.getUuid() == null) {
            return;
        }
        String deviceUuid = c2DeviceVO.getUuid();

        // Helper to get latest task result
        try {
            c2DeviceVO.setSoftwareList(getLatestTaskResult(deviceUuid, "get_software"));
            c2DeviceVO.setWifiData(getLatestTaskResult(deviceUuid, "get_wifi"));
            c2DeviceVO.setFileList(getLatestTaskResult(deviceUuid, "scan_disk"));
            c2DeviceVO.setRecentFiles(getLatestTaskResult(deviceUuid, "scan_recent"));
        } catch (Exception e) {
            // Log error but don't fail the request
            System.err.println("Error populating task data for device UUID " + deviceUuid + ": " + e.getMessage());
        }
    }

    private String getLatestTaskResult(String deviceUuid, String command) {
        try {
            QueryWrapper<C2Task> queryWrapper = new QueryWrapper<>();
            if (deviceUuid != null && !deviceUuid.isEmpty()) {
                queryWrapper.eq("device_uuid", deviceUuid);
            } else {
                return null;
            }
            queryWrapper.eq("command", command);
            queryWrapper.isNotNull("result");
            queryWrapper.orderByDesc("createTime");
            queryWrapper.last("limit 1");
            C2Task task = c2TaskMapper.selectOne(queryWrapper);
            return task != null ? task.getResult() : null;
        } catch (Exception e) {
             return null;
        }
    }
}
