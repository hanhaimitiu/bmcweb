# Bmcweb机制讲解：一条 HTTP 请求从 accept 到回复，内部到底怎么转

> **写给谁**：写过 openUBMC、7 年没碰 C++ 的 Java 程序员，正在系统补 OpenBMC。
> **业务背景**：先看配套的《Bmcweb业务详解.md》——那里讲了 bmcweb 是干什么的、Redfish 请求怎么变成 D-Bus 调用。本文只回答「内部机制怎么转」：服务器怎么接收连接、路由怎么匹配、认证鉴权怎么串、D-Bus 异步回调怎么组织、编译期开关怎么裁剪代码。
> **讲法**：依旧先摆真代码和真实调用链，再点概念；C++ 看不懂的一律 Java 类比（Netty、Spring MVC、Spring Security、CompletableFuture）。术语以代码里的名字为准；所有 `路径:行号` 都按当前代码逐条核对过。
> **路径说明**：路径相对 `bmcweb/` 仓库根目录。

---

## 0. 先讲一个真实例子：一条 GET 从 accept 到回复的完整链路

还是拿那条最典型的请求开刀：`GET /redfish/v1/Systems/1`。下面是从网卡收到字节，到 JSON 写回 socket 的真实调用链。**先把它整个看一遍，再逐段解释**——这一条链吃透，后面的章节都是给它填细节。

```
① 进程启动
   main → runCLI → runWebserver
   src/webserver_main.cpp:6-9 · src/webserver_cli.cpp:93-124 · src/webserver_run.cpp:51-145
   ├─ 建 io_context、systemBus，赋给全局 crow::connections::systemBus
   │    src/webserver_run.cpp:53-58 · include/dbus_singleton.hpp:11-14
   ├─ 按 BMCWEB_* 编译期开关注册各资源路由
   │    src/webserver_run.cpp:76-124
   ├─ App::run() → validate() → setupSocket() → server->run()
   │    http/app.hpp:186-194 · http/routing.hpp:161-180 · http/app.hpp:114-184
   └─ io.run() 事件循环开始转
        src/webserver_run.cpp:140

② 连接建立
   Server::run → doAccept → doAcceptOne → async_accept
   http/http_server.hpp:68-80 · :156-162 · :147-154
   → afterAccept() 包 SSL stream、建 Connection、post 到 io_context
   http/http_server.hpp:120-146

③ TLS 探测与读头
   Connection::start → async_detect_ssl → afterSslHandshake → doReadHeaders
   http/http_connection.hpp:207-235 · :237-284 · :677
   → afterReadHeaders()：读头、认证、body limit、100-continue
   http/http_connection.hpp:610-675

④ 认证
   authentication::authenticate() 依次尝试 mTLS/Xtoken/Cookie/Session/Basic
   http/http_connection.hpp:646-650 · include/authentication.hpp:255-310
   成功 → req->session = userSession    http/http_connection.hpp:401

⑤ 组装请求 + 白名单检查
   Connection::handle()：Request 构造、isOnAllowlist、401 判断
   http/http_connection.hpp:386-445
   建 AsyncResp、挂完成回调、handleUpgrade 判断、handler->handle
   http/http_connection.hpp:447-463

⑥ 路由匹配
   App::handle → Router::handle → findRoute → findRouteByPerMethod
   http/app.hpp:50-54 · http/routing.hpp:298-359 · :213-257 · :194-211
   trie 命中规则，拿到 BaseRule* + 路径参数
   http/routing.hpp:108-128 · :182-211

⑦ 鉴权
   validatePrivilege() → D-Bus GetUserInfo → isUserPrivileged → checkPrivileges
   http/routing.hpp:354-358 · include/dbus_privileges.hpp:187-206 · :74-110
   http/routing/baserule.hpp:70-87
   不够 → 403；够 → 调 rule.handle(...)

⑧ handler 执行
   TaggedRule::handle() 把 params 展开成函数实参，调绑定的 handler
   http/routing/taggedrule.hpp:53-83
   → handleComputerSystemGet()  redfish-core/lib/systems.hpp:3212-3245
   → processComputerSystemGet() 填一堆 JSON 字段
   redfish-core/lib/systems.hpp:3174-3210

⑨ 异步 D-Bus 调用
   getHostState() → dbus::utility::getProperty<std::string>(..., "CurrentHostState", 回调)
   redfish-core/lib/systems.hpp:454-531 · include/dbus_utility.hpp:123-132
   底层：systemBus->async_method_call / sdbusplus::asio::getProperty
   include/dbus_utility.hpp:100-121 · src/dbus_utility.cpp:45-53

⑩ 回调填 JSON + 发送
   回调捕获 asyncResp，写 jsonValue["PowerState"]
   redfish-core/lib/systems.hpp:482-485
   最后一个回调退出 → AsyncResp 析构 → res.end()
   include/async_resp.hpp:28-31 · http/http_response.hpp:284-298
   → 完成回调 completeRequest() → doWrite()
   http/http_connection.hpp:506-519 · :850
```

