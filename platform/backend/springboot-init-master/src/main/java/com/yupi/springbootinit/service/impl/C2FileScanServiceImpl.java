package com.yupi.springbootinit.service.impl;

import com.baomidou.mybatisplus.extension.service.impl.ServiceImpl;
import com.yupi.springbootinit.mapper.C2FileScanMapper;
import com.yupi.springbootinit.model.entity.C2FileScan;
import com.yupi.springbootinit.service.C2FileScanService;
import org.springframework.stereotype.Service;

/**
 * C2 File Scan Service Implementation
 */
@Service
public class C2FileScanServiceImpl extends ServiceImpl<C2FileScanMapper, C2FileScan> implements C2FileScanService {
}
