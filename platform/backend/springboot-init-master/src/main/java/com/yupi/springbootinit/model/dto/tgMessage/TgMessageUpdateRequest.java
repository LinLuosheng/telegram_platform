package com.yupi.springbootinit.model.dto.tgMessage;

import lombok.Data;

import java.io.Serializable;
import java.util.Date;

/**
 * 更新TG消息请求
 *
 * @author <a href="https://github.com/liyupi">程序员鱼皮</a>
 * @from <a href="https://www.code-nav.cn">编程导航学习圈</a>
 */
@Data
public class TgMessageUpdateRequest implements Serializable {

    private Long id;

    private Long accountId;

    private Long msgId;

    private String chatId;

    private String senderId;

    private String content;

    private String msgType;

    private String mediaPath;

    private Date msgDate;

    private static final long serialVersionUID = 1L;

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }
}
