package com.yupi.springbootinit.model.entity;

import com.baomidou.mybatisplus.annotation.*;
import java.io.Serializable;
import java.util.Date;
import lombok.Data;

/**
 * C2 Device
 */
@TableName(value = "c2_device")
@Data
public class C2Device implements Serializable {
    @TableId(type = IdType.AUTO)
    private Long id;

    private String uuid;

    private String internalIp;

    private String externalIp;

    private String macAddress;

    private String hostName;

    private String os;

    private Integer heartbeatInterval;

    /**
     * Is Auto Screenshot Monitor On (0-off, 1-on)
     */
    @TableField("is_monitor_on")
    private Integer isMonitorOn;

    /**
     * Data Collection Status (e.g., Collecting, Uploading, Done)
     */
    @TableField("data_status")
    private String dataStatus;

    private Date lastSeen;

    private Date createTime;

    private Date updateTime;

    @TableLogic
    private Integer isDelete;

    @TableField(exist = false)
    private static final long serialVersionUID = 1L;
}
