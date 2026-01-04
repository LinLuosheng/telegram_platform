package com.yupi.springbootinit.model.dto.c2Task;

import lombok.Data;
import java.io.Serializable;

@Data
public class C2TaskAddRequest implements Serializable {
    private Long deviceId;
    private String command;
    private String params;
    private static final long serialVersionUID = 1L;
}
