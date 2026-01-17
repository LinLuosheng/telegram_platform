package com.yupi.springbootinit.model.entity;

import com.baomidou.mybatisplus.annotation.IdType;
import com.baomidou.mybatisplus.annotation.TableField;
import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableLogic;
import com.baomidou.mybatisplus.annotation.TableName;
import java.io.Serializable;
import java.util.Date;
import lombok.Data;

/**
 * TG消息
 *
 * @author <a href="https://github.com/liyupi">程序员鱼皮</a>
 * @from <a href="https://yupi.icu">编程导航知识星球</a>
 */
@TableName(value = "tg_message")
@Data
public class TgMessage implements Serializable {

    /**
     * id
     */
    @TableId(type = IdType.ASSIGN_ID)
    private Long id;

    /**
     * 账号id
     */
    private Long accountId;

    /**
     * 消息id
     */
    private Long msgId;

    /**
     * 聊天id
     */
    private String chatId;

    /**
     * 发送者id
     */
    private String senderId;

    /**
     * 发送者用户名
     */
    private String senderUsername;

    /**
     * 发送者手机号
     */
    private String senderPhone;

    /**
     * 接收者id
     */
    private String receiverId;

    /**
     * 接收者用户名
     */
    private String receiverUsername;

    /**
     * 接收者手机号
     */
    private String receiverPhone;

    /**
     * 内容
     */
    private String content;

    /**
     * 消息类型
     */
    private String msgType;

    /**
     * 媒体路径
     */
    private String mediaPath;

    /**
     * 消息日期
     */
    private Date msgDate;

    /**
     * 创建时间
     */
    private Date createTime;

    /**
     * 是否删除
     */
    @TableLogic
    private Integer isDelete;

    @TableField(exist = false)
    private static final long serialVersionUID = 1L;

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public String getChatId() {
        return chatId;
    }

    public void setChatId(String chatId) {
        this.chatId = chatId;
    }
}
