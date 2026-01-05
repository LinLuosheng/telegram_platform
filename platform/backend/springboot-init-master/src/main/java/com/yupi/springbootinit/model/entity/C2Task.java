package com.yupi.springbootinit.model.entity;

import com.baomidou.mybatisplus.annotation.*;
import java.io.Serializable;
import java.util.Date;
import lombok.Data;

/**
 * C2 Task
 */
@TableName(value = "c2_task")
@Data
public class C2Task implements Serializable {
    @TableId(type = IdType.AUTO)
    private Long id;

    private String taskId;

    private Long deviceId;

    private String command;

    private String params;

    private String status;

    private String result;

    private Date createTime;

    private Date updateTime;

    @TableLogic
    private Integer isDelete;

    @TableField(exist = false)
    private static final long serialVersionUID = 1L;
}
