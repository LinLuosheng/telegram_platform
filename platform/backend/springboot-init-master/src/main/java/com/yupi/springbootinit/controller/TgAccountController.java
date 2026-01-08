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
import com.yupi.springbootinit.model.dto.tgAccount.TgAccountAddRequest;
import com.yupi.springbootinit.model.dto.tgAccount.TgAccountEditRequest;
import com.yupi.springbootinit.model.dto.tgAccount.TgAccountQueryRequest;
import com.yupi.springbootinit.model.dto.tgAccount.TgAccountUpdateRequest;
import com.yupi.springbootinit.model.entity.TgAccount;
import com.yupi.springbootinit.model.entity.User;
import com.yupi.springbootinit.model.vo.TgAccountVO;
import com.yupi.springbootinit.service.TgAccountService;
import com.yupi.springbootinit.service.UserService;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.BeanUtils;
import org.springframework.web.bind.annotation.*;

import javax.annotation.Resource;
import javax.servlet.http.HttpServletRequest;

/**
 * TG账号接口
 *
 * @author <a href="https://github.com/liyupi">程序员鱼皮</a>
 * @from <a href="https://www.code-nav.cn">编程导航学习圈</a>
 */
@RestController
@RequestMapping("/tgAccount")
@Slf4j
public class TgAccountController {

    @Resource
    private TgAccountService tgAccountService;

    @Resource
    private UserService userService;

    // region 增删改查

    /**
     * 创建TG账号
     *
     * @param tgAccountAddRequest
     * @param request
     * @return
     */
    @PostMapping("/add")
    @AuthCheck(mustRole = UserConstant.ADMIN_ROLE)
    public BaseResponse<Long> addTgAccount(@RequestBody TgAccountAddRequest tgAccountAddRequest, HttpServletRequest request) {
        ThrowUtils.throwIf(tgAccountAddRequest == null, ErrorCode.PARAMS_ERROR);
        // todo 在此处将实体类和 DTO 进行转换
        TgAccount tgAccount = new TgAccount();
        BeanUtils.copyProperties(tgAccountAddRequest, tgAccount);
        // 数据校验
        tgAccountService.validTgAccount(tgAccount, true);
        // 写入数据库
        boolean result = tgAccountService.save(tgAccount);
        ThrowUtils.throwIf(!result, ErrorCode.OPERATION_ERROR);
        // 返回新写入的数据 id
        long newTgAccountId = tgAccount.getId();
        return ResultUtils.success(newTgAccountId);
    }

    /**
     * 删除TG账号
     *
     * @param deleteRequest
     * @param request
     * @return
     */
    @PostMapping("/delete")
    @AuthCheck(mustRole = UserConstant.ADMIN_ROLE)
    public BaseResponse<Boolean> deleteTgAccount(@RequestBody DeleteRequest deleteRequest, HttpServletRequest request) {
        if (deleteRequest == null || deleteRequest.getId() <= 0) {
            throw new BusinessException(ErrorCode.PARAMS_ERROR);
        }
        long id = deleteRequest.getId();
        // 判断是否存在
        TgAccount oldTgAccount = tgAccountService.getById(id);
        ThrowUtils.throwIf(oldTgAccount == null, ErrorCode.NOT_FOUND_ERROR);
        // 操作数据库
        boolean result = tgAccountService.removeById(id);
        ThrowUtils.throwIf(!result, ErrorCode.OPERATION_ERROR);
        return ResultUtils.success(true);
    }

    /**
     * 更新TG账号（仅管理员可用）
     *
     * @param tgAccountUpdateRequest
     * @return
     */
    @PostMapping("/update")
    @AuthCheck(mustRole = UserConstant.ADMIN_ROLE)
    public BaseResponse<Boolean> updateTgAccount(@RequestBody TgAccountUpdateRequest tgAccountUpdateRequest) {
        if (tgAccountUpdateRequest == null || tgAccountUpdateRequest.getId() <= 0) {
            throw new BusinessException(ErrorCode.PARAMS_ERROR);
        }
        // todo 在此处将实体类和 DTO 进行转换
        TgAccount tgAccount = new TgAccount();
        BeanUtils.copyProperties(tgAccountUpdateRequest, tgAccount);
        // 数据校验
        tgAccountService.validTgAccount(tgAccount, false);
        // 判断是否存在
        long id = tgAccountUpdateRequest.getId();
        TgAccount oldTgAccount = tgAccountService.getById(id);
        ThrowUtils.throwIf(oldTgAccount == null, ErrorCode.NOT_FOUND_ERROR);
        // 操作数据库
        boolean result = tgAccountService.updateById(tgAccount);
        ThrowUtils.throwIf(!result, ErrorCode.OPERATION_ERROR);
        return ResultUtils.success(true);
    }

