package com.yupi.springbootinit.model.dto.c2Task;

import com.yupi.springbootinit.common.PageRequest;
import lombok.Data;
import lombok.EqualsAndHashCode;

import java.io.Serializable;

@EqualsAndHashCode(callSuper = true)
@Data
public class C2TaskQueryRequest extends PageRequest implements Serializable {

    private Long id;

    private String searchText;

    private String taskId;

    private String deviceUuid;

    private String command;

    private String status;

    private static final long serialVersionUID = 1L;
}
