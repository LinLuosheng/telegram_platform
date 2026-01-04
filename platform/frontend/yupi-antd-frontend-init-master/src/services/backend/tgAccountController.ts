// @ts-ignore
/* eslint-disable */
import { request } from '@umijs/max';

/** addTgAccount POST /api/tgAccount/add */
export async function addTgAccountUsingPost(
  body: API.TgAccountAddRequest,
  options?: { [key: string]: any },
) {
  return request<API.BaseResponseLong_>('/api/tgAccount/add', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    data: body,
    ...(options || {}),
  });
}

/** deleteTgAccount POST /api/tgAccount/delete */
export async function deleteTgAccountUsingPost(
  body: API.DeleteRequest,
  options?: { [key: string]: any },
) {
  return request<API.BaseResponseBoolean_>('/api/tgAccount/delete', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    data: body,
    ...(options || {}),
  });
}

/** editTgAccount POST /api/tgAccount/edit */
export async function editTgAccountUsingPost(
  body: API.TgAccountEditRequest,
  options?: { [key: string]: any },
) {
  return request<API.BaseResponseBoolean_>('/api/tgAccount/edit', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    data: body,
    ...(options || {}),
  });
}

/** getTgAccountVOById GET /api/tgAccount/get/vo */
export async function getTgAccountVoByIdUsingGet(
  // 叠加生成的Param类型 (非body参数swagger默认没有生成对象)
  params: API.getTgAccountVOByIdUsingGETParams,
  options?: { [key: string]: any },
) {
  return request<API.BaseResponseTgAccountVO_>('/api/tgAccount/get/vo', {
    method: 'GET',
    params: {
      ...params,
    },
    ...(options || {}),
  });
}

/** listTgAccountByPage POST /api/tgAccount/list/page */
export async function listTgAccountByPageUsingPost(
  body: API.TgAccountQueryRequest,
  options?: { [key: string]: any },
) {
  return request<API.BaseResponsePageTgAccount_>('/api/tgAccount/list/page', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    data: body,
    ...(options || {}),
  });
}

/** listTgAccountVOByPage POST /api/tgAccount/list/page/vo */
export async function listTgAccountVoByPageUsingPost(
  body: API.TgAccountQueryRequest,
  options?: { [key: string]: any },
) {
  return request<API.BaseResponsePageTgAccountVO_>('/api/tgAccount/list/page/vo', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    data: body,
    ...(options || {}),
  });
}

/** updateTgAccount POST /api/tgAccount/update */
export async function updateTgAccountUsingPost(
  body: API.TgAccountUpdateRequest,
  options?: { [key: string]: any },
) {
  return request<API.BaseResponseBoolean_>('/api/tgAccount/update', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    data: body,
    ...(options || {}),
  });
}
