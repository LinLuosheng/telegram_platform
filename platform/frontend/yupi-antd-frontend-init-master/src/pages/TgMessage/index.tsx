import { PlusOutlined } from '@ant-design/icons';
import type { ActionType, ProColumns } from '@ant-design/pro-components';
import { PageContainer, ProTable } from '@ant-design/pro-components';
import '@umijs/max';
import { Button, message, Space, Typography } from 'antd';
import React, { useRef, useState } from 'react';
import { listTgMessageVoByPageUsingPost, deleteTgMessageUsingPost } from '@/services/backend/tgMessageController';

const TgMessagePage: React.FC = () => {
  const actionRef = useRef<ActionType>();
  
  const handleDelete = async (row: API.TgMessageVO) => {
    const hide = message.loading('正在删除');
    if (!row) return true;
    try {
      await deleteTgMessageUsingPost({
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

  const columns: ProColumns<API.TgMessageVO>[] = [
    {
      title: 'id',
      dataIndex: 'id',
      valueType: 'text',
      hideInForm: true,
      hideInSearch: true,
    },
    {
      title: '账号ID',
      dataIndex: 'accountId',
      valueType: 'text',
    },
    {
      title: '消息ID',
      dataIndex: 'msgId',
      valueType: 'text',
      hideInSearch: true,
    },
    {
      title: 'Chat ID',
      dataIndex: 'chatId',
      valueType: 'text',
    },
    {
      title: '发送者ID',
      dataIndex: 'senderId',
      valueType: 'text',
    },
    {
      title: '内容',
      dataIndex: 'content',
      valueType: 'textarea',
      ellipsis: true,
    },
    {
      title: '类型',
      dataIndex: 'msgType',
      valueType: 'text',
    },
    {
      title: '媒体路径',
      dataIndex: 'mediaPath',
      valueType: 'text',
      hideInSearch: true,
    },
    {
      title: '消息时间',
      dataIndex: 'msgDate',
      valueType: 'dateTime',
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
      <ProTable<API.TgMessageVO>
        headerTitle={'TG消息列表'}
        actionRef={actionRef}
        rowKey="id"
        search={{
          labelWidth: 120,
        }}
        request={async (params, sort, filter) => {
          const sortField = Object.keys(sort)?.[0];
          const sortOrder = sort?.[sortField] ?? undefined;

          const { data, code } = await listTgMessageVoByPageUsingPost({
            ...params,
            sortField,
            sortOrder,
            ...filter,
          } as API.TgMessageQueryRequest);

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

export default TgMessagePage;
