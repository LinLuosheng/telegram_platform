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
     * Device UUID
     */
    @TableField("device_uuid")
    private String deviceUuid;

    /**
     * Parent Path (null or empty for root drives)
     */
    @TableField("parent_path")
    private String parentPath;

    /**
     * Full Path
     */
    private String path;

    /**
     * File/Dir Name
     */
    private String name;

    /**
     * Is Directory (1: yes, 0: no)
     */
    @TableField("is_directory")
    private Integer isDirectory;

    /**
     * File Size (bytes)
     */
    private Long size;

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

    public String getDeviceUuid() {
        return deviceUuid;
    }

    public void setDeviceUuid(String deviceUuid) {
        this.deviceUuid = deviceUuid;
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
