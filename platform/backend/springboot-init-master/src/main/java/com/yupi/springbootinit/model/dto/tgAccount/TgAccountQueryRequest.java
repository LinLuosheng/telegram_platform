package com.yupi.springbootinit.model.dto.tgAccount;

import com.yupi.springbootinit.common.PageRequest;
import lombok.Data;
import lombok.EqualsAndHashCode;

import java.io.Serializable;

/**
 * 查询TG账号请求
 *
 * @author <a href="https://github.com/liyupi">程序员鱼皮</a>
 * @from <a href="https://www.code-nav.cn">编程导航学习圈</a>
 */
@EqualsAndHashCode(callSuper = true)
@Data
public class TgAccountQueryRequest extends PageRequest implements Serializable {

    private Long id;

    private Long notId;

    private String searchText;

    private String tgId;

    private String username;

    private String phone;

    private static final long serialVersionUID = 1L;
}
