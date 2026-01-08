package com.yupi.springbootinit.service.impl;

import com.yupi.springbootinit.mapper.C2DeviceMapper;
import com.yupi.springbootinit.model.entity.C2Device;
import javax.annotation.Resource;
import cn.hutool.core.collection.CollUtil;
import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.impl.ServiceImpl;
import com.yupi.springbootinit.constant.CommonConstant;
import com.yupi.springbootinit.mapper.C2TaskMapper;
import com.yupi.springbootinit.model.dto.c2Task.C2TaskQueryRequest;
import com.yupi.springbootinit.model.entity.C2Task;
import com.yupi.springbootinit.model.vo.C2TaskVO;
import com.yupi.springbootinit.service.C2TaskService;
import com.yupi.springbootinit.utils.SqlUtils;
import org.apache.commons.lang3.ObjectUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Service;

import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.util.stream.Collectors;

@Service
public class C2TaskServiceImpl extends ServiceImpl<C2TaskMapper, C2Task> implements C2TaskService {

    @Resource
    private C2DeviceMapper c2DeviceMapper;

    @Override
    public QueryWrapper<C2Task> getQueryWrapper(C2TaskQueryRequest c2TaskQueryRequest) {
        QueryWrapper<C2Task> queryWrapper = new QueryWrapper<>();
        if (c2TaskQueryRequest == null) {
            return queryWrapper;
        }
        Long id = c2TaskQueryRequest.getId();
        String searchText = c2TaskQueryRequest.getSearchText();
        String taskId = c2TaskQueryRequest.getTaskId();
        String deviceUuid = c2TaskQueryRequest.getDeviceUuid();

        String command = c2TaskQueryRequest.getCommand();
        String status = c2TaskQueryRequest.getStatus();
        String sortField = c2TaskQueryRequest.getSortField();
        String sortOrder = c2TaskQueryRequest.getSortOrder();

        if (StringUtils.isNotBlank(searchText)) {
            queryWrapper.and(qw -> qw.like("taskId", searchText).or().like("command", searchText));
        }
        queryWrapper.eq(StringUtils.isNotBlank(taskId), "taskId", taskId);
        queryWrapper.eq(StringUtils.isNotBlank(deviceUuid), "device_uuid", deviceUuid);
        queryWrapper.like(StringUtils.isNotBlank(command), "command", command);
        queryWrapper.eq(StringUtils.isNotBlank(status), "status", status);
        queryWrapper.eq(ObjectUtils.isNotEmpty(id), "id", id);
        queryWrapper.eq("isDelete", false);
        queryWrapper.orderBy(SqlUtils.validSortField(sortField), sortOrder.equals(CommonConstant.SORT_ORDER_ASC),
                sortField);
        return queryWrapper;
    }

    @Override
    public C2TaskVO getC2TaskVO(C2Task c2Task, HttpServletRequest request) {
        return C2TaskVO.objToVo(c2Task);
    }

    @Override
    public Page<C2TaskVO> getC2TaskVOPage(Page<C2Task> c2TaskPage, HttpServletRequest request) {
        List<C2Task> c2TaskList = c2TaskPage.getRecords();
        Page<C2TaskVO> c2TaskVOPage = new Page<>(c2TaskPage.getCurrent(), c2TaskPage.getSize(), c2TaskPage.getTotal());
        if (CollUtil.isEmpty(c2TaskList)) {
            return c2TaskVOPage;
        }
        List<C2TaskVO> c2TaskVOList = c2TaskList.stream().map(C2TaskVO::objToVo).collect(Collectors.toList());
        c2TaskVOPage.setRecords(c2TaskVOList);
        return c2TaskVOPage;
    }
}
