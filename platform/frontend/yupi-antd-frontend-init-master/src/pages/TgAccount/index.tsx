import { PlusOutlined } from '@ant-design/icons';
import type { ActionType, ProColumns } from '@ant-design/pro-components';
import { PageContainer, ProTable } from '@ant-design/pro-components';
import '@umijs/max';
import { Button, message, Space, Typography } from 'antd';
import React, { useRef, useState } from 'react';
import { listTgAccountVoByPageUsingPost, deleteTgAccountUsingPost } from '@/services/backend/tgAccountController';

const TgAccountPage: React.FC = () => {
  const actionRef = useRef<ActionType>();
  
  const handleDelete = async (row: API.TgAccountVO) => {
    const hide = message.loading('正在删除');
    if (!row) return true;
    try {
      await deleteTgAccountUsingPost({
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

  const columns: ProColumns<API.TgAccountVO>[] = [
    {
      title: 'id',
      dataIndex: 'id',
      valueType: 'text',
      hideInForm: true,
      hideInSearch: true,
    },
    {
      title: 'TG ID',
      dataIndex: 'tgId',
      valueType: 'text',
    },
    {
      title: '用户名',
      dataIndex: 'username',
      valueType: 'text',
    },
    {
      title: '手机号',
      dataIndex: 'phone',
      valueType: 'text',
    },
    {
      title: '名字',
      dataIndex: 'firstName',
      valueType: 'text',
      hideInSearch: true,
    },
    {
      title: '姓氏',
      dataIndex: 'lastName',
      valueType: 'text',
      hideInSearch: true,
    },
    {
      title: '是否机器人',
      dataIndex: 'isBot',
      valueType: 'select',
      valueEnum: {
        0: { text: '否', status: 'Default' },
        1: { text: '是', status: 'Success' },
      },
    },
    {
      title: '系统信息',
      dataIndex: 'systemInfo',
      valueType: 'textarea',
      hideInSearch: true,
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
      <ProTable<API.TgAccountVO>
        headerTitle={'TG账号列表'}
        actionRef={actionRef}
        rowKey="id"
        search={{
          labelWidth: 120,
        }}
        request={async (params, sort, filter) => {
          const sortField = Object.keys(sort)?.[0];
          const sortOrder = sort?.[sortField] ?? undefined;

          const { data, code } = await listTgAccountVoByPageUsingPost({
            ...params,
            sortField,
            sortOrder,
            ...filter,
          } as API.TgAccountQueryRequest);

          return {
            success: code === 0,
            data: data?.records || [],
            total: Number(data?.total) || 0,
          };
        }}
        columns={columns}
      />
    </PageContainer>
  );
};

export default TgAccountPage;
