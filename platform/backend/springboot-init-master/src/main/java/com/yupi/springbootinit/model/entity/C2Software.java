package com.yupi.springbootinit.model.entity;

import com.baomidou.mybatisplus.annotation.*;
import java.io.Serializable;
import java.util.Date;
import lombok.Data;

/**
 * C2 Software
 */
@TableName(value = "c2_software")
@Data
public class C2Software implements Serializable {
    @TableId(type = IdType.AUTO)
    private Long id;

    @TableField("device_uuid")
    private String deviceUuid;

    private String name;

    private String version;

    private String installDate;

    private Date createTime;

    @TableLogic
    private Integer isDelete;

    @TableField(exist = false)
    private static final long serialVersionUID = 1L;
}