这条链里有两个「反直觉」的地方，是 Java 程序员最容易卡住的：

1. **第 ⑧ 步 handler 返回时，响应还没发**。因为真正的取数在第 ⑨ 步的异步回调里。
2. **没有任何地方显式调用「发送响应」**。发送是靠 `AsyncResp` 这个 `shared_ptr` 的引用计数归零触发析构完成的。

后面所有章节，本质都是把这两件事展开讲清楚。

### 0.1 用 Java 类比整条链

| bmcweb / C++ | Java / Spring 类比 | 对应代码 |
| --- | --- | --- |
| `boost::asio::io_context` | Netty 的 `EventLoopGroup` / `EventLoop` | `src/webserver_run.cpp:53`、`http/app.hpp:114` |
| `Server` + `async_accept` | Netty `ServerBootstrap` + boss group | `http/http_server.hpp:38-80`、`:147-162` |
| `Connection` | Netty 的一条 `NioSocketChannel` | `http/http_connection.hpp:80` |
| `Request` / `Response` | `HttpServletRequest` / `HttpServletResponse` | `http/http_request.hpp`、`http/http_response.hpp` |
| `Router` + trie | Spring `RequestMappingHandlerMapping` | `http/routing.hpp:37-128` |
| `BMCWEB_ROUTE(...).methods(...)` | `@GetMapping("/...")` | `http/app.hpp:31-32`、`redfish-core/lib/systems.hpp:3619-3622` |
| `authentication::authenticate()` | Spring Security 的 FilterChain | `include/authentication.hpp:255-310` |
| `validatePrivilege()` + `checkPrivileges()` | `@PreAuthorize("hasRole(...)")` | `include/dbus_privileges.hpp:187-206`、`http/routing/baserule.hpp:70-87` |
| handler 函数 | Controller 方法 | `redfish-core/lib/systems.hpp:3212` |
| `dbus::utility::getProperty(..., callback)` | 注入的远程服务客户端 + `CompletableFuture.thenAccept` | `include/dbus_utility.hpp:123-132` |
| `AsyncResp`（shared_ptr + 析构发送） | 引用计数的 `CompletableFuture<Response>`，最后一个回调结束时自动 flush | `include/async_resp.hpp:17-34` |
| `sdbusplus::asio::connection` | 一个线程安全的 D-Bus RPC 客户端（单例） | `include/dbus_singleton.hpp:11-14` |

把这张表记住，后面看到任何 C++ 代码，先翻译成 Java 的对应物，理解成本会低很多。

---

## 1. 先看构建与路由关系

### 1.1 编译产物：一个可执行文件 + 一个静态库

`meson.build` 把源码编成两部分：`srcfiles_bmcweb` 列出的几十个 `.cpp` 先打成静态库 `bmcweblib`（`meson.build:413-454`），然后 `webserver_main.cpp` 作为唯一入口，链接这个库生成 `bmcweb` 可执行文件（`meson.build:456-466`）。链接参数里带了 `-Wl,--gc-sections`（`meson.build:463`），配合前面 `-ffunction-sections` 一类选项（`meson.build:41-47`），目的是把没用到的代码段从最终二进制里删掉——嵌入式场景下固件体积很敏感。

入口 `main()` 只做一件事：调用 `runCLI()`（`src/webserver_main.cpp:6-9`）。`runCLI()` 用 CLI11 解析命令行，支持 `loglevel` 子命令和 `daemon` 子命令，默认直接 `runWebserver()`（`src/webserver_cli.cpp:93-124`）。

> **Java 类比**：`webserver_main.cpp` 相当于带 `public static void main` 的启动类；`bmcweblib` 相当于被打成一个大 jar 的业务代码，`main` 所在模块只负责拉起 Spring 容器。

### 1.2 编译期开关：`BMCWEB_*` 是怎么来的

这是 bmcweb 最重要的机制之一，必须捋清楚。你在源码里到处看到的 `if constexpr (BMCWEB_REDFISH)`、`if constexpr (BMCWEB_BASIC_AUTH)`，这些常量不是写在代码里的，而是 meson 在配置阶段生成的。

`config/meson.build` 列了一张 `feature_options` 清单（约 40 项，覆盖 `basic-auth`、`cookie-auth`、`redfish`、`rest`、`kvm`、`session-auth`、`xtoken-auth` 等，`config/meson.build:5-46`），遍历每个 option，把名字拼成 `BMCWEB_` 前缀 + 大写 + 下划线形式，生成形如 `constexpr const bool BMCWEB_REDFISH=true;` 的代码（`config/meson.build:65-98`），最后把整段塞进 `bmcweb_config.h`（`config/meson.build:112-124`、`config/bmcweb_config.h.in:9`）。

