package com.yupi.springbootinit.service;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.IService;
import com.yupi.springbootinit.model.dto.tgAccount.TgAccountQueryRequest;
import com.yupi.springbootinit.model.entity.TgAccount;
import com.yupi.springbootinit.model.vo.TgAccountVO;

import javax.servlet.http.HttpServletRequest;

/**
 * TG账号服务
 *
 * @author <a href="https://github.com/liyupi">程序员鱼皮</a>
 * @from <a href="https://www.code-nav.cn">编程导航学习圈</a>
 */
public interface TgAccountService extends IService<TgAccount> {

    /**
     * 校验数据
     *
     * @param tgAccount
     * @param add 对创建的数据进行校验
     */
    void validTgAccount(TgAccount tgAccount, boolean add);

    /**
     * 获取查询条件
     *
     * @param tgAccountQueryRequest
     * @return
     */
    QueryWrapper<TgAccount> getQueryWrapper(TgAccountQueryRequest tgAccountQueryRequest);
    
    /**
     * 获取TG账号封装
     *
     * @param tgAccount
     * @param request
     * @return
     */
    TgAccountVO getTgAccountVO(TgAccount tgAccount, HttpServletRequest request);

    /**
     * 分页获取TG账号封装
     *
     * @param tgAccountPage
     * @param request
     * @return
     */
    Page<TgAccountVO> getTgAccountVOPage(Page<TgAccount> tgAccountPage, HttpServletRequest request);
}
