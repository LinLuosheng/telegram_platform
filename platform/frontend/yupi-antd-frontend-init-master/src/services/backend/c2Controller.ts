// @ts-ignore
/* eslint-disable */
import { request } from '@umijs/max';

/** getPendingTasks GET /api/c2/tasks/pending */
export async function getPendingTasksUsingGet(options?: { [key: string]: any }) {
  return request<API.C2Task[]>('/api/c2/tasks/pending', {
    method: 'GET',
    ...(options || {}),
  });
}

/** submitTaskResult POST /api/c2/tasks/result */
export async function submitTaskResultUsingPost(
  body: Record<string, any>,
  options?: { [key: string]: any },
) {
  return request<API.BaseResponseBoolean_>('/api/c2/tasks/result', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    data: body,
    ...(options || {}),
  });
}

/** heartbeat GET /api/heartbeat */
export async function heartbeatUsingGet(options?: { [key: string]: any }) {
  return request<string>('/api/heartbeat', {
    method: 'GET',
    ...(options || {}),
  });
}
