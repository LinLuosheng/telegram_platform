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

    private String internalIp;

    private String externalIp;

    private String hostName;

    private String os;

    private Date lastSeen;

    private Date createTime;

    private Date updateTime;

    @TableLogic
    private Integer isDelete;

    @TableField(exist = false)
    private static final long serialVersionUID = 1L;
}
