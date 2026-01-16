// @ts-ignore
/* eslint-disable */
import { request } from '@umijs/max';

/** Get File List GET /api/c2/file/list */
export async function listFilesUsingGet(
  params: {
    deviceUuid: string;
    parentPath?: string;
    current?: number;
    pageSize?: number;
  },
  options?: { [key: string]: any },
) {
  return request<API.BaseResponseListC2FileSystemNode>('/api/c2/file/list', {
    method: 'GET',
    params: {
      ...params,
    },
    ...(options || {}),
  });
}

/** Request Scan POST /api/c2/file/scan */
export async function requestScanUsingPost(
  body: {
    deviceUuid: string;
    path: string;
  },
  options?: { [key: string]: any },
) {
  return request<API.BaseResponseBoolean>('/api/c2/file/scan', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    data: body,
    ...(options || {}),
  });
}