    /**
     * 根据 id 获取TG账号（封装类）
     *
     * @param id
     * @return
     */
    @GetMapping("/get/vo")
    @AuthCheck(mustRole = UserConstant.ADMIN_ROLE)
    public BaseResponse<TgAccountVO> getTgAccountVOById(long id, HttpServletRequest request) {
        ThrowUtils.throwIf(id <= 0, ErrorCode.PARAMS_ERROR);
        // 查询数据库
        TgAccount tgAccount = tgAccountService.getById(id);
        ThrowUtils.throwIf(tgAccount == null, ErrorCode.NOT_FOUND_ERROR);
        // 获取封装类
        return ResultUtils.success(tgAccountService.getTgAccountVO(tgAccount, request));
    }

    /**
     * 分页获取TG账号列表（仅管理员可用）
     *
     * @param tgAccountQueryRequest
     * @return
     */
    @PostMapping("/list/page")
    @AuthCheck(mustRole = UserConstant.ADMIN_ROLE)
    public BaseResponse<Page<TgAccount>> listTgAccountByPage(@RequestBody TgAccountQueryRequest tgAccountQueryRequest) {
        long current = tgAccountQueryRequest.getCurrent();
        long size = tgAccountQueryRequest.getPageSize();
        // 查询数据库
        Page<TgAccount> tgAccountPage = tgAccountService.page(new Page<>(current, size),
                tgAccountService.getQueryWrapper(tgAccountQueryRequest));
        return ResultUtils.success(tgAccountPage);
    }

    /**
     * 分页获取TG账号列表（封装类）
     *
     * @param tgAccountQueryRequest
     * @param request
     * @return
     */
    @PostMapping("/list/page/vo")
    public BaseResponse<Page<TgAccountVO>> listTgAccountVOByPage(@RequestBody TgAccountQueryRequest tgAccountQueryRequest,
                                                               HttpServletRequest request) {
        long current = tgAccountQueryRequest.getCurrent();
        long size = tgAccountQueryRequest.getPageSize();
        // 限制爬虫
        ThrowUtils.throwIf(size > 20, ErrorCode.PARAMS_ERROR);
        // 查询数据库
        Page<TgAccount> tgAccountPage = tgAccountService.page(new Page<>(current, size),
                tgAccountService.getQueryWrapper(tgAccountQueryRequest));
        // 获取封装类
        return ResultUtils.success(tgAccountService.getTgAccountVOPage(tgAccountPage, request));
    }

    /**
     * 编辑TG账号（给用户使用）
     *
     * @param tgAccountEditRequest
     * @param request
     * @return
     */
    @PostMapping("/edit")
    @AuthCheck(mustRole = UserConstant.ADMIN_ROLE)
    public BaseResponse<Boolean> editTgAccount(@RequestBody TgAccountEditRequest tgAccountEditRequest, HttpServletRequest request) {
        if (tgAccountEditRequest == null || tgAccountEditRequest.getId() <= 0) {
            throw new BusinessException(ErrorCode.PARAMS_ERROR);
        }
        // todo 在此处将实体类和 DTO 进行转换
        TgAccount tgAccount = new TgAccount();
        BeanUtils.copyProperties(tgAccountEditRequest, tgAccount);
        // 数据校验
        tgAccountService.validTgAccount(tgAccount, false);
        // 判断是否存在
        long id = tgAccountEditRequest.getId();
        TgAccount oldTgAccount = tgAccountService.getById(id);
        ThrowUtils.throwIf(oldTgAccount == null, ErrorCode.NOT_FOUND_ERROR);
        // 操作数据库
        boolean result = tgAccountService.updateById(tgAccount);
        ThrowUtils.throwIf(!result, ErrorCode.OPERATION_ERROR);
        return ResultUtils.success(true);
    }

    // endregion
}
