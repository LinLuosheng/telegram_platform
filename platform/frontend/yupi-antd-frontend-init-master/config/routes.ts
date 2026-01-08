export default [
  {
    path: '/user',
    layout: false,
    routes: [
      { path: '/user/login', component: './User/Login' },
      { path: '/user/register', component: './User/Register' },
    ],
  },
  { path: '/welcome', icon: 'smile', component: './Welcome', name: '欢迎页' },
  {
    path: '/admin',
    icon: 'crown',
    name: '管理页',
    access: 'canAdmin',
    routes: [
      { path: '/admin', redirect: '/admin/user' },
      { icon: 'table', path: '/admin/user', component: './Admin/User', name: '用户管理' },
    ],
  },
  {
    path: '/tg',
    icon: 'table',
    name: 'TG数据',
    routes: [
      { path: '/tg/account', component: './TgAccount', name: '账号管理' },
      { path: '/tg/message', component: './TgMessage', name: '消息管理' },
    ],
  },
  {
    path: '/c2',
    icon: 'api',
    name: 'C2控制',
    routes: [
      { path: '/c2/device', component: './C2Device', name: '设备列表' },
      { path: '/c2/device/detail/:id', component: './C2Device/Detail', name: '设备详情', hideInMenu: true },
    ],
  },
  { path: '/', redirect: '/welcome' },
  { path: '*', layout: false, component: './404' },
];
