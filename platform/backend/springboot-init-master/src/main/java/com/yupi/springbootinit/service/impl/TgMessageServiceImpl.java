package com.yupi.springbootinit.service.impl;

import cn.hutool.core.collection.CollUtil;
import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.baomidou.mybatisplus.extension.service.impl.ServiceImpl;
import com.yupi.springbootinit.common.ErrorCode;
import com.yupi.springbootinit.constant.CommonConstant;
import com.yupi.springbootinit.exception.ThrowUtils;
import com.yupi.springbootinit.mapper.TgMessageMapper;
import com.yupi.springbootinit.model.dto.tgMessage.TgMessageQueryRequest;
import com.yupi.springbootinit.model.entity.TgMessage;
import com.yupi.springbootinit.model.entity.User;
import com.yupi.springbootinit.model.vo.TgMessageVO;
import com.yupi.springbootinit.model.vo.UserVO;
import com.yupi.springbootinit.service.TgMessageService;
import com.yupi.springbootinit.service.UserService;
import com.yupi.springbootinit.utils.SqlUtils;
import lombok.extern.slf4j.Slf4j;
import org.apache.commons.lang3.ObjectUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Service;

import javax.annotation.Resource;
import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.util.stream.Collectors;

/**
 * TG消息服务实现
 *
 * @author <a href="https://github.com/liyupi">程序员鱼皮</a>
 * @from <a href="https://www.code-nav.cn">编程导航学习圈</a>
 */
@Service
@Slf4j
public class TgMessageServiceImpl extends ServiceImpl<TgMessageMapper, TgMessage> implements TgMessageService {

    @Resource
    private UserService userService;

    /**
     * 校验数据
     *
     * @param tgMessage
     * @param add      对创建的数据进行校验
     */
    @Override
    public void validTgMessage(TgMessage tgMessage, boolean add) {
        ThrowUtils.throwIf(tgMessage == null, ErrorCode.PARAMS_ERROR);
        // todo 从对象中取值
        String chatId = tgMessage.getChatId();
        // 创建数据时，参数不能为空
        if (add) {
            // todo 补充校验规则
            ThrowUtils.throwIf(StringUtils.isBlank(chatId), ErrorCode.PARAMS_ERROR);
        }
        // 修改数据时，有参数则校验
        // todo 补充校验规则
        if (StringUtils.isNotBlank(chatId)) {
            ThrowUtils.throwIf(chatId.length() > 64, ErrorCode.PARAMS_ERROR, "ChatId过长");
        }
    }

    /**
     * 获取查询条件
     *
     * @param tgMessageQueryRequest
     * @return
     */
    @Override
    public QueryWrapper<TgMessage> getQueryWrapper(TgMessageQueryRequest tgMessageQueryRequest) {
        QueryWrapper<TgMessage> queryWrapper = new QueryWrapper<>();
        if (tgMessageQueryRequest == null) {
            return queryWrapper;
        }
        // todo 从对象中取值
        Long id = tgMessageQueryRequest.getId();
        Long accountId = tgMessageQueryRequest.getAccountId();
        Long notId = tgMessageQueryRequest.getNotId();
        String searchText = tgMessageQueryRequest.getSearchText();
        String chatId = tgMessageQueryRequest.getChatId();
        String senderId = tgMessageQueryRequest.getSenderId();
        String msgType = tgMessageQueryRequest.getMsgType();
        String sortField = tgMessageQueryRequest.getSortField();
        String sortOrder = tgMessageQueryRequest.getSortOrder();
        
        // todo 补充需要的查询条件
        // 从多字段中搜索
        if (StringUtils.isNotBlank(searchText)) {
            // 需要拼接查询条件
            queryWrapper.and(qw -> qw.like("content", searchText).or().like("chatId", searchText));
        }
        
        // 精确查询
        queryWrapper.ne(ObjectUtils.isNotEmpty(notId), "id", notId);
        queryWrapper.eq(ObjectUtils.isNotEmpty(id), "id", id);
        queryWrapper.eq(ObjectUtils.isNotEmpty(accountId), "accountId", accountId);
        queryWrapper.eq(StringUtils.isNotBlank(chatId), "chatId", chatId);
        queryWrapper.eq(StringUtils.isNotBlank(senderId), "senderId", senderId);
        queryWrapper.eq(StringUtils.isNotBlank(msgType), "msgType", msgType);
        queryWrapper.eq("isDelete", false);
        // 排序规则
        queryWrapper.orderBy(SqlUtils.validSortField(sortField),
                sortOrder.equals(CommonConstant.SORT_ORDER_ASC),
                sortField);
        return queryWrapper;
    }

    /**
     * 获取TG消息封装
     *
     * @param tgMessage
     * @param request
     * @return
     */
    @Override
    public TgMessageVO getTgMessageVO(TgMessage tgMessage, HttpServletRequest request) {
        if (tgMessage == null) {
            return null;
        }
        TgMessageVO tgMessageVO = TgMessageVO.objToVo(tgMessage);
        // 1. 关联查询用户信息
        // 2. 已登录，获取用户点赞、收藏状态
        return tgMessageVO;
    }

    /**
     * 分页获取TG消息封装
     *
     * @param tgMessagePage
     * @param request
     * @return
     */
    @Override
    public Page<TgMessageVO> getTgMessageVOPage(Page<TgMessage> tgMessagePage, HttpServletRequest request) {
        List<TgMessage> tgMessageList = tgMessagePage.getRecords();
        Page<TgMessageVO> tgMessageVOPage = new Page<>(tgMessagePage.getCurrent(), tgMessagePage.getSize(), tgMessagePage.getTotal());
        if (CollUtil.isEmpty(tgMessageList)) {
            return tgMessageVOPage;
        }
        // 1. 关联查询用户信息
        // 2. 已登录，获取用户点赞、收藏状态
        List<TgMessageVO> tgMessageVOList = tgMessageList.stream().map(tgMessage -> {
            TgMessageVO tgMessageVO = TgMessageVO.objToVo(tgMessage);
            return tgMessageVO;
        }).collect(Collectors.toList());
        tgMessageVOPage.setRecords(tgMessageVOList);
        return tgMessageVOPage;
    }

}
