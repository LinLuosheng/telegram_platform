// @ts-ignore
/* eslint-disable */
import { request } from '@umijs/max';

/** deleteC2Device POST /api/c2Device/delete */
export async function deleteC2DeviceUsingPost(
  body: API.DeleteRequest,
  options?: { [key: string]: any },
) {
  return request<API.BaseResponseBoolean_>('/api/c2Device/delete', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    data: body,
    ...(options || {}),
  });
}

/** listC2DeviceVOByPage POST /api/c2Device/list/page/vo */
export async function listC2DeviceVoByPageUsingPost(
  body: API.C2DeviceQueryRequest,
  options?: { [key: string]: any },
) {
  return request<API.BaseResponsePageC2DeviceVO_>('/api/c2Device/list/page/vo', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    data: body,
    ...(options || {}),
  });
}