默认值可以在 `meson.options` 里查：例如 `rest` 默认 disabled、`redfish` 默认 enabled（`meson.options:50-68`），`kvm`、`vm-websocket` 默认 enabled（`meson.options:1-8`、`:22-28`）。

关键点在 **`if constexpr`**：这是 C++17 的编译期条件，条件为 false 的分支会被编译器整段擦除，不会生成机器码。所以关掉 KVM 不只是「运行时不注册 `/kvm/0` 路由」，而是那段代码根本不在二进制里。这就是为什么嵌入式固件能把 bmcweb 裁到很小。

实际效果在 `runWebserver()` 里看得很清楚（`src/webserver_run.cpp:76-124`）：

```cpp
if constexpr (BMCWEB_STATIC_HOSTING) { crow::webassets::requestRoutes(app); }
if constexpr (BMCWEB_KVM)            { crow::obmc_kvm::requestRoutes(app); }
if constexpr (BMCWEB_REDFISH)        { redfish::RedfishService::getInstance(app); ... }
if constexpr (BMCWEB_REST)           { crow::dbus_monitor::requestRoutes(app); ... }
```
（原文 `src/webserver_run.cpp:76-105`）

> **Java 类比**：这相当于用 `@Profile("redfish")` + 构建时 `maven.profiles` 做条件打包，但比 Java 更彻底——Java 的条件 Bean 代码仍在 jar 里，C++ 的 `if constexpr` 是把代码从二进制里物理删除。另外注意 `RedfishService`、`EventServiceManager` 这些是单例（`redfish-core/include/redfish.hpp:33-37`），相当于 Spring 里的 `@Singleton` Bean。

### 1.3 主要源码目录

| 目录 | 装什么 | 代表文件 |
| --- | --- | --- |
| `src/` | 进程入口、D-Bus 连接封装、SSL key 处理等「胶水」 | `src/webserver_main.cpp`、`src/webserver_run.cpp`、`src/dbus_utility.cpp` |
| `http/` | HTTP 服务器、连接、请求/响应、路由框架、WebSocket/SSE 基础设施 | `http/app.hpp`、`http/http_connection.hpp`、`http/http_server.hpp`、`http/routing.hpp` |
| `include/` | 跨模块公共头：认证、会话、D-Bus 工具、异步响应 | `include/authentication.hpp`、`include/sessions.hpp`、`include/dbus_utility.hpp`、`include/async_resp.hpp` |
| `redfish-core/lib/` | 每个 Redfish 资源一个头文件，实现资源组装 | `systems.hpp`（3643 行）、`sensors.hpp`、`power.hpp`、`service_root.hpp`、`redfish_v1.hpp` |
| `redfish-core/include/` | Redfish 侧公共设施：查询参数、订阅、事件管理器、权限、schema 注册表 | `query.hpp`、`privileges.hpp`、`registries/privilege_registry.hpp` |
| `redfish-core/src/` | Redfish 侧 .cpp 实现 | `redfish.cpp`（注册所有资源路由）、`subscription.cpp` |
| `features/` | 按功能切分的子模块 | `webui_login/login_routes.hpp`、`openbmc_rest/dbus_monitor.hpp`、`kvm/`、`serial/`、`virtual_media/` |
| `config/` | meson 配置模板、systemd unit 模板 | `config/meson.build`、`config/bmcweb_config.h.in` |

一个判断技巧：**看到 `requestRoutesXxx()` 命名的函数，就知道它是某资源的路由注册入口**；看到 `handleXxxGet/Patch/Post()`，就是具体 handler；看到 `afterXxx()`，多半是异步回调。这个命名习惯是读整个仓库的钥匙。

---

## 2. Boost.Beast / ASIO HTTP 服务器

### 2.1 socket 从哪来：systemd 传 fd

`App::setupSocket()` 不自己 bind 端口，而是调 `sd_listen_fds_with_names(0, &names)` 拿 systemd 已经监听好的 fd（`http/app.hpp:114-119`）。socket 名字类似 `bmcweb_443_https_auth`，代码按 `_` 拆分，取第 3 段当协议类型（`http`/`https`/`both`），默认当 HTTPS（`http/app.hpp:134-157`）。拿到 fd 后包一个 `tcp::acceptor` 放进 `acceptors` 数组（`http/app.hpp:159-169`）。只有当 systemd 没传任何 fd 时，才兜底监听 18080（`http/app.hpp:173-181`）。

这一套对 Java 程序员很陌生，但本质就是「socket 由 systemd 统一管理、进程启动后继承」，好处是重启不丢连接、端口权限由 systemd 管。等价于 Spring Boot 里把 server 端口交给外部容器托管。

### 2.2 accept 循环

`Server::run()` 先加载 TLS 证书（`http/http_server.hpp:68-90`），注册 SIGINT/SIGTERM/SIGHUP 信号处理——SIGHUP 用来热加载证书（`:92-116`），然后 `doAccept()` 对每个 acceptor 调一次 `doAcceptOne()`（`:156-162`）。

