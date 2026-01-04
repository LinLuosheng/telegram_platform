package com.yupi.springbootinit.model.dto.c2Device;

import com.yupi.springbootinit.common.PageRequest;
import lombok.Data;
import lombok.EqualsAndHashCode;

import java.io.Serializable;

@EqualsAndHashCode(callSuper = true)
@Data
public class C2DeviceQueryRequest extends PageRequest implements Serializable {

    private Long id;

    private String searchText;

    private String ip;

    private String os;

    private static final long serialVersionUID = 1L;
}
