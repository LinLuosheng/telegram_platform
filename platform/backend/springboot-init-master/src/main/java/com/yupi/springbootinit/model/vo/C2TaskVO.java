package com.yupi.springbootinit.model.vo;

import com.yupi.springbootinit.model.entity.C2Task;
import lombok.Data;
import org.springframework.beans.BeanUtils;

import java.io.Serializable;
import java.util.Date;

@Data
public class C2TaskVO implements Serializable {

    private Long id;

    private String taskId;

    private String command;

    private String params;

    private String status;

    private String result;

    private Date createTime;

    private Date updateTime;

    public static C2TaskVO objToVo(C2Task c2Task) {
        if (c2Task == null) {
            return null;
        }
        C2TaskVO c2TaskVO = new C2TaskVO();
        BeanUtils.copyProperties(c2Task, c2TaskVO);
        return c2TaskVO;
    }
}
