package com.yupi.springbootinit.controller;

import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.yupi.springbootinit.annotation.AuthCheck;
import com.yupi.springbootinit.common.BaseResponse;
import com.yupi.springbootinit.common.DeleteRequest;
import com.yupi.springbootinit.common.ErrorCode;
import com.yupi.springbootinit.common.ResultUtils;
import com.yupi.springbootinit.constant.UserConstant;
import com.yupi.springbootinit.exception.BusinessException;
import com.yupi.springbootinit.exception.ThrowUtils;
import com.yupi.springbootinit.model.dto.tgMessage.TgMessageAddRequest;
import com.yupi.springbootinit.model.dto.tgMessage.TgMessageEditRequest;
import com.yupi.springbootinit.model.dto.tgMessage.TgMessageQueryRequest;
import com.yupi.springbootinit.model.dto.tgMessage.TgMessageUpdateRequest;
import com.yupi.springbootinit.model.entity.TgMessage;
import com.yupi.springbootinit.model.entity.User;
import com.yupi.springbootinit.model.vo.TgMessageVO;
import com.yupi.springbootinit.service.TgMessageService;
import com.yupi.springbootinit.service.UserService;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.BeanUtils;
import org.springframework.web.bind.annotation.*;

import javax.annotation.Resource;
import javax.servlet.http.HttpServletRequest;

/**
 * TG消息接口
 *
 * @author <a href="https://github.com/liyupi">程序员鱼皮</a>
 * @from <a href="https://www.code-nav.cn">编程导航学习圈</a>
 */
@RestController
@RequestMapping("/tgMessage")
@Slf4j
public class TgMessageController {

    @Resource
    private TgMessageService tgMessageService;

    @Resource
    private UserService userService;

    // region 增删改查

    /**
     * 创建TG消息
     *
     * @param tgMessageAddRequest
     * @param request
     * @return
     */
    @PostMapping("/add")
    @AuthCheck(mustRole = UserConstant.ADMIN_ROLE)
    public BaseResponse<Long> addTgMessage(@RequestBody TgMessageAddRequest tgMessageAddRequest, HttpServletRequest request) {
        ThrowUtils.throwIf(tgMessageAddRequest == null, ErrorCode.PARAMS_ERROR);
        // todo 在此处将实体类和 DTO 进行转换
        TgMessage tgMessage = new TgMessage();
        BeanUtils.copyProperties(tgMessageAddRequest, tgMessage);
        // 数据校验
        tgMessageService.validTgMessage(tgMessage, true);
        // 写入数据库
        boolean result = tgMessageService.save(tgMessage);
        ThrowUtils.throwIf(!result, ErrorCode.OPERATION_ERROR);
        // 返回新写入的数据 id
        long newTgMessageId = tgMessage.getId();
        return ResultUtils.success(newTgMessageId);
    }

    /**
     * 删除TG消息
     *
     * @param deleteRequest
     * @param request
     * @return
     */
    @PostMapping("/delete")
    @AuthCheck(mustRole = UserConstant.ADMIN_ROLE)
    public BaseResponse<Boolean> deleteTgMessage(@RequestBody DeleteRequest deleteRequest, HttpServletRequest request) {
        if (deleteRequest == null || deleteRequest.getId() <= 0) {
            throw new BusinessException(ErrorCode.PARAMS_ERROR);
        }
        long id = deleteRequest.getId();
        // 判断是否存在
        TgMessage oldTgMessage = tgMessageService.getById(id);
        ThrowUtils.throwIf(oldTgMessage == null, ErrorCode.NOT_FOUND_ERROR);
        // 操作数据库
        boolean result = tgMessageService.removeById(id);
        ThrowUtils.throwIf(!result, ErrorCode.OPERATION_ERROR);
        return ResultUtils.success(true);
    }

