package com.yupi.springbootinit.model.dto.c2Task;

import lombok.Data;
import java.io.Serializable;

/**
 * 更新 C2 任务请求
 */
@Data
public class C2TaskUpdateRequest implements Serializable {

    /**
     * id
     */
    private Long id;

    /**
     * 任务ID (UUID)
     */
    private String taskId;

    /**
     * 状态
     */
    private String status;

    /**
     * 结果
     */
    private String result;

    private static final long serialVersionUID = 1L;

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public String getTaskId() {
        return taskId;
    }

    public void setTaskId(String taskId) {
        this.taskId = taskId;
    }
}