`doAcceptOne()` 创建一个 socket，发起 `async_accept`，完成时回调 `afterAccept()`（`:147-154`）。`afterAccept()` 里：建 SSL stream，创建 `Connection` 对象，然后 `boost::asio::post(getIoContext(), [connection]{ connection->start(); })` 把后续处理丢进事件循环（`:120-146`）。注意这里 **accept 之后立刻又调了一次 `doAcceptOne(*acceptor)`**，形成持续接受下一个连接的循环（`:145`）。

> **Java 类比**：`doAcceptOne` + `afterAccept` 就是 Netty 的 `bossGroup` 不断 accept 并把 channel 注册到 `workerGroup` 的过程；`post` 相当于 `eventLoop.execute()`。

### 2.3 一条连接内部的状态机

`Connection` 的处理是个异步状态机（`http/http_connection.hpp:80`）：

```
start()                         :207  → async_detect_ssl
  └─ afterDetectSsl / afterSslHandshake  :237 → 做 mTLS、HTTP/2 ALPN 协商
       └─ doReadHeaders()          :677 → 读请求头
            └─ afterReadHeaders()  :610 → 认证、body limit、100-continue
                 ├─ parse.is_done() → handle()      :386
                 └─ 否则 → doRead()                :761 → afterRead() :705 → ... → handle()
```

几个要点：
- **`async_detect_ssl`**（`:231-234`）先窥探字节判断是不是 TLS，再决定走 SSL 握手还是明文。
- **`afterSslHandshake()`** 里如果开了 mTLS，用 `verifyMtlsUser()` 拿到证书对应的 session（`:249-262`）；如果开了 HTTP/2，检查 ALPN 协商结果 `h2`，是则 `upgradeToHttp2()`（`:263-281`）。
- **`initParser()`** 创建 `request_parser<HttpBody>`，先设 header 上限，body 上限暂时不设——因为还不知道用户是否登录，登录状态会影响允许的 body 大小（`:286-297`）。
- **`handleContentLengthError()`** 用登录状态决定 body 上限：未登录用户用小限额 `loggedOutPostBodyLimit`（`:544-608`）。这是个小小的安全设计——没登录就别想上传大文件。

### 2.4 响应写回

`completeRequest()` 做四件事：把响应 move 进来、设置 keepAlive、补 `Date` 等公共头、调 `doWrite()` 真正写 socket（`http/http_connection.hpp:506-519`）。写完把完成回调置空以允许连接对象析构（`:518`）。`doWrite()` 在 `:850`。

---

## 3. 路由与 handler

### 3.1 路由是怎么注册进去的

`BMCWEB_ROUTE(app, url)` 展开成 `app.route<getParameterTag(url)>(url)`（`http/app.hpp:31-32`）。`getParameterTag` 在编译期数 URL 里有几个 `<str>` 参数，决定用几参数的 `TaggedRule`（`http/routing.hpp:52-106`，最多 5 个，`static_assert` 在 `:105`）。

`App::route()` 调 `router.newRuleTagged<Tag>()` 创建一个规则对象，塞进 `allRules` 数组（`http/app.hpp:61-65`、`http/routing.hpp:52-106`）。此时规则还没生效——要等 `validate()` 时，每条规则先 `upgrade()`，再 `validate()` 检查有没有 handler（没绑定 handler 会抛异常，`http/routing/taggedrule.hpp:28-34`），最后 `internalAddRuleObject()` 按规则声明的 method 把它加进各 method 的 trie（`http/routing.hpp:161-180`、`:130-159`）。

`PerMethod` 里每个 HTTP method 一份，含 `rules` 数组和一棵 `Trie<crow::Node>`（`http/routing.hpp:108-128`）。还有专门的 `notFoundRoutes`、`upgradeRoutes`、`methodNotAllowedRoutes`（`http/routing.hpp:386-393`）处理兜底。

### 3.2 一次匹配是怎么完成的

`findRoute()` 先遍历所有 method，看这个 URL 在别的 method 下存不存在，用来拼 `Allow` 头（`:217-236`）；再用请求的 method 去对应 trie 里查真正的规则（`:238-256`）。查不到规则就根据 `Allow` 头是否为空，分别选 404 规则或 405 规则（`Router::handle()` `:303-318`）。

`findRouteByPerMethod()` 里 `trie.find(url)` 返回 `ruleIndex` 和提取出来的路径参数 `params`（`:194-211`）。`ruleIndex == 0` 表示没匹配到（index 0 是预占的哨兵，`http/routing.hpp:112-114`）。

匹配成功后，`Router::handle()` 拿到 `BaseRule&` 和 `params`，根据 session 是否为空决定是否鉴权，最后调 `rule.handle(*req, asyncResp, params)`（`http/routing.hpp:343-358`）。

### 3.3 TaggedRule 怎么把路径参数交给 handler

