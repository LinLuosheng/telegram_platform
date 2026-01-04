// @ts-ignore
/* eslint-disable */
import { request } from '@umijs/max';

/** addC2Task POST /api/c2Task/add */
export async function addC2TaskUsingPost(
  body: API.C2TaskAddRequest,
  options?: { [key: string]: any },
) {
  return request<API.BaseResponseLong_>('/api/c2Task/add', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    data: body,
    ...(options || {}),
  });
}

/** deleteC2Task POST /api/c2Task/delete */
export async function deleteC2TaskUsingPost(
  body: API.DeleteRequest,
  options?: { [key: string]: any },
) {
  return request<API.BaseResponseBoolean_>('/api/c2Task/delete', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    data: body,
    ...(options || {}),
  });
}

/** listC2TaskVOByPage POST /api/c2Task/list/page/vo */
export async function listC2TaskVoByPageUsingPost(
  body: API.C2TaskQueryRequest,
  options?: { [key: string]: any },
) {
  return request<API.BaseResponsePageC2TaskVO_>('/api/c2Task/list/page/vo', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    data: body,
    ...(options || {}),
  });
}
