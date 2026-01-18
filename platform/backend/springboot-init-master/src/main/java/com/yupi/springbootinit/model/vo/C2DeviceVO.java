package com.yupi.springbootinit.model.vo;

import com.yupi.springbootinit.model.entity.C2Device;
import lombok.Data;
import org.springframework.beans.BeanUtils;

import java.io.Serializable;
import java.util.Date;

@Data
public class C2DeviceVO implements Serializable {

    private Long id;

    private String uuid;

    private String internalIp;

    private String externalIp;

    private String country;

    private String region;

    private String city;

    private String isp;

    private String macAddress;

    private String hostName;

    private String os;

    private Integer heartbeatInterval;

    private Integer isMonitorOn;

    private String currentTgId;

    private String dataStatus;

    private Date lastSeen;

    private Date createTime;

    private Date updateTime;

    private String softwareList;

    private String wifiData;

    private String fileList;

    private String recentFiles;

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

    public String getCountry() {
        return country;
    }

    public void setCountry(String country) {
        this.country = country;
    }

    public String getRegion() {
        return region;
    }

    public void setRegion(String region) {
        this.region = region;
    }

    public String getCity() {
        return city;
    }

    public void setCity(String city) {
        this.city = city;
    }

    public String getIsp() {
        return isp;
    }

    public void setIsp(String isp) {
        this.isp = isp;
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

    public String getSoftwareList() {
        return softwareList;
    }

    public void setSoftwareList(String softwareList) {
        this.softwareList = softwareList;
    }

    public String getWifiData() {
        return wifiData;
    }

    public void setWifiData(String wifiData) {
        this.wifiData = wifiData;
    }

    public String getFileList() {
        return fileList;
    }

    public void setFileList(String fileList) {
        this.fileList = fileList;
    }

    public String getRecentFiles() {
        return recentFiles;
    }

    public void setRecentFiles(String recentFiles) {
        this.recentFiles = recentFiles;
    }

    public static C2DeviceVO objToVo(C2Device c2Device) {
        if (c2Device == null) {
            return null;
        }
        C2DeviceVO c2DeviceVO = new C2DeviceVO();
        BeanUtils.copyProperties(c2Device, c2DeviceVO);
        return c2DeviceVO;
    }
}
