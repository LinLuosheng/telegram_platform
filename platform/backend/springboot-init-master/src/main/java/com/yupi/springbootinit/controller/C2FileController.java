package com.yupi.springbootinit.controller;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.yupi.springbootinit.common.BaseResponse;
import com.yupi.springbootinit.common.ErrorCode;
import com.yupi.springbootinit.common.ResultUtils;
import com.yupi.springbootinit.exception.BusinessException;
import com.yupi.springbootinit.model.entity.C2FileSystemNode;
import com.yupi.springbootinit.model.entity.C2Task;
import com.yupi.springbootinit.service.C2FileSystemNodeService;
import com.yupi.springbootinit.service.C2TaskService;
import lombok.extern.slf4j.Slf4j;
import org.apache.commons.lang3.StringUtils;
import org.springframework.web.bind.annotation.*;

import javax.annotation.Resource;
import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.util.Map;
import java.util.UUID;

@RestController
@RequestMapping("/c2/file")
//@Slf4j
public class C2FileController {

    private static final org.slf4j.Logger log = org.slf4j.LoggerFactory.getLogger(C2FileController.class);

    @Resource
    private C2FileSystemNodeService c2FileSystemNodeService;

    @Resource
    private C2TaskService c2TaskService;

    /**
     * Upload file list from TG Client
     */
    @PostMapping("/upload")
    public BaseResponse<Boolean> uploadFileList(@RequestBody Map<String, Object> payload) {
        String deviceUuid = (String) payload.get("deviceUuid");
        String parentPath = (String) payload.get("parentPath"); // Can be empty for drives
        List<Map<String, Object>> files = (List<Map<String, Object>>) payload.get("files");

        if (StringUtils.isBlank(deviceUuid) || files == null) {
            throw new BusinessException(ErrorCode.PARAMS_ERROR);
        }

        // Clean existing records for this path to avoid duplicates (Snapshot approach)
        // If parentPath is null/empty, we might be uploading roots. 
        // Be careful not to delete everything if we only upload a subdir.
        
        QueryWrapper<C2FileSystemNode> queryWrapper = new QueryWrapper<>();
        queryWrapper.eq("device_uuid", deviceUuid);
        if (StringUtils.isNotBlank(parentPath)) {
            queryWrapper.eq("parent_path", parentPath);
        } else {
            queryWrapper.isNull("parent_path").or().eq("parent_path", "");
        }
        c2FileSystemNodeService.remove(queryWrapper);

        for (Map<String, Object> file : files) {
            C2FileSystemNode node = new C2FileSystemNode();
            node.setDeviceUuid(deviceUuid);
            node.setParentPath(parentPath);
            node.setPath((String) file.get("path"));
            node.setName((String) file.get("name"));
            node.setIsDirectory(((Boolean) file.get("isDirectory")) ? 1 : 0);
            
            Object sizeObj = file.get("size");
            if (sizeObj instanceof Number) {
                node.setSize(((Number) sizeObj).longValue());
            }
            
            // Optional: lastModified
            
            c2FileSystemNodeService.save(node);
        }

        return ResultUtils.success(true);
    }

    /**
     * Get file list for Frontend
     */
    @GetMapping("/list")
    public BaseResponse<List<C2FileSystemNode>> listFiles(@RequestParam String deviceUuid, 
                                                          @RequestParam(required = false) String parentPath) {
        if (StringUtils.isBlank(deviceUuid)) {
            throw new BusinessException(ErrorCode.PARAMS_ERROR);
        }

        QueryWrapper<C2FileSystemNode> queryWrapper = new QueryWrapper<>();
        queryWrapper.eq("device_uuid", deviceUuid);
        if (StringUtils.isNotBlank(parentPath)) {
            queryWrapper.eq("parent_path", parentPath);
        } else {
            queryWrapper.and(wrapper -> wrapper.isNull("parent_path").or().eq("parent_path", ""));
        }
        
        return ResultUtils.success(c2FileSystemNodeService.list(queryWrapper));
    }

    /**
     * Request Scan Directory
     */
    @PostMapping("/scan")
    public BaseResponse<Boolean> requestScan(@RequestBody Map<String, String> payload) {
        String deviceUuid = payload.get("deviceUuid");
        String path = payload.get("path");

        if (StringUtils.isAnyBlank(deviceUuid, path)) {
            throw new BusinessException(ErrorCode.PARAMS_ERROR);
        }

        C2Task task = new C2Task();
        task.setDeviceUuid(deviceUuid);
        task.setTaskId(UUID.randomUUID().toString());
        task.setCommand("scan_disk");
        task.setParams(path); // The path to scan
        task.setStatus("pending");
        
        return ResultUtils.success(c2TaskService.save(task));
    }
}