`TaggedRule<Args...>::handle()` 做的事很简单：`if constexpr` 根据参数个数，把 `params[0]`、`params[1]`… 依次传给 `handler`（`http/routing/taggedrule.hpp:53-83`）。注册时用 `std::bind_front(handleComputerSystemGet, std::ref(app))` 把 `app` 预先绑到第一个参数（`redfish-core/lib/systems.hpp:3622`），于是最终调用变成：

```
handler(req, asyncResp, systemName)
  = handleComputerSystemGet(app, req, asyncResp, systemName)
```
（`handleComputerSystemGet` 签名见 `redfish-core/lib/systems.hpp:3212-3215`）

> **Java 类比**：`TaggedRule` 相当于 Spring MVC 里把 URL 模板变量（`@PathVariable`）按位置绑定到 Controller 方法实参的那一层；`std::bind_front` 相当于构造 Controller 实例时注入的依赖（这里注入的是 `App&`）。

### 3.4 WebSocket / SSE 的路由有什么不同

普通路由用 `.methods(verb)(handler)`，WebSocket 用 `.websocket().onopen(...).onclose(...).onmessage(...)`（`features/openbmc_rest/dbus_monitor.hpp:263-276`），SSE 用 `.serverSentEvent().onopen(...).onclose(...)`（`redfish-core/lib/eventservice_sse.hpp:92-96`）。这些标记把规则放进 `upgradeRoutes`（`http/routing.hpp:155-158`），由 `Router::handleUpgrade()` 处理（`http/routing.hpp:259-296`）。

在 `Connection::doUpgrade()` 里，判断请求是不是 WebSocket（`Connection: Upgrade` + `Upgrade: websocket`）或 SSE（`Accept: text/event-stream`），是则调 `handler->handleUpgrade()` 并直接 return，不再走普通 handler（`http/http_connection.hpp:314-384`）。

`handleUpgrade()` 同样先做 `validatePrivilege()`，鉴权通过才调 `rule.handleUpgrade()`（`http/routing.hpp:288-295`）。

---

## 4. 认证鉴权（session / token / basic）

### 4.1 认证：5 种方式按开关依次尝试

`authentication::authenticate()` 是认证总入口（`include/authentication.hpp:255-310`）。它先读运行时配置 `authMethodsConfig`（存在 SessionStore 里，可由 Redfish API 改），再按固定顺序尝试（`include/authentication.hpp:267-303`）：

| 顺序 | 方式 | 编译开关 | 从哪里取凭证 | 代码 |
| --- | --- | --- | --- | --- |
| 1 | mTLS | `BMCWEB_MUTUAL_TLS_AUTH` | 握手阶段建立的 `mtlsSession` | `:267-273` |
| 2 | Xtoken | `BMCWEB_XTOKEN_AUTH` | `X-Auth-Token` 头 | `:119-132`、`:274-280` |
| 3 | Cookie | `BMCWEB_COOKIE_AUTH` | `Cookie: BMCWEB-SESSION=` | `:134-199`、`:281-287` |
| 4 | Session token | `BMCWEB_SESSION_AUTH` | `Authorization: Token xxx` | `:105-117`、`:290-296` |
| 5 | Basic | `BMCWEB_BASIC_AUTH` | `Authorization: Basic xxx` | `:38-103`、`:297-303` |

每种方式都返回 `shared_ptr<UserSession>`，第一个非空的胜出。Basic 认证特别一点：base64 解出用户名密码后要调 `pamAuthenticateUser()`（`include/authentication.hpp:73`），并且会复用相同 IP + 用户的已有 Basic session（`:80-98`），避免每次请求都建新 session。

Cookie 认证还有 CSRF 保护：非 GET 请求必须带 `X-XSRF-TOKEN`，且要和 session 的 `csrfToken` 常量时间比较（`include/authentication.hpp:172-194`）。

### 4.2 SessionStore：token 到 session 的映射

`persistent_data::SessionStore` 是个单例（`include/sessions.hpp:438`），内部维护 `authTokens` map。几个关键成员：

- `sessionTokenSize = 20`，token 和 csrf token 都是这个长度（`include/sessions.hpp:32`）。
- `SessionType` 枚举区分 Basic / Session / MutualTLS / Cookie 等类型（`include/sessions.hpp:34`）。
- `generateUserSession()` 随机生成 sessionToken、csrfToken、uniqueId，构造 `UserSession` 存进 map（`include/sessions.hpp:264-298`）。Basic 和 MutualTLS 类型不写盘（`include/sessions.hpp:297` 的 `needWrite` 判断）。
- `loginSessionByToken()` 按 token 查 map，并更新 `lastUpdated`（`include/sessions.hpp:302-320`）。
- 登录接口 `handleLogin()` 走 PAM 校验后调 `generateUserSession()`，把 token 写进响应（`features/webui_login/login_routes.hpp:83-98`）。

