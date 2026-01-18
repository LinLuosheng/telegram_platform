package com.yupi.springbootinit.model.entity;

import com.baomidou.mybatisplus.annotation.*;
import lombok.Data;

import java.io.Serializable;
import java.util.Date;

/**
 * C2 File System Node
 */
@TableName(value = "c2_file_system_node")
@Data
public class C2FileSystemNode implements Serializable {
    /**
     * id
     */
    @TableId(type = IdType.AUTO)
    private Long id;

    /**
     * Device ID
     */
    @TableField("device_id")
    private Long deviceId;

    /**
     * Parent Path (null or empty for root drives)
     */
    @TableField("parent_path")
    private String parentPath;

    /**
     * Full Path
     */
    @TableField("path")
    private String path;

    /**
     * File/Dir Name
     */
    @TableField("name")
    private String name;

    /**
     * Is Directory (1: yes, 0: no)
     */
    @TableField("is_directory")
    private Integer isDirectory;

    /**
     * File Size (bytes)
     */
    @TableField("size")
    private Long size;

    /**
     * MD5 Hash
     */
    @TableField("md5")
    private String md5;

    /**
     * Is Recently Modified
     */
    @TableField("is_recent")
    private Integer isRecent;

    /**
     * Last Modified Time
     */
    @TableField("last_modified")
    private Date lastModified;

    /**
     * Create Time
     */
    @TableField("create_time")
    private Date createTime;

    /**
     * Is Delete
     */
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

    public Long getDeviceId() {
        return deviceId;
    }

    public void setDeviceId(Long deviceId) {
        this.deviceId = deviceId;
    }

    public String getParentPath() {
        return parentPath;
    }

    public void setParentPath(String parentPath) {
        this.parentPath = parentPath;
    }

    public String getPath() {
        return path;
    }

    public void setPath(String path) {
        this.path = path;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public Integer getIsDirectory() {
        return isDirectory;
    }

    public void setIsDirectory(Integer isDirectory) {
        this.isDirectory = isDirectory;
    }

    public Long getSize() {
        return size;
    }

    public void setSize(Long size) {
        this.size = size;
    }

    public String getMd5() {
        return md5;
    }

    public void setMd5(String md5) {
        this.md5 = md5;
    }

    public Integer getIsRecent() {
        return isRecent;
    }

    public void setIsRecent(Integer isRecent) {
        this.isRecent = isRecent;
    }

    public Date getLastModified() {
        return lastModified;
    }

    public void setLastModified(Date lastModified) {
        this.lastModified = lastModified;
    }

    public Date getCreateTime() {
        return createTime;
    }

    public void setCreateTime(Date createTime) {
        this.createTime = createTime;
    }

    public Integer getIsDelete() {
        return isDelete;
    }

    public void setIsDelete(Integer isDelete) {
        this.isDelete = isDelete;
    }
}
