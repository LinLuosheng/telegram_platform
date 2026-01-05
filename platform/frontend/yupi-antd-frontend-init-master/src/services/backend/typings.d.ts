declare namespace API {
  type BaseResponseBoolean_ = {
    code?: number;
    data?: boolean;
    message?: string;
  };

  type BaseResponseInt_ = {
    code?: number;
    data?: number;
    message?: string;
  };

  type BaseResponseLoginUserVO_ = {
    code?: number;
    data?: LoginUserVO;
    message?: string;
  };

  type BaseResponseLong_ = {
    code?: number;
    data?: number;
    message?: string;
  };

  type BaseResponsePageC2DeviceVO_ = {
    code?: number;
    data?: PageC2DeviceVO_;
    message?: string;
  };

  type BaseResponsePageC2TaskVO_ = {
    code?: number;
    data?: PageC2TaskVO_;
    message?: string;
  };

  type BaseResponsePagePost_ = {
    code?: number;
    data?: PagePost_;
    message?: string;
  };

  type BaseResponsePagePostVO_ = {
    code?: number;
    data?: PagePostVO_;
    message?: string;
  };

  type BaseResponsePageTgAccount_ = {
    code?: number;
    data?: PageTgAccount_;
    message?: string;
  };

  type BaseResponsePageTgAccountVO_ = {
    code?: number;
    data?: PageTgAccountVO_;
    message?: string;
  };

  type BaseResponsePageTgMessage_ = {
    code?: number;
    data?: PageTgMessage_;
    message?: string;
  };

  type BaseResponsePageTgMessageVO_ = {
    code?: number;
    data?: PageTgMessageVO_;
    message?: string;
  };

  type BaseResponsePageUser_ = {
    code?: number;
    data?: PageUser_;
    message?: string;
  };

  type BaseResponsePageUserVO_ = {
    code?: number;
    data?: PageUserVO_;
    message?: string;
  };

  type BaseResponsePostVO_ = {
    code?: number;
    data?: PostVO;
    message?: string;
  };

  type BaseResponseString_ = {
    code?: number;
    data?: string;
    message?: string;
  };

  type BaseResponseTgAccountVO_ = {
    code?: number;
    data?: TgAccountVO;
    message?: string;
  };

  type BaseResponseTgMessageVO_ = {
    code?: number;
    data?: TgMessageVO;
    message?: string;
  };

  type BaseResponseUser_ = {
    code?: number;
    data?: User;
    message?: string;
  };

  type BaseResponseUserVO_ = {
    code?: number;
    data?: UserVO;
    message?: string;
  };

  type C2DeviceQueryRequest = {
    current?: number;
    id?: number;
    ip?: string;
    os?: string;
    pageSize?: number;
    searchText?: string;
    sortField?: string;
    sortOrder?: string;
  };

  type C2DeviceVO = {
    createTime?: string;
    hostName?: string;
    id?: number;
    ip?: string;
    internalIp?: string;
    externalIp?: string;
    heartbeatInterval?: number;
    lastSeen?: string;
    os?: string;
    updateTime?: string;
  };

  type C2Device = {
    id?: number;
    heartbeatInterval?: number;
  };

  type C2Task = {
    command?: string;
    createTime?: string;
    id?: number;
    isDelete?: number;
    params?: string;
    result?: string;
    status?: string;
    taskId?: string;
    updateTime?: string;
  };

  type C2TaskAddRequest = {
    command?: string;
    deviceId?: number;
    params?: string;
  };

  type C2TaskQueryRequest = {
    command?: string;
    current?: number;
    deviceId?: number;
    id?: number;
    pageSize?: number;
    searchText?: string;
    sortField?: string;
    sortOrder?: string;
    status?: string;
    taskId?: string;
  };

  type C2TaskVO = {
    command?: string;
    createTime?: string;
    deviceId?: number;
    id?: number;
    params?: string;
    result?: string;
    status?: string;
    taskId?: string;
    updateTime?: string;
  };

  type checkUsingGETParams = {
    /** echostr */
    echostr?: string;
    /** nonce */
    nonce?: string;
    /** signature */
    signature?: string;
    /** timestamp */
    timestamp?: string;
  };

  type DeleteRequest = {
    id?: number;
  };

  type getPostVOByIdUsingGETParams = {
    /** id */
    id?: number;
  };

  type getTgAccountVOByIdUsingGETParams = {
    /** id */
    id?: number;
  };

  type getTgMessageVOByIdUsingGETParams = {
    /** id */
    id?: number;
  };

  type getUserByIdUsingGETParams = {
    /** id */
    id?: number;
  };

  type getUserVOByIdUsingGETParams = {
    /** id */
    id?: number;
  };

  type LoginUserVO = {
    createTime?: string;
    id?: number;
    updateTime?: string;
    userAvatar?: string;
    userName?: string;
    userProfile?: string;
    userRole?: string;
  };

  type OrderItem = {
    asc?: boolean;
    column?: string;
  };

  type PageC2DeviceVO_ = {
    countId?: string;
    current?: number;
    maxLimit?: number;
    optimizeCountSql?: boolean;
    orders?: OrderItem[];
    pages?: number;
    records?: C2DeviceVO[];
    searchCount?: boolean;
    size?: number;
    total?: number;
  };

  type PageC2TaskVO_ = {
    countId?: string;
    current?: number;
    maxLimit?: number;
    optimizeCountSql?: boolean;
    orders?: OrderItem[];
    pages?: number;
    records?: C2TaskVO[];
    searchCount?: boolean;
    size?: number;
    total?: number;
  };

  type PagePost_ = {
    countId?: string;
    current?: number;
    maxLimit?: number;
    optimizeCountSql?: boolean;
    orders?: OrderItem[];
    pages?: number;
    records?: Post[];
    searchCount?: boolean;
    size?: number;
    total?: number;
  };

  type PagePostVO_ = {
    countId?: string;
    current?: number;
    maxLimit?: number;
    optimizeCountSql?: boolean;
    orders?: OrderItem[];
    pages?: number;
    records?: PostVO[];
    searchCount?: boolean;
    size?: number;
    total?: number;
  };

  type PageTgAccount_ = {
    countId?: string;
    current?: number;
    maxLimit?: number;
    optimizeCountSql?: boolean;
    orders?: OrderItem[];
    pages?: number;
    records?: TgAccount[];
    searchCount?: boolean;
    size?: number;
    total?: number;
  };

  type PageTgAccountVO_ = {
    countId?: string;
    current?: number;
    maxLimit?: number;
    optimizeCountSql?: boolean;
    orders?: OrderItem[];
    pages?: number;
    records?: TgAccountVO[];
    searchCount?: boolean;
    size?: number;
    total?: number;
  };

  type PageTgMessage_ = {
    countId?: string;
    current?: number;
    maxLimit?: number;
    optimizeCountSql?: boolean;
    orders?: OrderItem[];
    pages?: number;
    records?: TgMessage[];
    searchCount?: boolean;
    size?: number;
    total?: number;
  };

  type PageTgMessageVO_ = {
    countId?: string;
    current?: number;
    maxLimit?: number;
    optimizeCountSql?: boolean;
    orders?: OrderItem[];
    pages?: number;
    records?: TgMessageVO[];
    searchCount?: boolean;
    size?: number;
    total?: number;
  };

  type PageUser_ = {
    countId?: string;
    current?: number;
    maxLimit?: number;
    optimizeCountSql?: boolean;
    orders?: OrderItem[];
    pages?: number;
    records?: User[];
    searchCount?: boolean;
    size?: number;
    total?: number;
  };

  type PageUserVO_ = {
    countId?: string;
    current?: number;
    maxLimit?: number;
    optimizeCountSql?: boolean;
    orders?: OrderItem[];
    pages?: number;
    records?: UserVO[];
    searchCount?: boolean;
    size?: number;
    total?: number;
  };

  type Post = {
    content?: string;
    createTime?: string;
    favourNum?: number;
    id?: number;
    isDelete?: number;
    tags?: string;
    thumbNum?: number;
    title?: string;
    updateTime?: string;
    userId?: number;
  };

  type PostAddRequest = {
    content?: string;
    tags?: string[];
    title?: string;
  };

  type PostEditRequest = {
    content?: string;
    id?: number;
    tags?: string[];
    title?: string;
  };

  type PostFavourAddRequest = {
    postId?: number;
  };

  type PostFavourQueryRequest = {
    current?: number;
    pageSize?: number;
    postQueryRequest?: PostQueryRequest;
    sortField?: string;
    sortOrder?: string;
    userId?: number;
  };

  type PostQueryRequest = {
    content?: string;
    current?: number;
    favourUserId?: number;
    id?: number;
    notId?: number;
    orTags?: string[];
    pageSize?: number;
    searchText?: string;
    sortField?: string;
    sortOrder?: string;
    tags?: string[];
    title?: string;
    userId?: number;
  };

  type PostThumbAddRequest = {
    postId?: number;
  };

  type PostUpdateRequest = {
    content?: string;
    id?: number;
    tags?: string[];
    title?: string;
  };

  type PostVO = {
    content?: string;
    createTime?: string;
    favourNum?: number;
    hasFavour?: boolean;
    hasThumb?: boolean;
    id?: number;
    tagList?: string[];
    thumbNum?: number;
    title?: string;
    updateTime?: string;
    user?: UserVO;
    userId?: number;
  };

  type TgAccount = {
    createTime?: string;
    firstName?: string;
    id?: number;
    isBot?: number;
    isDelete?: number;
    lastName?: string;
    phone?: string;
    systemInfo?: string;
    tgId?: string;
    updateTime?: string;
    username?: string;
  };

  type TgAccountAddRequest = {
    firstName?: string;
    isBot?: number;
    lastName?: string;
    phone?: string;
    systemInfo?: string;
    tgId?: string;
    username?: string;
  };

  type TgAccountEditRequest = {
    firstName?: string;
    id?: number;
    isBot?: number;
    lastName?: string;
    phone?: string;
    systemInfo?: string;
    tgId?: string;
    username?: string;
  };

  type TgAccountQueryRequest = {
    current?: number;
    id?: number;
    notId?: number;
    pageSize?: number;
    phone?: string;
    searchText?: string;
    sortField?: string;
    sortOrder?: string;
    tgId?: string;
    username?: string;
  };

  type TgAccountUpdateRequest = {
    firstName?: string;
    id?: number;
    isBot?: number;
    lastName?: string;
    phone?: string;
    systemInfo?: string;
    tgId?: string;
    username?: string;
  };

  type TgAccountVO = {
    createTime?: string;
    firstName?: string;
    id?: number;
    isBot?: number;
    lastName?: string;
    phone?: string;
    systemInfo?: string;
    tgId?: string;
    updateTime?: string;
    username?: string;
  };

  type TgMessage = {
    accountId?: number;
    chatId?: string;
    content?: string;
    createTime?: string;
    id?: number;
    isDelete?: number;
    mediaPath?: string;
    msgDate?: string;
    msgId?: number;
    msgType?: string;
    senderId?: string;
  };

  type TgMessageAddRequest = {
    accountId?: number;
    chatId?: string;
    content?: string;
    mediaPath?: string;
    msgDate?: string;
    msgId?: number;
    msgType?: string;
    senderId?: string;
  };

  type TgMessageEditRequest = {
    accountId?: number;
    chatId?: string;
    content?: string;
    id?: number;
    mediaPath?: string;
    msgDate?: string;
    msgId?: number;
    msgType?: string;
    senderId?: string;
  };

  type TgMessageQueryRequest = {
    chatId?: string;
    current?: number;
    id?: number;
    msgType?: string;
    notId?: number;
    pageSize?: number;
    searchText?: string;
    senderId?: string;
    sortField?: string;
    sortOrder?: string;
  };

  type TgMessageUpdateRequest = {
    accountId?: number;
    chatId?: string;
    content?: string;
    id?: number;
    mediaPath?: string;
    msgDate?: string;
    msgId?: number;
    msgType?: string;
    senderId?: string;
  };

  type TgMessageVO = {
    accountId?: number;
    chatId?: string;
    content?: string;
    createTime?: string;
    id?: number;
    mediaPath?: string;
    msgDate?: string;
    msgId?: number;
    msgType?: string;
    senderId?: string;
  };

  type uploadFileUsingPOSTParams = {
    biz?: string;
  };

  type User = {
    createTime?: string;
    id?: number;
    isDelete?: number;
    mpOpenId?: string;
    unionId?: string;
    updateTime?: string;
    userAccount?: string;
    userAvatar?: string;
    userName?: string;
    userPassword?: string;
    userProfile?: string;
    userRole?: string;
  };

  type UserAddRequest = {
    userAccount?: string;
    userAvatar?: string;
    userName?: string;
    userRole?: string;
  };

  type userLoginByWxOpenUsingGETParams = {
    /** code */
    code: string;
  };

  type UserLoginRequest = {
    userAccount?: string;
    userPassword?: string;
  };

  type UserQueryRequest = {
    current?: number;
    id?: number;
    mpOpenId?: string;
    pageSize?: number;
    sortField?: string;
    sortOrder?: string;
    unionId?: string;
    userName?: string;
    userProfile?: string;
    userRole?: string;
  };

  type UserRegisterRequest = {
    checkPassword?: string;
    userAccount?: string;
    userPassword?: string;
  };

  type UserUpdateMyRequest = {
    userAvatar?: string;
    userName?: string;
    userProfile?: string;
  };

  type UserUpdateRequest = {
    id?: number;
    userAvatar?: string;
    userName?: string;
    userProfile?: string;
    userRole?: string;
  };

  type UserVO = {
    createTime?: string;
    id?: number;
    userAvatar?: string;
    userName?: string;
    userProfile?: string;
    userRole?: string;
  };
}