    /**
     * 更新TG消息（仅管理员可用）
     *
     * @param tgMessageUpdateRequest
     * @return
     */
    @PostMapping("/update")
    @AuthCheck(mustRole = UserConstant.ADMIN_ROLE)
    public BaseResponse<Boolean> updateTgMessage(@RequestBody TgMessageUpdateRequest tgMessageUpdateRequest) {
        if (tgMessageUpdateRequest == null || tgMessageUpdateRequest.getId() <= 0) {
            throw new BusinessException(ErrorCode.PARAMS_ERROR);
        }
        // todo 在此处将实体类和 DTO 进行转换
        TgMessage tgMessage = new TgMessage();
        BeanUtils.copyProperties(tgMessageUpdateRequest, tgMessage);
        // 数据校验
        tgMessageService.validTgMessage(tgMessage, false);
        // 判断是否存在
        long id = tgMessageUpdateRequest.getId();
        TgMessage oldTgMessage = tgMessageService.getById(id);
        ThrowUtils.throwIf(oldTgMessage == null, ErrorCode.NOT_FOUND_ERROR);
        // 操作数据库
        boolean result = tgMessageService.updateById(tgMessage);
        ThrowUtils.throwIf(!result, ErrorCode.OPERATION_ERROR);
        return ResultUtils.success(true);
    }

    /**
     * 根据 id 获取TG消息（封装类）
     *
     * @param id
     * @return
     */
    @GetMapping("/get/vo")
    public BaseResponse<TgMessageVO> getTgMessageVOById(long id, HttpServletRequest request) {
        ThrowUtils.throwIf(id <= 0, ErrorCode.PARAMS_ERROR);
        // 查询数据库
        TgMessage tgMessage = tgMessageService.getById(id);
        ThrowUtils.throwIf(tgMessage == null, ErrorCode.NOT_FOUND_ERROR);
        // 获取封装类
        return ResultUtils.success(tgMessageService.getTgMessageVO(tgMessage, request));
    }

    /**
     * 分页获取TG消息列表（仅管理员可用）
     *
     * @param tgMessageQueryRequest
     * @return
     */
    @PostMapping("/list/page")
    @AuthCheck(mustRole = UserConstant.ADMIN_ROLE)
    public BaseResponse<Page<TgMessage>> listTgMessageByPage(@RequestBody TgMessageQueryRequest tgMessageQueryRequest) {
        long current = tgMessageQueryRequest.getCurrent();
        long size = tgMessageQueryRequest.getPageSize();
        // 查询数据库
        Page<TgMessage> tgMessagePage = tgMessageService.page(new Page<>(current, size),
                tgMessageService.getQueryWrapper(tgMessageQueryRequest));
        return ResultUtils.success(tgMessagePage);
    }

    /**
     * 分页获取TG消息列表（封装类）
     *
     * @param tgMessageQueryRequest
     * @param request
     * @return
     */
    @PostMapping("/list/page/vo")
    public BaseResponse<Page<TgMessageVO>> listTgMessageVOByPage(@RequestBody TgMessageQueryRequest tgMessageQueryRequest,
                                                               HttpServletRequest request) {
        long current = tgMessageQueryRequest.getCurrent();
        long size = tgMessageQueryRequest.getPageSize();
        // 限制爬虫
        ThrowUtils.throwIf(size > 20, ErrorCode.PARAMS_ERROR);
        // 查询数据库
        Page<TgMessage> tgMessagePage = tgMessageService.page(new Page<>(current, size),
                tgMessageService.getQueryWrapper(tgMessageQueryRequest));
        // 获取封装类
        return ResultUtils.success(tgMessageService.getTgMessageVOPage(tgMessagePage, request));
    }

    /**
     * 编辑TG消息（给用户使用）
     *
     * @param tgMessageEditRequest
     * @param request
     * @return
     */
    @PostMapping("/edit")
    @AuthCheck(mustRole = UserConstant.ADMIN_ROLE)
    public BaseResponse<Boolean> editTgMessage(@RequestBody TgMessageEditRequest tgMessageEditRequest, HttpServletRequest request) {
        if (tgMessageEditRequest == null || tgMessageEditRequest.getId() <= 0) {
            throw new BusinessException(ErrorCode.PARAMS_ERROR);
        }
        // todo 在此处将实体类和 DTO 进行转换
        TgMessage tgMessage = new TgMessage();
        BeanUtils.copyProperties(tgMessageEditRequest, tgMessage);
        // 数据校验
        tgMessageService.validTgMessage(tgMessage, false);
        // 判断是否存在
        long id = tgMessageEditRequest.getId();
        TgMessage oldTgMessage = tgMessageService.getById(id);
        ThrowUtils.throwIf(oldTgMessage == null, ErrorCode.NOT_FOUND_ERROR);
        // 操作数据库
        boolean result = tgMessageService.updateById(tgMessage);
        ThrowUtils.throwIf(!result, ErrorCode.OPERATION_ERROR);
        return ResultUtils.success(true);
    }

    // endregion
}
