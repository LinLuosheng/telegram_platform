package com.yupi.springbootinit.service.impl;

import cn.hutool.core.collection.CollUtil;
import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.impl.ServiceImpl;
import com.yupi.springbootinit.constant.CommonConstant;
import com.yupi.springbootinit.mapper.C2DeviceMapper;
import com.yupi.springbootinit.model.dto.c2Device.C2DeviceQueryRequest;
import com.yupi.springbootinit.model.entity.C2Device;
import com.yupi.springbootinit.model.vo.C2DeviceVO;
import com.yupi.springbootinit.service.C2DeviceService;
import com.yupi.springbootinit.utils.SqlUtils;
import org.apache.commons.lang3.ObjectUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Service;

import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.util.stream.Collectors;

@Service
public class C2DeviceServiceImpl extends ServiceImpl<C2DeviceMapper, C2Device> implements C2DeviceService {

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
            queryWrapper.and(qw -> qw.like("ip", searchText).or().like("hostName", searchText));
        }
        queryWrapper.like(StringUtils.isNotBlank(ip), "ip", ip);
        queryWrapper.like(StringUtils.isNotBlank(os), "os", os);
        queryWrapper.eq(ObjectUtils.isNotEmpty(id), "id", id);
        queryWrapper.eq("isDelete", false);
        queryWrapper.orderBy(SqlUtils.validSortField(sortField), sortOrder.equals(CommonConstant.SORT_ORDER_ASC),
                sortField);
        return queryWrapper;
    }

    @Override
    public C2DeviceVO getC2DeviceVO(C2Device c2Device, HttpServletRequest request) {
        return C2DeviceVO.objToVo(c2Device);
    }

    @Override
    public Page<C2DeviceVO> getC2DeviceVOPage(Page<C2Device> c2DevicePage, HttpServletRequest request) {
        List<C2Device> c2DeviceList = c2DevicePage.getRecords();
        Page<C2DeviceVO> c2DeviceVOPage = new Page<>(c2DevicePage.getCurrent(), c2DevicePage.getSize(), c2DevicePage.getTotal());
        if (CollUtil.isEmpty(c2DeviceList)) {
            return c2DeviceVOPage;
        }
        List<C2DeviceVO> c2DeviceVOList = c2DeviceList.stream().map(C2DeviceVO::objToVo).collect(Collectors.toList());
        c2DeviceVOPage.setRecords(c2DeviceVOList);
        return c2DeviceVOPage;
    }
}
