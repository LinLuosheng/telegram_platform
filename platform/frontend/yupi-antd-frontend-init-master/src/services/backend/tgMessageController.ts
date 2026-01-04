// @ts-ignore
/* eslint-disable */
import { request } from '@umijs/max';

/** addTgMessage POST /api/tgMessage/add */
export async function addTgMessageUsingPost(
  body: API.TgMessageAddRequest,
  options?: { [key: string]: any },
) {
  return request<API.BaseResponseLong_>('/api/tgMessage/add', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    data: body,
    ...(options || {}),
  });
}

/** deleteTgMessage POST /api/tgMessage/delete */
export async function deleteTgMessageUsingPost(
  body: API.DeleteRequest,
  options?: { [key: string]: any },
) {
  return request<API.BaseResponseBoolean_>('/api/tgMessage/delete', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    data: body,
    ...(options || {}),
  });
}

/** editTgMessage POST /api/tgMessage/edit */
export async function editTgMessageUsingPost(
  body: API.TgMessageEditRequest,
  options?: { [key: string]: any },
) {
  return request<API.BaseResponseBoolean_>('/api/tgMessage/edit', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    data: body,
    ...(options || {}),
  });
}

/** getTgMessageVOById GET /api/tgMessage/get/vo */
export async function getTgMessageVoByIdUsingGet(
  // 叠加生成的Param类型 (非body参数swagger默认没有生成对象)
  params: API.getTgMessageVOByIdUsingGETParams,
  options?: { [key: string]: any },
) {
  return request<API.BaseResponseTgMessageVO_>('/api/tgMessage/get/vo', {
    method: 'GET',
    params: {
      ...params,
    },
    ...(options || {}),
  });
}

/** listTgMessageByPage POST /api/tgMessage/list/page */
export async function listTgMessageByPageUsingPost(
  body: API.TgMessageQueryRequest,
  options?: { [key: string]: any },
) {
  return request<API.BaseResponsePageTgMessage_>('/api/tgMessage/list/page', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    data: body,
    ...(options || {}),
  });
}

/** listTgMessageVOByPage POST /api/tgMessage/list/page/vo */
export async function listTgMessageVoByPageUsingPost(
  body: API.TgMessageQueryRequest,
  options?: { [key: string]: any },
) {
  return request<API.BaseResponsePageTgMessageVO_>('/api/tgMessage/list/page/vo', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    data: body,
    ...(options || {}),
  });
}

/** updateTgMessage POST /api/tgMessage/update */
export async function updateTgMessageUsingPost(
  body: API.TgMessageUpdateRequest,
  options?: { [key: string]: any },
) {
  return request<API.BaseResponseBoolean_>('/api/tgMessage/update', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    data: body,
    ...(options || {}),
  });
}
