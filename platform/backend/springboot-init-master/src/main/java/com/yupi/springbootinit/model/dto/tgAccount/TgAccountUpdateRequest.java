package com.yupi.springbootinit.model.dto.tgAccount;

import lombok.Data;

import java.io.Serializable;

/**
 * 更新TG账号请求
 *
 * @author <a href="https://github.com/liyupi">程序员鱼皮</a>
 * @from <a href="https://www.code-nav.cn">编程导航学习圈</a>
 */
@Data
public class TgAccountUpdateRequest implements Serializable {

    private Long id;

    private String tgId;

    private String username;

    private String phone;

    private String firstName;

    private String lastName;

    private Integer isBot;

    private String systemInfo;

    private static final long serialVersionUID = 1L;
}
