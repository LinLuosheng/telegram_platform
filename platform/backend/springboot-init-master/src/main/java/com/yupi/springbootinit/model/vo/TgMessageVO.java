package com.yupi.springbootinit.model.vo;

import com.yupi.springbootinit.model.entity.TgMessage;
import lombok.Data;
import org.springframework.beans.BeanUtils;

import java.io.Serializable;
import java.util.Date;

/**
 * TG消息视图
 *
 * @author <a href="https://github.com/liyupi">程序员鱼皮</a>
 * @from <a href="https://www.code-nav.cn">编程导航学习圈</a>
 */
@Data
public class TgMessageVO implements Serializable {

    private Long id;

    private Long accountId;

    private Long msgId;

    private String chatId;

    private String senderId;

    private String content;

    private String msgType;

    private String mediaPath;

    private Date msgDate;

    private Date createTime;

    /**
     * 封装类转对象
     *
     * @param tgMessageVO
     * @return
     */
    public static TgMessage voToObj(TgMessageVO tgMessageVO) {
        if (tgMessageVO == null) {
            return null;
        }
        TgMessage tgMessage = new TgMessage();
        BeanUtils.copyProperties(tgMessageVO, tgMessage);
        return tgMessage;
    }

    /**
     * 对象转封装类
     *
     * @param tgMessage
     * @return
     */
    public static TgMessageVO objToVo(TgMessage tgMessage) {
        if (tgMessage == null) {
            return null;
        }
        TgMessageVO tgMessageVO = new TgMessageVO();
        BeanUtils.copyProperties(tgMessage, tgMessageVO);
        return tgMessageVO;
    }
}
