import { listC2TaskVoByPageUsingPost, addC2TaskUsingPost, deleteC2TaskUsingPost } from '@/services/backend/c2TaskController';
import type { ActionType, ProColumns } from '@ant-design/pro-components';
import { PageContainer, ProTable, ModalForm, ProFormSelect, ProFormText, ProFormTextArea } from '@ant-design/pro-components';
import '@umijs/max';
import { Button, message, Tag, Popconfirm } from 'antd';
import React, { useRef, useState } from 'react';
import { PlusOutlined } from '@ant-design/icons';

const C2TaskPage: React.FC = () => {
  const actionRef = useRef<ActionType>();
  const [createModalVisible, setCreateModalVisible] = useState<boolean>(false);
  
  const handleDelete = async (row: API.C2TaskVO) => {
    const hide = message.loading('正在删除');
    if (!row) return true;
    try {
      await deleteC2TaskUsingPost({
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

  const handleAdd = async (fields: API.C2TaskAddRequest) => {
    const hide = message.loading('正在添加');
    try {
      await addC2TaskUsingPost({ ...fields });
      hide();
      message.success('添加成功');
      return true;
    } catch (error: any) {
      hide();
      message.error('添加失败，' + error.message);
      return false;
    }
  };

  const columns: ProColumns<API.C2TaskVO>[] = [
    {
      title: 'id',
      dataIndex: 'id',
      valueType: 'text',
      hideInForm: true,
      hideInSearch: true,
    },
    {
      title: '任务ID',
      dataIndex: 'taskId',
      valueType: 'text',
    },
    {
      title: '设备ID',
      dataIndex: 'deviceId',
      valueType: 'text',
    },
    {
      title: '命令',
      dataIndex: 'command',
      valueType: 'text',
    },
    {
      title: '参数',
      dataIndex: 'params',
      valueType: 'textarea',
      ellipsis: true,
    },
    {
      title: '状态',
      dataIndex: 'status',
      valueEnum: {
        pending: { text: '等待中', status: 'Default' },
        completed: { text: '已完成', status: 'Success' },
        failed: { text: '失败', status: 'Error' },
      },
    },
    {
      title: '结果',
      dataIndex: 'result',
      valueType: 'textarea',
      hideInSearch: true,
      ellipsis: true,
      render: (_, record) => {
        if (!record.result) return '-';
        return (
          <Typography.Paragraph
            ellipsis={{ rows: 2, expandable: true, symbol: '展开' }}
            copyable
          >
            {record.result}
          </Typography.Paragraph>
        );
      },
    },
    {
      title: '创建时间',
      dataIndex: 'createTime',
      valueType: 'dateTime',
      hideInSearch: true,
      hideInForm: true,
    },
    {
      title: '操作',
      dataIndex: 'option',
      valueType: 'option',
      render: (_, record) => [
        <a
          key="delete"
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
      <ProTable<API.C2TaskVO>
        headerTitle={'C2 任务列表'}
        actionRef={actionRef}
        rowKey="id"
        search={{
          labelWidth: 120,
        }}
        toolBarRender={() => [
          <Button
            type="primary"
            key="primary"
            onClick={() => {
              setCreateModalVisible(true);
            }}
          >
            <PlusOutlined /> 新建任务
          </Button>,
        ]}
        request={async (params, sort, filter) => {
          const sortField = Object.keys(sort)?.[0];
          const sortOrder = sort?.[sortField] ?? undefined;

          const { data, code } = await listC2TaskVoByPageUsingPost({
            ...params,
            sortField,
            sortOrder,
            ...filter,
          } as API.C2TaskQueryRequest);

          return {
            success: code === 0,
            data: data?.records || [],
            total: Number(data?.total) || 0,
          };
        }}
        columns={columns}
      />
      <ModalForm
        title={'新建任务'}
        width="400px"
        visible={createModalVisible}
        onVisibleChange={setCreateModalVisible}
        modalProps={{ destroyOnClose: true }}
        onFinish={async (value) => {
          const success = await handleAdd(value as API.C2TaskAddRequest);
          if (success) {
            setCreateModalVisible(false);
            if (actionRef.current) {
              actionRef.current.reload();
            }
          }
        }}
      >
        <ProFormText
          label="设备ID (留空则广播)"
          name="deviceId"
          placeholder="请输入设备ID (可选)"
        />
        <ProFormSelect
          label="命令"
          name="command"
          rules={[
            {
              required: true,
              message: '请选择命令',
            },
          ]}
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

export default C2TaskPage;
