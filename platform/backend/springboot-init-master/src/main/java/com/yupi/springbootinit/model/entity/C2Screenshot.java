package com.yupi.springbootinit.model.entity;

import com.baomidou.mybatisplus.annotation.*;
import java.io.Serializable;
import java.util.Date;
import lombok.Data;

/**
 * C2 Screenshot
 */
@TableName(value = "c2_screenshot")
@Data
public class C2Screenshot implements Serializable {
    @TableId(type = IdType.AUTO)
    private Long id;

    @TableField("device_uuid")
    private String deviceUuid;

    @TableField("task_id")
    private String taskId;

    @TableField("url")
    private String url; // URL to access the image

    @TableField("ocr_result")
    private String ocrResult; // OCR Text

    @TableField("create_time")
    private Date createTime;

    @TableLogic
    @TableField("is_delete")
    private Integer isDelete;

    @TableField(exist = false)
    private static final long serialVersionUID = 1L;
}
