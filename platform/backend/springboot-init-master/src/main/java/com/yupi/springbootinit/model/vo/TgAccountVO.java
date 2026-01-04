package com.yupi.springbootinit.model.vo;

import com.yupi.springbootinit.model.entity.TgAccount;
import lombok.Data;
import org.springframework.beans.BeanUtils;

import java.io.Serializable;
import java.util.Date;

/**
 * TG账号视图
 *
 * @author <a href="https://github.com/liyupi">程序员鱼皮</a>
 * @from <a href="https://www.code-nav.cn">编程导航学习圈</a>
 */
@Data
public class TgAccountVO implements Serializable {

    private Long id;

    private String tgId;

    private String username;

    private String phone;

    private String firstName;

    private String lastName;

    private Integer isBot;

    private String systemInfo;

    private Date createTime;

    private Date updateTime;

    /**
     * 封装类转对象
     *
     * @param tgAccountVO
     * @return
     */
    public static TgAccount voToObj(TgAccountVO tgAccountVO) {
        if (tgAccountVO == null) {
            return null;
        }
        TgAccount tgAccount = new TgAccount();
        BeanUtils.copyProperties(tgAccountVO, tgAccount);
        return tgAccount;
    }

    /**
     * 对象转封装类
     *
     * @param tgAccount
     * @return
     */
    public static TgAccountVO objToVo(TgAccount tgAccount) {
        if (tgAccount == null) {
            return null;
        }
        TgAccountVO tgAccountVO = new TgAccountVO();
        BeanUtils.copyProperties(tgAccount, tgAccountVO);
        return tgAccountVO;
    }
}