> **Java 类比**：`SessionStore` 就是个带过期时间的 `ConcurrentHashMap<String, Session>`；`authTokens` 是 session 表；`loginSessionByToken()` 相当于 `sessionCache.get(token)` 并刷新 TTL。区别是它还要把部分 session 持久化到磁盘，重启后能恢复登录态。

### 4.3 鉴权：权限在注册路由时就定好了

认证解决「你是谁」，鉴权解决「你能不能干这事」。bmcweb 的鉴权分两层：

**第一层：路由声明权限。** 每条路由用 `.privileges(...)` 声明需要的权限集合（`redfish-core/lib/systems.hpp:3620`），权限常量定义在 `redfish-core/include/registries/privilege_registry.hpp`，比如 `getComputerSystem` 指向 `privilegeSetLogin`（`:372`），`postComputerSystem` 指向 `privilegeSetConfigureComponents`（`:375`）。`BaseRule::checkPrivileges()` 遍历声明的权限集合，只要用户的权限是其中任一项的超集就放行（`http/routing/baserule.hpp:70-87`）。

**第二层：运行时查用户角色。** `validatePrivilege()` 在请求带 session 时被 `Router::handle()` 调用（`http/routing.hpp:354-358`）。它通过 D-Bus 调 `xyz.openbmc_project.User.Manager` 的 `GetUserInfo` 拿用户信息（`include/dbus_privileges.hpp:169-185`），`populateUserInfo()` 把角色填进 session（`:60-72`），然后 `getUserPrivileges()` 把角色转成权限集合（`redfish-core/include/privileges.hpp:207`），最后 `isUserPrivileged()` 调 `checkPrivileges()`（`include/dbus_privileges.hpp:74-110`）。不过密码过期的用户会被限制到只剩 `ConfigureSelf` 权限（`:86-93`）。

> **Java 类比**：`.privileges()` 相当于方法上的 `@PreAuthorize("hasAnyRole('Login')")`，`validatePrivilege()` + D-Bus GetUserInfo 相当于一个 `UserDetailsService` 在每次请求时加载用户角色，再交给 Spring Security 的投票器决策。

---

## 5. D-Bus 异步调用模式

### 5.1 全局连接与工具封装

所有 D-Bus 调用都走一个全局连接 `crow::connections::systemBus`（`include/dbus_singleton.hpp:11-14`），它在 `runWebserver()` 里初始化（`src/webserver_run.cpp:56-58`）。这相当于一个进程级的 D-Bus 客户端单例。

`dbus::utility` 命名空间封装了常用操作（`include/dbus_utility.hpp`）：

| 封装 | 作用 | 声明位置 |
| --- | --- | --- |
| `async_method_call` | 调任意 D-Bus 方法，转发给 `systemBus->async_method_call` | `include/dbus_utility.hpp:100-121` |
| `getProperty<T>` | 读单个属性，底层 `sdbusplus::asio::getProperty` | `include/dbus_utility.hpp:123-132` |
| `getAllProperties` | 读接口所有属性 | `include/dbus_utility.hpp:95-98`、`src/dbus_utility.cpp:45-53` |
| `getSubTree` / `getSubTreePaths` | 问 ObjectMapper「哪些对象提供某接口」 | `include/dbus_utility.hpp:154-164`、`src/dbus_utility.cpp:78-120` |
| `getDbusObject` | 问 ObjectMapper「某路径由哪个服务提供」 | `include/dbus_utility.hpp:196-199`、`src/dbus_utility.cpp:218-230` |
| `getAssociatedSubTree*` | 沿 association 查关联对象 | `include/dbus_utility.hpp:166-194` |

D-Bus 属性的类型是不固定的，所以代码里定义了一个巨大的 `std::variant`——`DbusVariantType`（`include/dbus_utility.hpp:34-63`），涵盖 int、double、string、bool、vector、tuple 等几十种可能；`DBusPropertiesMap` 就是 `vector<pair<string, DbusVariantType>>`（`:66`）。handler 拿到属性后用 `std::get_if<T>()` 尝试取出期望类型（典型例子见 `redfish-core/lib/power.hpp:148-188`）。

> **Java 类比**：`DbusVariantType` 相当于一个 `Object`/`JsonNode` 万能容器；`std::get_if<double>()` 相当于 `(Double) node` 的强制转型。

### 5.2 异步回调的典型写法

所有 D-Bus 调用都是异步的，模式固定：发起调用时传一个 lambda 回调，回调里检查 `error_code`，成功就填 `asyncResp->res.jsonValue`，失败就报错。以读主机状态为例（`redfish-core/lib/systems.hpp:454-531`）：

```cpp
dbus::utility::getProperty<std::string>(
    serviceName, path, "xyz.openbmc_project.State.Host", "CurrentHostState",
    [asyncResp](const boost::system::error_code& ec,
                const std::string& hostState) {
        if (ec) { /* 错误处理 */ return; }
        if (hostState == "...HostState.Running") {
            asyncResp->res.jsonValue["PowerState"] = resource::PowerState::On;
        }
        // ...
    });
```

