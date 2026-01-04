package com.yupi.springbootinit.model.vo;

import com.yupi.springbootinit.model.entity.C2Device;
import lombok.Data;
import org.springframework.beans.BeanUtils;

import java.io.Serializable;
import java.util.Date;

@Data
public class C2DeviceVO implements Serializable {

    private Long id;

    private String ip;

    private String hostName;

    private String os;

    private Date lastSeen;

    private Date createTime;

    private Date updateTime;

    public static C2DeviceVO objToVo(C2Device c2Device) {
        if (c2Device == null) {
            return null;
        }
        C2DeviceVO c2DeviceVO = new C2DeviceVO();
        BeanUtils.copyProperties(c2Device, c2DeviceVO);
        return c2DeviceVO;
    }
}
