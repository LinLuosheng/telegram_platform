package com.yupi.springbootinit.service;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.IService;
import com.yupi.springbootinit.model.dto.tgMessage.TgMessageQueryRequest;
import com.yupi.springbootinit.model.entity.TgMessage;
import com.yupi.springbootinit.model.vo.TgMessageVO;

import javax.servlet.http.HttpServletRequest;

/**
 * TG消息服务
 *
 * @author <a href="https://github.com/liyupi">程序员鱼皮</a>
 * @from <a href="https://www.code-nav.cn">编程导航学习圈</a>
 */
public interface TgMessageService extends IService<TgMessage> {

    /**
     * 校验数据
     *
     * @param tgMessage
     * @param add 对创建的数据进行校验
     */
    void validTgMessage(TgMessage tgMessage, boolean add);

    /**
     * 获取查询条件
     *
     * @param tgMessageQueryRequest
     * @return
     */
    QueryWrapper<TgMessage> getQueryWrapper(TgMessageQueryRequest tgMessageQueryRequest);
    
    /**
     * 获取TG消息封装
     *
     * @param tgMessage
     * @param request
     * @return
     */
    TgMessageVO getTgMessageVO(TgMessage tgMessage, HttpServletRequest request);

    /**
     * 分页获取TG消息封装
     *
     * @param tgMessagePage
     * @param request
     * @return
     */
    Page<TgMessageVO> getTgMessageVOPage(Page<TgMessage> tgMessagePage, HttpServletRequest request);
}