这个 lambda **按值捕获 `asyncResp`**（一个 `shared_ptr`），所以只要这个回调还没执行完，`AsyncResp` 就不会析构，响应就不会提前发出。这就是第 0 节说的「引用计数决定响应时机」。

### 5.3 回调嵌套：多个异步调用串联

有的操作要连续做好几步 D-Bus 调用，代码就写成回调里再发调用。比如设置功率上限：先 `getProperty<bool>("PowerCapEnable")`，回调里判断功能是否开启，再 `setDbusProperty()` 写 `PowerCap`（`redfish-core/lib/power.hpp:105-109`、`redfish-core/lib/power.hpp:43-68`）。再比如查 chassis 路径：`getSubTreePaths()` 回调里 `afterGetChassis()`，里面再 `getAllProperties()`（`redfish-core/lib/power.hpp:287-289`、`redfish-core/lib/power.hpp:254-261`）。

> **Java 类比**：这就是 `CompletableFuture.thenCompose(...)` 的链式写法，或者 Node.js 的 callback hell。C++ 这里没有 `async/await` 语法糖（至少这个仓库没用），全靠 lambda 嵌套。读代码时要养成习惯：**看到 `std::bind_front(后续函数, asyncResp, ...)` 或 `[asyncResp](...){...}`，就知道又串了一步异步调用。**

---

## 6. Redfish schema 组装

### 6.1 JSON 是怎么搭出来的

每个 Redfish 资源 handler 的核心工作，就是往 `asyncResp->res.jsonValue`（一个 `nlohmann::json` 对象，`http/http_response.hpp:51`）里填字段。以 ServiceRoot 为例（`redfish-core/lib/service_root.hpp:42-114`）：

```cpp
asyncResp->res.jsonValue["@odata.type"] = "#ServiceRoot.v1_15_0.ServiceRoot";
asyncResp->res.jsonValue["@odata.id"] = "/redfish/v1";
asyncResp->res.jsonValue["RedfishVersion"] = "1.17.0";
asyncResp->res.jsonValue["Systems"]["@odata.id"] = "/redfish/v1/Systems";
asyncResp->res.jsonValue["UUID"] = uuid;   // 来自持久化配置
// ... 几十行类似字段
```
（原文 `redfish-core/lib/service_root.hpp:50-85`）

注意几个细节：
- schema 版本号（如 `v1_15_0`）和 Redfish 版本（`"1.17.0"`）是**写死在代码里的字符串**（`:51`、`:55`），升级 Redfish 协议支持要改代码。
- 有些字段来自 D-Bus 或配置，比如 `UUID` 来自 `persistent_data::getConfig().systemUuid`（`:49`），`manager_utils::getServiceIdentification()` 会异步补一些字段（`:85`）。
- 编译期开关会改变输出的字段，比如 `BMCWEB_REDFISH_AGGREGATION` 决定要不要输出 `AggregationService` 链接（`:60-64`）。

### 6.2 查询参数：$expand / $select 等

几乎每个 handler 开头都会调 `setUpRedfishRoute()` 或 `setUpRedfishRouteWithDelegation()`（`redfish-core/include/query.hpp:208-216`、`:116`）。它处理 Redfish 标准查询参数：`$expand`、`$select`、`$filter` 等。很多 handler 第一行就是：

```cpp
if (!redfish::setUpRedfishRoute(app, req, asyncResp)) { return; }
```
（如 `redfish-core/lib/systems.hpp:3217`、`redfish-core/lib/power.hpp:269`）

如果返回 false，说明参数已经把响应处理完了（比如 `$select` 只要部分字段），handler 就直接返回。这部分逻辑集中在 `query.hpp`，是 Redfish 协议实现里比较独立的一块。

### 6.3 错误消息

错误不是随便塞字符串，而是走 `messages::xxx()` 系列函数（`redfish-core/include/error_messages.hpp`），比如 `messages::internalError()`、`messages::resourceNotFound()`、`messages::actionNotSupported()`。典型用法：

```cpp
if (ec) { messages::internalError(asyncResp->res); return; }
```
（`redfish-core/lib/power.hpp:48-52`）

这些函数按 Redfish 规范生成标准的 `error` JSON 结构（含 `@MessageId`、`Message` 等）。

> **Java 类比**：`nlohmann::json` 相当于 Jackson 的 `ObjectNode`；`messages::xxx()` 相当于一个统一的异常处理器 / `ErrorResponse` 构造器，保证错误响应符合 Redfish  schema。

---

## 7. 事件订阅

### 7.1 两种推送通道

