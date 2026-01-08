package com.yupi.springbootinit.model.entity;

import com.baomidou.mybatisplus.annotation.*;
import java.io.Serializable;
import java.util.Date;
import lombok.Data;

/**
 * C2 File Scan Result
 */
@TableName(value = "c2_file_scan")
@Data
public class C2FileScan implements Serializable {
    @TableId(type = IdType.AUTO)
    private Long id;

    @TableField("device_uuid")
    private String deviceUuid;

    private String filePath;

    private String fileName;

    private Long fileSize;

    private String md5;

    private Integer isRecent;

    private Date lastModified;

    private Date createTime;

    @TableLogic
    private Integer isDelete;

    @TableField(exist = false)
    private static final long serialVersionUID = 1L;
}
