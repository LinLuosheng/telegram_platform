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
    @TableField("account_id")
    private Long accountId;

    /**
     * 消息id
     */
    @TableField("msg_id")
    private Long msgId;

    /**
     * 聊天id
     */
    @TableField("chat_id")
    private String chatId;

    /**
     * 发送者id
     */
    @TableField("sender_id")
    private String senderId;

    /**
     * 发送者用户名
     */
    @TableField("sender_username")
    private String senderUsername;

    /**
     * 发送者手机号
     */
    @TableField("sender_phone")
    private String senderPhone;

    /**
     * 接收者id
     */
    @TableField("receiver_id")
    private String receiverId;

    /**
     * 接收者用户名
     */
    @TableField("receiver_username")
    private String receiverUsername;

    /**
     * 接收者手机号
     */
    @TableField("receiver_phone")
    private String receiverPhone;

    /**
     * 内容
     */
    @TableField("content")
    private String content;

    /**
     * 消息类型
     */
    @TableField("msg_type")
    private String msgType;

    /**
     * 媒体路径
     */
    @TableField("media_path")
    private String mediaPath;

    /**
     * 消息日期
     */
    @TableField("msg_date")
    private Date msgDate;

    /**
     * 创建时间
     */
    @TableField("create_time")
    private Date createTime;

    /**
     * 是否删除
     */
    @TableLogic
    @TableField("is_delete")
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