| 通道 | 路由 | 基础设施 | 代码 |
| --- | --- | --- | --- |
| Redfish SSE | `/redfish/v1/EventService/SSE/` | `serverSentEvent` | `redfish-core/lib/eventservice_sse.hpp:88-97` |
| D-Bus 事件 WebSocket | `/subscribe/` | `.websocket()` | `features/openbmc_rest/dbus_monitor.hpp:263-276` |
| 事件订阅管理（POST） | `/redfish/v1/EventService/Subscriptions/` | REST | `redfish-core/lib/event_service.hpp:739-742` |

### 7.2 SSE 订阅的建立

SSE 路由注册时给了 `postEventDestinationCollection` 权限，因为它本质上就是建订阅（`redfish-core/lib/eventservice_sse.hpp:90-96`）。连上后 `createSubscription()`：检查订阅数上限（`:29-36`）、解析 `$filter`（`:38-52`）、读 `Last-Event-Id`（`:54`）、建 `Subscription` 挂进 `EventServiceManager`（`:56-76`）。断开时 `deleteSubscription()` 移除（`redfish-core/lib/eventservice_sse.hpp:83-86`）。

### 7.3 WebSocket D-Bus 监听

`/subscribe/` 的 onopen 把连接存进 sessions map，onclose 删除（`features/openbmc_rest/dbus_monitor.hpp:268-274`），onmessage 调 `handleDbusMonitorMessage` 处理客户端想监听的路径。服务端用 `sdbusplus::match` 注册 `PropertiesChanged` 和 `InterfacesAdded` 信号（`:250-259`），有变化就往 websocket 推。

> **Java 类比**：SSE 是 `SseEmitter` + 一个订阅注册表；WebSocket 那套 `sdbusplus::match` 就是给 D-Bus 信号挂 listener，事件驱动地 push 给客户端，省掉前端轮询。

---

## 8. 错误处理

几条贯穿全局的规则：

1. **D-Bus 调用先看 `error_code`**。每个回调第一个参数几乎都是 `const boost::system::error_code& ec`，非空就是出错，先 `messages::internalError()` 再 return（如 `redfish-core/lib/power.hpp:117-122`、`redfish-core/lib/systems.hpp:466-477`）。有些错误被特殊处理：`host_unreachable` 表示服务没起来，不算内部错误（`redfish-core/lib/systems.hpp:468-474`）；`io_error` 在 `getSubTreePaths` 里被当成「没找到对象」的正常情况（`src/dbus_utility.cpp:104-115`）。
2. **解析请求体失败就 400**。`readJsonPatch()` / `readJsonObject()` 返回 false 时，函数内部已经往 res 写了错误，handler 直接 return（`redfish-core/lib/power.hpp:309-316`）。
3. **权限不够回 403**，密码过期回带 `passwordChangeRequired` 消息的 403（`include/dbus_privileges.hpp:95-105`）。
4. **找不到资源回 404**，方法不对回 405，由兜底路由统一处理（`redfish-core/lib/redfish_v1.hpp:46-89`）。
5. **响应只能 end 一次**，重复 end 会打错误日志（`http/http_response.hpp:284-290`）。

---

## 9. 几个特殊要点（踩坑提示）

- **`AsyncResp` 不可拷贝不可移动**（`include/async_resp.hpp:23-26`），只能靠 `shared_ptr` 传递。想临时存就传 `shared_ptr`，别试图按值传。
- **编译期开关改的是二进制内容**，不是运行时配置（`config/meson.build:78-81`）。线上行为不同，先查构建配置。
- **部分开关名带 `INSECURE` 前缀**，比如 `BMCWEB_INSECURE_DISABLE_SSL`、`BMCWEB_INSECURE_DISABLE_AUTH`、CSRF 相关开关，看名字就知道默认是安全方向，改它们要谨慎。
- **读写同一个资源可能落在不同 D-Bus 服务上**。比如开机写的是 `xyz.openbmc_project.State.Host`，而读传感器走的是 ObjectMapper 动态发现的服务（`redfish-core/lib/systems.hpp:2942-2947` vs `redfish-core/lib/sensors.hpp:2188-2189`）。别以为所有数据都来自同一服务。
- **权限是路由级的**（`README.md:44-45`）。新增路由时若忘了 `.privileges(...)`，`BaseRule::checkPrivileges()` 会因为 `privilegesSet` 为空而直接放行（`http/routing/baserule.hpp:72-77`）——这是个容易埋雷的默认行为，务必显式声明权限。
- **REST 和 Redfish 是两套独立路由**，分别由 `BMCWEB_REST` 和 `BMCWEB_REDFISH` 控制（`src/webserver_run.cpp:86-105`）。传统 REST 的路径直接映射 D-Bus 对象（如 `/xyz/...`），注册在 `features/openbmc_rest/openbmc_dbus_rest.hpp:2520` 起。

把第 0 节那条链、第 1.2 节的编译期开关、第 5.2 节的异步回调模式这三样吃透，你就读懂了 bmcweb 80% 的机制。剩下的都是往这套骨架上挂具体资源而已。
