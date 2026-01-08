package com.yupi.springbootinit.mapper;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;
import com.yupi.springbootinit.model.entity.C2Device;
import org.apache.ibatis.annotations.Update;

/**
 * C2 Device Mapper
 */
public interface C2DeviceMapper extends BaseMapper<C2Device> {

    @Update("UPDATE c2_device SET isDelete = 0 WHERE isDelete = 1")
    void restoreAll();
}
