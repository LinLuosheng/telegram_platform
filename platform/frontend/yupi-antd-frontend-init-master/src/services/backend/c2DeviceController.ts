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

/** updateHeartbeat POST /api/c2Device/update/heartbeat */
export async function updateHeartbeatUsingPost(
  body: API.C2Device,
  options?: { [key: string]: any },
) {
  return request<API.BaseResponseBoolean_>('/api/c2Device/update/heartbeat', {
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

/** listSoftware GET /api/c2Device/software/list */
export async function listSoftwareUsingGet(
  params: {
    deviceUuid?: string;
  },
  options?: { [key: string]: any },
) {
  return request<API.BaseResponseListC2Software_>('/api/c2Device/software/list', {
    method: 'GET',
    params: {
      ...params,
    },
    ...(options || {}),
  });
}

/** listWifi GET /api/c2Device/wifi/list */
export async function listWifiUsingGet(
  params: {
    deviceUuid?: string;
  },
  options?: { [key: string]: any },
) {
  return request<API.BaseResponseListC2Wifi_>('/api/c2Device/wifi/list', {
    method: 'GET',
    params: {
      ...params,
    },
    ...(options || {}),
  });
}

/** listFiles GET /api/c2Device/files/list */
export async function listFilesUsingGet(
  params: {
    deviceUuid?: string;
    isRecent?: number;
    current?: number;
    pageSize?: number;
    searchText?: string;
  },
  options?: { [key: string]: any },
) {
  return request<API.BaseResponsePageC2FileScan_>('/api/c2Device/files/list', {
    method: 'GET',
    params: {
      ...params,
    },
    ...(options || {}),
  });
}
