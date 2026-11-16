# Bmcweb业务详解：浏览器里一次「看电源状态」，背后走了一整条 D-Bus 链路

> **写给谁**：写过 openUBMC、但 7 年没碰 C++ 的 Java 程序员，正在系统补 OpenBMC。
> **本文回答什么**：bmcweb 到底是个什么业务？浏览器/Redfish 客户端发来的一个 HTTP 请求，是怎么一步步变成对 D-Bus 上硬件对象的读写的？Web 界面点「开机」，这条命令又是怎么落到状态管理服务上的？
> **讲法**：先摆真实场景、真实 JSON、真实代码，再点概念。C++ 看不懂的地方一律用 Java 类比——异步 HTTP 服务器想成 Netty，路由注册想成 Spring MVC 的 `@GetMapping`，认证/鉴权想成 Spring Security 的 Filter 链，D-Bus 异步调用想成 `CompletableFuture` 回调。术语直接用代码里的原名，不另起译名。
> **路径说明**：文中 `路径:行号` 都是 `bmcweb/` 仓库内的相对路径，行号按当前代码逐条核对过。看不懂的类名，直接按路径打开对照即可。

---

## 0. 一句话说清业务

把你在 openUBMC 里的心智直接搬过来：**bmcweb 就是 OpenBMC 的 Web 服务器**，它自己不产生任何硬件数据，而是把 D-Bus 上那些硬件状态对象（`xyz.openbmc_project.State.Host`、`xyz.openbmc_project.Sensor.*`……）包装成 Redfish 和传统 REST / WebSocket API，是运维界面（webui-vue）和所有 Redfish/REST 客户端的唯一 HTTP 入口。

官方 README 把它列成一个「do everything 的嵌入式 Web 服务器」，实现了几种不同的接口：D-Bus 事件 WebSocket、OpenBMC D-Bus REST API、串口 WebSocket、Redfish（明确写着是「DBus to Redfish translator」）、KVM 的 RFB/VNC WebSocket（`README.md:3` `README.md:7-17`）。

换成 Java 的说法：**bmcweb 相当于给整个 BMC 套了一层 Spring Boot + Spring MVC 的 Controller 层**，只不过下面对接的不是数据库和 Service，而是 D-Bus；对外输出的不是普通 JSON，而是符合 DMTF 规范的 Redfish JSON。

它对外提供 http 和 https 两种协议，TLS 走 OpenSSL，HTTP/1 和 HTTP/2 都支持（`README.md:19-23`）。认证方式一大堆——Basic、Cookie、mTLS、Session、X-Auth-Token，全部可以被编译期或运行时开关单独打开/关掉，用户名密码最后都交给 libpam 去校验（`README.md:27-40`）。鉴权则在注册路由时就定好，每条路由各自声明需要什么权限，遵循 Redfish 的 PrivilegeRegistry（`README.md:42-48`）。

这一层「翻译 + 唯一入口」的定位，就是理解整个仓库的主干。剩下 1300 多个文件，基本都是这层的各种资源实现和工具函数。

---

## 1. 真实场景一：浏览器打开 Redfish `/redfish/v1/Systems/1`，看到电源状态，这条 HTTP 请求内部怎么走

先看你实际会看到的东西。客户端 GET `/redfish/v1/Systems/1`，拿到的 JSON 里有一段是这样的（字段名直接来自代码）：

```json
{
  "@odata.type":   "#ComputerSystem.v1_20_0.ComputerSystem",
  "@odata.id":     "/redfish/v1/Systems/1",
  "PowerState":    "On",
  "Status": { "State": "Enabled", "Health": "OK" }
}
```

其中 `PowerState` 这个字段的值，就是 bmcweb 从 D-Bus 属性 `xyz.openbmc_project.State.Host` 的 `CurrentHostState` 读出来再翻译的（`redfish-core/lib/systems.hpp:460-462`、`redfish-core/lib/systems.hpp:482-485`）。

> 说明一下 URL 里的 `1`：路由本身是 `/redfish/v1/Systems/<str>/`，`<str>` 是路径参数（`redfish-core/lib/systems.hpp:3614`）；这个「系统 ID」来自编译选项 `redfish-system-uri-name`，默认值其实是 `system`（`meson.options:188-191`），很多发行版会把它配成 `1`。所以你看到 `Systems/1` 还是 `Systems/system`，取决于构建时的配置，链路完全一样。

### 1.1 先把完整链路摆出来

下面这条链，是从「浏览器按下回车」到「JSON 回到浏览器」真正经过的函数调用。每个箭头后面都带了 `文件:行号`，可以逐个打开核对：

```
浏览器 GET /redfish/v1/Systems/1
  │
  ├─① socket 监听：systemd 把 listen fd 传给进程，App::setupSocket() 解析 socket 名字
  │     http/app.hpp:114-184   （默认端口 18080 兜底：http/app.hpp:175-180）
  │     App::run() → Server::run() → doAccept()
  │     http/app.hpp:186-194 · http/http_server.hpp:68-80 · http/http_server.hpp:156-162
  │
  ├─② accept 连接：Server::afterAccept() 建 SSL stream + Connection 对象
  │     http/http_server.hpp:120-146
  │     Connection::start() → async_detect_ssl() → afterSslHandshake() → doReadHeaders()
  │     http/http_connection.hpp:207-235 · http/http_connection.hpp:237-284
  │
  ├─③ 读请求头并认证：afterReadHeaders() → authentication::authenticate()
  │     http/http_connection.hpp:610-675 （认证调用在 :646-650）
  │     include/authentication.hpp:255-310
  │
  ├─④ 组装请求、查白名单：Connection::handle()
  │     req->session = userSession                      http/http_connection.hpp:401
  │     isOnAllowlist() + 未登录 → 401                   http/http_connection.hpp:431-445
  │     建 AsyncResp、挂完成回调                          http/http_connection.hpp:447-452
  │
  ├─⑤ 交给路由：handler->handle(req, asyncResp)
  │     http/http_connection.hpp:463
  │     App::handle() → Router::handle()                 http/app.hpp:50-54 · http/routing.hpp:298-359
  │
  ├─⑥ 查路由表：findRoute() 用 trie 按 URL 匹配到规则
  │     http/routing.hpp:213-257 · http/routing.hpp:194-211
  │
  ├─⑦ 鉴权：validatePrivilege() → D-Bus GetUserInfo → checkPrivileges()
  │     http/routing.hpp:354-358 · include/dbus_privileges.hpp:187-206
  │     include/dbus_privileges.hpp:74-110 · http/routing/baserule.hpp:70-87
  │
  ├─⑧ 命中 handler：TaggedRule::handle() 调绑定的函数
  │     http/routing/taggedrule.hpp:53-83
  │     → handleComputerSystemGet()  redfish-core/lib/systems.hpp:3212-3245
  │
  ├─⑨ 组装数据：先 processComputerSystemGet() 填字段
  │     redfish-core/lib/systems.hpp:3174-3210
  │     其中 getHostState() 异步读 D-Bus 的 CurrentHostState
  │     redfish-core/lib/systems.hpp:454-531
  │
  ├─⑩ D-Bus 调用：dbus::utility::getProperty → sdbusplus::asio::getProperty
  │     include/dbus_utility.hpp:123-132
  │     全局 systemBus 连接：include/dbus_singleton.hpp:8-16
  │
  ├─⑪ 回调填 JSON：拿到 "HostState.Running" → 写 PowerState="On"
  │     redfish-core/lib/systems.hpp:482-485
  │
  └─⑫ 发送响应：AsyncResp 析构 → res.end() → completeRequest() → doWrite()
        include/async_resp.hpp:28-31 · http/http_response.hpp:284-298
        http/http_connection.hpp:506-519 · http/http_connection.hpp:850
```

这一长串看着吓人，其实就是「Netty 接收请求 → Spring Security 过滤 → Spring MVC 找到 Controller → Controller 调远程服务 → 把结果写回 Netty channel」。下面挑几段真正决定行为的拆开讲。

### 1.2 请求是怎么被 accept 进来的

服务启动时，`runWebserver()` 拿到全局 `io_context`，创建到系统总线的连接，并把它塞进全局变量 `crow::connections::systemBus`（`src/webserver_run.cpp:51-58`）。这个全局变量就是后面所有 D-Bus 调用的出口，声明在 `include/dbus_singleton.hpp:11-14`。

然后它按编译期开关逐段注册路由：`BMCWEB_STATIC_HOSTING` 开静态资源、`BMCWEB_REDFISH` 开 Redfish、`BMCWEB_REST` 开传统 REST、`BMCWEB_KVM` 开 KVM（`src/webserver_run.cpp:76-117`）。**注意 `if constexpr` 这个写法**——这是 C++ 编译期 if，条件为 false 的代码块会被编译器整段丢掉，相当于 Java 里用 `@Profile` 或者条件 Bean，但发生在编译期而不是运行期。最后 `app.run()` 启动服务器、`io.run()` 让事件循环转起来（`src/webserver_run.cpp:136-140`）。

监听 socket 不是代码里 new 出来的，而是 systemd 通过 `sd_listen_fds_with_names()` 传进来的（名字类似 `bmcweb_443_https_auth`），代码解析名字里的 `http`/`https`/`both` 决定用哪种协议（`http/app.hpp:114-168`）。没有 systemd 传 fd 时才兜底监听 18080 端口（`http/app.hpp:173-181`）。

`Server::doAcceptOne()` 对每个 acceptor 发一个 `async_accept`（`http/http_server.hpp:147-154`），连上后在 `afterAccept()` 里包一层 SSL stream，再 `post` 到 io_context 让 `Connection::start()` 跑起来（`http/http_server.hpp:120-146`）。

> **Java 类比**：`Server` 就是 Netty 的 `ServerBootstrap` + `bossGroup`，`async_accept` 对应 `NioServerSocketChannel` 的 accept 事件；每个 `Connection` 对应一条 `NioSocketChannel`；`boost::asio::post(io_context, ...)` 相当于往 Netty 的 `eventLoop.execute(...)` 丢任务。

### 1.3 认证：在读到 header 的那一刻就做

`Connection::afterReadHeaders()` 一旦读到请求头，立刻调 `authentication::authenticate(...)`（`http/http_connection.hpp:646-650`）。这个函数做的事很直接：按编译期开关和运行时配置，依次尝试各种认证方式（`include/authentication.hpp:255-310`）：

- `BMCWEB_MUTUAL_TLS_AUTH`：用握手阶段建立的 mTLS session（`include/authentication.hpp:267-273`）
- `BMCWEB_XTOKEN_AUTH`：读 `X-Auth-Token` 头（`include/authentication.hpp:119-132`、`:274-280`）
- `BMCWEB_COOKIE_AUTH`：读 `Cookie` 里的 `BMCWEB-SESSION=`，而且非 GET 请求还要校验 `X-XSRF-TOKEN`（`include/authentication.hpp:134-199`、`:281-287`）
- `BMCWEB_SESSION_AUTH`：读 `Authorization: Token xxx`（`include/authentication.hpp:105-117`、`:290-296`）
- `BMCWEB_BASIC_AUTH`：读 `Authorization: Basic xxx`，base64 解出用户名密码后走 `pamAuthenticateUser()`（`include/authentication.hpp:38-103`、`:297-303`）

认证成功后得到一个 `UserSession`，存进 `req->session`（`http/http_connection.hpp:401`）。紧接着 `handle()` 检查：如果 URL 不在白名单里、又没登录，就直接回 401（`http/http_connection.hpp:431-445`）。白名单本身在 `isOnAllowlist()` 里写死——比如 `/redfish`、`/redfish/v1`、登录和创建 session 的 POST 都允许匿名访问（`include/authentication.hpp:216-253`）。

> **Java 类比**：这一段就是 Spring Security 的 `UsernamePasswordAuthenticationFilter` + `BasicAuthenticationFilter` + `CookieAuthenticationFilter` 串成的一条 FilterChain；`isOnAllowlist()` 就是 `WebSecurityConfigurerAdapter` 里的 `permitAll()` 规则。

### 1.4 路由：一张按 URL 前缀建的 trie

`Router::handle()` 先 `findRoute()` 找到匹配的规则（`http/routing.hpp:298-301`）。`findRoute()` 对每个 HTTP method 各有一张 `PerMethod`，里面是一棵 trie（`http/routing.hpp:213-257`、`http/routing.hpp:108-128`）。路由是在启动时由各资源的 `requestRoutesXxx(app)` 注册的，比如 Systems 的路由集中注册在 `requestRoutesSystems()`（`redfish-core/lib/systems.hpp:3602-3642`）：

```cpp
BMCWEB_ROUTE(app, "/redfish/v1/Systems/<str>/")
    .privileges(redfish::privileges::getComputerSystem)
    .methods(boost::beast::http::verb::get)(
        std::bind_front(handleComputerSystemGet, std::ref(app)));
```
（原文 `redfish-core/lib/systems.hpp:3619-3622`）

`BMCWEB_ROUTE` 这个宏本质是 `app.route<编译期算出的tag>(url)`（`http/app.hpp:31-32`），`<str>` 的个数会被编译期算出 tag，决定用几参数的 `TaggedRule`（`http/routing.hpp:52-106`）。`.methods(...)` 声明这条规则接受哪些 HTTP 动词，`.privileges(...)` 声明需要哪些权限——权限常量是从 privilege_registry 里取的引用，例如 `getComputerSystem` 等价于 `privilegeSetLogin`（`redfish-core/include/registries/privilege_registry.hpp:372`），`getPower` 同样是 `privilegeSetLogin`（`redfish-core/include/registries/privilege_registry.hpp:1470`）。

命中规则后，如果请求带 session，还要走一次鉴权：`validatePrivilege()` 通过 D-Bus 调 `xyz.openbmc_project.User.Manager` 的 `GetUserInfo` 拿用户角色（`include/dbus_privileges.hpp:169-185`），再用 `rule.checkPrivileges()` 比对权限集合（`http/routing/baserule.hpp:70-87`），不够就回 403（`include/dbus_privileges.hpp:95-105`）。

> **Java 类比**：`BMCWEB_ROUTE(...).privileges(...).methods(...)(handler)` 这一串链式调用，就是 Spring MVC 里 `@GetMapping` + `@PreAuthorize` + 方法体的组合；`Router` 的 trie 相当于 Spring 的 `RequestMappingHandlerMapping`，按 URL pattern 找 handler method。

### 1.5 handler 怎么把 D-Bus 数据变成 Redfish JSON

`handleComputerSystemGet()` 先调 `setUpRedfishRoute()` 处理 `$expand`、`$select` 这类查询参数（`redfish-core/lib/systems.hpp:3217-3220`，函数定义在 `redfish-core/include/query.hpp:208-216`），再校验 systemName，最后调 `processComputerSystemGet()`（`redfish-core/lib/systems.hpp:3231-3244`）。

`processComputerSystemGet()` 是一连串「取数据」函数的集合——它依次调用 `getComputerSystem()`、`getMainChassisId()`、`getHostState()`、`getBootProperties()`、`getBootProgress()`、`getPowerRestorePolicy()` 等一大票函数（`redfish-core/lib/systems.hpp:3174-3210`）。每个函数负责往 `asyncResp->res.jsonValue` 里填一部分字段。**这就是 Redfish 资源组装的真实形态：不是一个大函数从头写到尾，而是一堆小函数异步并发地各填一块 JSON。**

我们关心的 `PowerState` 来自 `getHostState()`（`redfish-core/lib/systems.hpp:3194`）。它做的事情非常典型：

```cpp
dbus::utility::getProperty<std::string>(
    systems_utils::getHostStateServiceName(computerSystemIndex), path,
    "xyz.openbmc_project.State.Host", "CurrentHostState",
    [asyncResp](const boost::system::error_code& ec,
                const std::string& hostState) {
        if (hostState == "xyz.openbmc_project.State.Host.HostState.Running")
        {
            asyncResp->res.jsonValue["PowerState"] = resource::PowerState::On;
            asyncResp->res.jsonValue["Status"]["State"] = resource::State::Enabled;
        }
        // ... 其它状态分支
    });
```
（原文 `redfish-core/lib/systems.hpp:460-485`）

也就是说：**Redfish 的 `PowerState=On`，本质是代码把 D-Bus 字符串 `xyz.openbmc_project.State.Host.HostState.Running` 翻译成中文枚举的结果**（其它状态如 `Quiesced`→On、`TransitioningToRunning`→`PoweringOn`、其余→Off，见 `redfish-core/lib/systems.hpp:489-529`）。这一步就是「DBus to Redfish translator」的字面含义。

### 1.6 关键机制：为什么 handler 返回了，响应还没发出去

这是刚从 Java 过来最容易懵的地方。你看 `handleComputerSystemGet()` 最后是 `getHostState(...)` 这种异步调用，函数本身没有 return 任何数据，那响应什么时候发出去？

答案在 `AsyncResp`：它内部只有一个 `crow::Response res`（`include/async_resp.hpp:33`），而它的析构函数会调用 `res.end()`（`include/async_resp.hpp:28-31`）。`res.end()` 又会触发挂在它上面的完成回调（`http/http_response.hpp:284-298`），这个回调是 `Connection::handle()` 在 `http/http_connection.hpp:449-452` 挂上去的，最终调用 `completeRequest()` → `doWrite()` 把字节写回 socket（`http/http_connection.hpp:506-519`）。

而 `asyncResp` 是一个 `shared_ptr`，它被传给每一个异步 D-Bus 调用的回调 lambda 捕获着。所以：**当最后一个 D-Bus 回调执行完、最后一个持有 `asyncResp` 的 lambda 被销毁时，`AsyncResp` 才析构，响应才发出去。**

> **Java 类比**：把 `AsyncResp` 想成一个引用计数的 `CompletableFuture<Response>`。每个异步 D-Bus 调用都是 `thenAcceptAsync(...)`，回调里往 Future 的结果对象里填字段；当所有 `thenAccept` 链都结束、没人再持有这个 Future 时，它的 `finally`/`close` 自动把结果 flush 出去。C++ 这里用 `shared_ptr` 的引用计数替代了 Java 的「所有 Future 都 complete」这一判断，非常巧但也很容易踩坑——如果你发起了异步调用却忘了让回调捕获 `asyncResp`，对象可能提前析构，响应就发了半截。

### 1.7 这些路由是怎么在启动时搭起来的

你会好奇：`requestRoutesSystems()` 这些函数是谁调用的？答案是 `RedfishService` 的构造函数。`runWebserver()` 里 `BMCWEB_REDFISH` 开着时调 `redfish::RedfishService::getInstance(app)`（`src/webserver_run.cpp:86-88`），这是个函数内静态单例（`redfish-core/include/redfish.hpp:33-37`），构造时一次性把所有 Redfish 资源注册进去（`redfish-core/src/redfish.cpp:74-248`）。

挑几个你以后最常打交道的资源，看它们各自在构造函数哪一行注册：

| Redfish 资源 | 注册函数 | 代码位置 |
| --- | --- | --- |
| ServiceRoot `/redfish/v1/` | `requestRoutesServiceRoot` | `redfish-core/src/redfish.cpp:90` |
| SessionService | `requestRoutesSession` | `redfish-core/src/redfish.cpp:92` |
| Chassis | `requestRoutesChassis` | `redfish-core/src/redfish.cpp:113` |
| Systems | `requestRoutesSystems` | `redfish-core/src/redfish.cpp:184` |
| Sensors | `requestRoutesSensorCollection` / `requestRoutesSensor` | `redfish-core/src/redfish.cpp:213-214` |
| EventService | `requestRoutesEventService` | `redfish-core/src/redfish.cpp:220` |
| TaskService | `requestRoutesTaskService` | `redfish-core/src/redfish.cpp:217` |
| AccountService | `requestAccountServiceRoutes` | `redfish-core/src/redfish.cpp:79` |
| UpdateService | `requestRoutesUpdateService` | `redfish-core/src/redfish.cpp:117` |
| LogServices | `requestRoutesSystemsLogServiceCollection` | `redfish-core/src/redfish.cpp:126` |

注意 `redfish-core/src/redfish.cpp:94-108`：老的 `Thermal`/`Power` 资源和新的 `PowerSubsystem`/`ThermalSubsystem` 是两套编译期开关（`BMCWEB_REDFISH_ALLOW_DEPRECATED_POWER_THERMAL` 和 `BMCWEB_REDFISH_NEW_POWERSUBSYSTEM_THERMALSUBSYSTEM`）分别控制的。你在线上看到的是哪一套，取决于构建配置。构造函数最后一条注册的是 `requestRoutesRedfish(app)`（`redfish-core/src/redfish.cpp:242-243`），注释明确写着「must be the last route registered」——因为它注册的是 `/redfish/<path>` 这个兜底 404/405 路由（`redfish-core/lib/redfish_v1.hpp:265-274`），必须先让具体路由都注册完，兜底的才生效。所有路由注册完后 `validate()` 把 trie 建好（`redfish-core/src/redfish.cpp:247`、`http/routing.hpp:161-180`）。

> **Java 类比**：`RedfishService` 构造函数相当于一个 `@Configuration` 类里集中 `@Bean` 注册所有 `HandlerMapping`；那条「最后注册兜底路由」的约束，相当于 Spring 里「兜底的 `/**` Controller 必须最后声明」，否则会把具体路径全吃掉。

---

## 2. 真实场景二：Web 界面点「开机」→ REST 怎么转成 D-Bus 调用

现在看写操作。运维在 webui-vue 上点「开机」，浏览器实际发的是：

```http
POST /redfish/v1/Systems/1/Actions/ComputerSystem.Reset
Content-Type: application/json

{ "ResetType": "On" }
```

### 2.1 这条请求走的路

```
POST .../Actions/ComputerSystem.Reset
  │
  ├─① 路由命中 Reset action 规则（GET 那条链路的 ①~⑧ 完全一样）
  │     BMCWEB_ROUTE("/redfish/v1/Systems/<str>/Actions/ComputerSystem.Reset/")
  │     .privileges(postComputerSystem)   redfish-core/lib/systems.hpp:3629-3632
  │
  ├─② handler：handleComputerSystemResetActionPost()
  │     redfish-core/lib/systems.hpp:2960-...
  │     → setUpRedfishRoute()           redfish-core/lib/systems.hpp:2965
  │     → 校验 systemName               redfish-core/lib/systems.hpp:2979-2987
  │     → 解析请求体里的 ResetType
  │
  ├─③ 翻译动作："On" → D-Bus transition 字符串
  │     processComputerSystemResetActionPost()
  │     redfish-core/lib/systems.hpp:2890-2900
  │
  ├─④ 写 D-Bus 属性：setDbusProperty(..., "RequestedHostTransition", command)
  │     redfish-core/lib/systems.hpp:2940-2947
  │
  └─⑤ 回调里写回 200 + success，AsyncResp 析构触发响应发送
```

### 2.2 权限比读操作严

注意路由声明的权限变了：Reset action 用的是 `postComputerSystem`，它等价于 `privilegeSetConfigureComponents`（`redfish-core/include/registries/privilege_registry.hpp:375`），比读操作的 `privilegeSetLogin` 高一级。也就是说「谁能看电源状态」和「谁能开机」是两套权限，这就是 README 说的「All authorization ... per route」（`README.md:44-45`）。

### 2.3 "On" 是怎么变成一条 D-Bus 属性写入的

核心在 `processComputerSystemResetActionPost()`。它拿到请求体里的 `resetType` 字符串，翻译成对应的 D-Bus transition 值（`redfish-core/lib/systems.hpp:2894-2938`）：

| Redfish ResetType | D-Bus transition 值 | host/chassis |
| --- | --- | --- |
| `On` / `ForceOn` | `xyz.openbmc_project.State.Host.Transition.On` | host（`redfish-core/lib/systems.hpp:2897-2900`） |
| `ForceOff` | `...State.Chassis.Transition.Off` | chassis（`:2902-2905`） |
| `ForceRestart` | `...State.Host.Transition.ForceWarmReboot` | host（`:2907-2911`） |
| `GracefulShutdown` | `...State.Host.Transition.Off` | host（`:2912-2916`） |
| `PowerCycle` | `...State.Host.Transition.Reboot` | host（`:2923-2927`） |
| `Nmi` | 不走属性写入，直接调 NMI 服务的 `NMI` 方法（`redfish-core/lib/systems.hpp:2858-2878`） | — |

得到 command 后，如果是 host 命令，就写 `xyz.openbmc_project.State.Host` 接口的 `RequestedHostTransition` 属性（`redfish-core/lib/systems.hpp:2940-2947`）；如果是 chassis 命令，则写 `xyz.openbmc_project.State.Chassis` 的 `RequestedPowerTransition`（`redfish-core/lib/systems.hpp:2949-2957`）。

> **Java 类比**：`setDbusProperty(...)` 相当于你调用一个远程配置服务的 `setProperty(service, path, iface, key, value)` RPC。而 `RequestedHostTransition` 这个属性名，正是 phosphor-state-manager 监听的「请求字段」——bmcweb 只负责把这个属性写成 `Transition.On`，真正去上电的是 state-manager 订阅。bmcweb 自己不碰 GPIO，也不碰电源芯片。

### 2.4 把 PATCH 也捋一下

顺带看一个改配置的例子，逻辑一样。`PATCH /redfish/v1/Chassis/1/Power/` 命中 `requestRoutesPower()` 里的 PATCH 规则（`redfish-core/lib/power.hpp:341-344`），handler 是 `handleChassisPowerPatch()`（`redfish-core/lib/power.hpp:292-332`）。它先用 `readJsonPatch()` 从请求体里读出 `PowerControl` / `Voltages`（`redfish-core/lib/power.hpp:309-316`），然后：

- 改功率上限：先查 chassis 是否有效（`redfish-core/lib/power.hpp:320-324`），读 D-Bus `PowerCapEnable` 确认功能开着（`redfish-core/lib/power.hpp:105-109`），再 `setDbusProperty()` 写 `xyz.openbmc_project.Control.Power.Cap` 的 `PowerCap`（`redfish-core/lib/power.hpp:63-67`）。
- 改传感器阈值：走 `setSensorsOverride()`（`redfish-core/lib/power.hpp:325-331`）。

对比读操作 `handleChassisPowerGet()`：它先建一个 `SensorsAsyncResp`，再 `getChassisData()` 采集传感器数据（`redfish-core/lib/power.hpp:264-280`），同时用 `getSubTreePaths()` 到 ObjectMapper 查 chassis 对象是否存在（`redfish-core/lib/power.hpp:287-289`），最后用 `getAllProperties()` 读电源上限配置并翻译成 Redfish 的 `PowerLimit`（`redfish-core/lib/power.hpp:254-261`、`redfish-core/lib/power.hpp:112-203`）。

读和写的套路完全一致：**解析请求 → 编译期权限已定 → 异步 D-Bus 调用 → 回调里组装 JSON 或写属性 → AsyncResp 析构发响应。**

### 2.5 把读和写放一张表里看

为了让你以后读任何一条 Redfish 路由都能套上，这里把两类请求的模式抽象成一张对照表（所有结论都能回到上面的 `file:line`）：

| 阶段 | 读（GET） | 写（POST/PATCH） |
| --- | --- | --- |
| 路由权限 | 一般是 `privilegeSetLogin`（`redfish-core/include/registries/privilege_registry.hpp:372`、`:1470`） | 一般是 `privilegeSetConfigure*` / `ConfigureComponents`（`:375`、`:1472`） |
| 入口 handler | `handleComputerSystemGet`（`redfish-core/lib/systems.hpp:3212`） | `handleComputerSystemResetActionPost`（`redfish-core/lib/systems.hpp:2960`）/ `handleChassisPowerPatch`（`redfish-core/lib/power.hpp:292`） |
| 请求体 | 无 | `json_util::readJsonPatch()` / `readJsonObject()` 解析（`redfish-core/lib/power.hpp:309-316`、`redfish-core/lib/systems.hpp:94-97`） |
| D-Bus 动作 | `getProperty` / `getAllProperties` / `getSubTreePaths`（`redfish-core/lib/systems.hpp:460`、`redfish-core/lib/power.hpp:254`、`:287`） | `setDbusProperty`（`redfish-core/lib/systems.hpp:2942`、`redfish-core/lib/power.hpp:63`） |
| 结果落点 | 填 `asyncResp->res.jsonValue[...]`（`redfish-core/lib/systems.hpp:484`） | 回调里 `messages::success()` 或出错时 `messages::internalError()`（`redfish-core/lib/systems.hpp:2875`、`redfish-core/lib/power.hpp:50`） |
| 响应发出 | `AsyncResp` 析构 → `res.end()`（`include/async_resp.hpp:28-31`） | 同左 |

这张表就是你读 bmcweb 任何资源的「套路模板」。看到一个陌生资源的 handler，先找它的 `requestRoutesXxx()` 看注册了哪些动词和权限，再顺着 handler 找它调了哪些 `dbus::utility::*` 函数，最后看回调往 `jsonValue` 里写了什么——三段式，基本通吃。

---

## 3. 真实场景三：读一个传感器 `/redfish/v1/Chassis/1/Sensors/xxx`，看 D-Bus 的动态发现

前两个场景里，D-Bus 的服务名和对象路径基本是写死的（`xyz.openbmc_project.Settings` 之类）。传感器不一样：**装了几个传感器、它们挂在哪个服务下，bmcweb 事先并不知道，要先问 ObjectMapper**。这个场景能让你看到 bmcweb 最典型的「动态发现 + 读属性」套路。

### 3.1 请求链路

```
GET /redfish/v1/Chassis/1/Sensors/<sensorId>/
  │
  ├─① 路由命中：/redfish/v1/Chassis/<str>/Sensors/<str>/
  │     redfish-core/lib/sensors.hpp:2274-2277
  │     → handleSensorGet()   redfish-core/lib/sensors.hpp:2209
  │
  ├─② 用 Mapper 查这个 sensor 路径由哪个服务提供
  │     handleSensorGet() 里 dbus::utility::getDbusObject()
  │     redfish-core/lib/sensors.hpp:2231-2239
  │     → "xyz.openbmc_project.ObjectMapper".GetObject
  │     （实现见 src/dbus_utility.cpp:218-230）
  │
  ├─③ 拿到服务名后读该对象的所有属性
  │     getSensorFromDbus() → dbus::utility::getAllProperties(...)
  │     redfish-core/lib/sensors.hpp:2173-2206
  │
  ├─④ 把 D-Bus 属性翻译成 Redfish Sensor JSON
  │     sensor_utils::objectPropertiesToJson()
  │     redfish-core/lib/sensors.hpp:2203-2205
  │
  └─⑤ AsyncResp 析构 → 响应发出
```

### 3.2 集合接口与单条接口的分工

传感器有两个路由，注册在 `requestRoutesSensorCollection()` 和 `requestRoutesSensor()`（`redfish-core/lib/sensors.hpp:2264-2278`）：

- **集合** `/redfish/v1/Chassis/<str>/Sensors/`：handler 是 `handleSensorCollectionGet()`（`redfish-core/lib/sensors.hpp:2137-2171`）。它先看查询参数里有没有 `$expand`（`redfish-core/lib/sensors.hpp:2142-2154`）：有就走「高效展开」，直接 `getChassisData()` 把每个传感器内联进 JSON（`:2155-2163`）；没有就返回一组超链接（`:2166-2170`）。
- **单条** `/redfish/v1/Chassis/<str>/Sensors/<str>/`：handler 是 `handleSensorGet()`（`redfish-core/lib/sensors.hpp:2209`），走上面那条「动态发现 + 读属性」的路。

### 3.3 为什么要先查 ObjectMapper

这是从 openUBMC 转过来最容易忽略的一点。D-Bus 上「对象路径」和「提供该对象的服务名」是两回事：比如传感器对象路径是 `/xyz/openbmc_project/sensors/temperature/cpu0`，但提供它的服务可能是 `xyz.openbmc_project.Hwmon` 也可能是别的 daemon，取决于实际硬件和构建配置。

所以 bmcweb 的规则是：**先用 ObjectMapper 按路径或接口查服务名，再用服务名去读属性**。`getSensorFromDbus()` 里 `mapperResponse` 必须恰好有 1 项，取它的 `first` 作为 `connectionName`（`redfish-core/lib/sensors.hpp:2178-2185`），然后才 `getAllProperties(*crow::connections::systemBus, connectionName, sensorPath, "", ...)`（`redfish-core/lib/sensors.hpp:2188-2189`）。Mapper 调用本身封装在 `dbus::utility::getSubTree()` / `getSubTreePaths()` / `getDbusObject()` 里（`include/dbus_utility.hpp:154-199`），底层都是对 `xyz.openbmc_project.ObjectMapper` 的 `async_method_call`（实现见 `src/dbus_utility.cpp:78-120`）。

> **Java 类比**：ObjectMapper 就是 D-Bus 世界的「服务注册中心 / 服务发现」（类似 Eureka 或 Consul）。bmcweb 不硬编码服务地址，而是先查「谁提供这个接口」，再发起调用——和你用 `@LoadBalanced RestTemplate` 按服务名调用是一个思路。这也是为什么 OpenBMC 里换一个传感器 daemon，Redfish 接口不用改。

读出来的属性最终靠 `sensor_utils::objectPropertiesToJson()` 转成 Redfish 的 `Reading`、`ReadingUnit` 等字段（`redfish-core/lib/sensors.hpp:2203-2205`）。到这里你又看到了那个熟悉的结尾：`AsyncResp` 的 `jsonValue` 被填好，最后一个回调退出，响应自动发出。

---

## 4. 真实场景四：登录、会话与事件订阅

### 3.1 登录：`POST /login/`

webui-vue 登录时发的是 `POST /login/`，请求体 `{"username":"root","password":"0penBmc"}`。这条路由是匿名可访问的（在白名单里，`include/authentication.hpp:244-246`）。

`handleLogin()` 解析出 username/password（`features/webui_login/login_routes.hpp:32-81`），调 `pamAuthenticateUser()` 校验（`features/webui_login/login_routes.hpp:83-89`），成功后 `SessionStore::generateUserSession()` 生成 session（`features/webui_login/login_routes.hpp:90-93`）：它随机生成 20 字节的 sessionToken 和 csrfToken（`include/sessions.hpp:32`、`include/sessions.hpp:264-272`），把 session 存进 `authTokens` map，并把 token 作为响应字段和 Cookie 返回（`features/webui_login/login_routes.hpp:95-98`）。

之后客户端带上 `X-Auth-Token` 或 Cookie 访问 Redfish，`authenticate()` 里的 `performXtokenAuth()` / `performCookieAuth()` 会查这个 map 恢复出 session（`include/authentication.hpp:119-132`、`include/sessions.hpp:302-320`）。这就是 openUBMC 里「登录拿 token、后续请求带 token」的那套体验，只不过实现换成了 C++。

### 3.2 事件推送：SSE 与 WebSocket

Redfish 客户端可以不轮询，改为订阅事件。有两条路：

**Redfish SSE**：`GET /redfish/v1/EventService/SSE/` 注册成 server-sent event 路由（`redfish-core/lib/eventservice_sse.hpp:88-97`）。连上后 `createSubscription()` 建一个 `Subscription` 挂进 `EventServiceManager`（`redfish-core/lib/eventservice_sse.hpp:26-81`、`redfish-core/lib/eventservice_sse.hpp:76`）。普通事件订阅则用 `POST /redfish/v1/EventService/Subscriptions/`（`redfish-core/lib/event_service.hpp:739-742`）。

**传统 D-Bus 事件 WebSocket**：`/subscribe/` 注册成 websocket 路由（`features/openbmc_rest/dbus_monitor.hpp:263-275`），客户端连上后发送想监听的 D-Bus 路径，服务端用 `sdbusplus::match` 注册 `PropertiesChanged` / `InterfacesAdded` 信号监听（`features/openbmc_rest/dbus_monitor.hpp:250-259`），有变化就往 websocket 推。

> **Java 类比**：SSE 就是 Spring 的 `SseEmitter`，WebSocket 就是 Spring WebSocket / STOMP；`sdbusplus::match` 相当于给 D-Bus 信号注册了一个 listener，事件来了主动 push 给前端，不用前端轮询。

---

### 4.3 把四个场景串成一张图

到现在你已经走完了 bmcweb 最主要的四类业务。把它们叠在一起看，其实是同一套骨架跑了四遍：

```
            ┌─────────────────── 外部世界 ───────────────────┐
  浏览器/webui-vue ──HTTP──▶ bmcweb ──D-Bus──▶ phosphor-* 各服务
            │                    │                    │
   场景1 GET Systems      场景2 POST Reset      state-manager
       读 CurrentHostState    写 RequestedHostTransition
   场景3 GET Sensor        场景4 登录/订阅
       先查 ObjectMapper       SessionStore / SSE / WebSocket
```

- 场景 1 展示了**读**：D-Bus 属性 → Redfish 字段的翻译。
- 场景 2 展示了**写**：Redfish 动作 → D-Bus 属性写入，真正的活由 phosphor-state-manager 干。
- 场景 3 展示了**动态发现**：路径到服务名的映射靠 ObjectMapper，不硬编码。
- 场景 4 展示了**长连接**：session 维持身份，SSE/WebSocket 负责把 D-Bus 事件推出去。

四条路最后都汇到同一个点：`AsyncResp` 析构 → `res.end()` → `completeRequest()` → `doWrite()`（`include/async_resp.hpp:28-31`、`http/http_response.hpp:284-298`、`http/http_connection.hpp:506-519`）。把这一点吃透，你就抓住了 bmcweb 业务的主干，剩下的 1300 多个文件只是这套骨架上挂的不同资源实现。

---

## 结尾：bmcweb 和其它 OpenBMC 组件的分工

把这条链路上的角色摆清楚，你就知道 bmcweb 在整个 OpenBMC 里的位置了：

| 组件 | 干什么（相对 bmcweb） | 在链路里的角色 |
| --- | --- | --- |
| **bmcweb** | HTTP/WebSocket 服务器，D-Bus ↔ Redfish/REST 翻译层 | 本文主角，唯一 HTTP 入口 |
| **phosphor-dbus-interfaces** | 定义 D-Bus 接口和属性名（如 `xyz.openbmc_project.State.Host.CurrentHostState`） | 提供 bmcweb 读写的「字段名标准」，bmcweb 代码里那些字符串就是它定义的 |
| **phosphor-state-manager** | 管理主机/机箱状态，监听 `RequestedHostTransition` 真正执行上电/断电 | bmcweb 写属性，它来干活（对应场景二的 `setDbusProperty`） |
| **dbus-sensors** | 采集温度、电压、风扇等传感器数据，暴露到 D-Bus | bmcweb 的 Sensor/Power/Thermal 资源从这里取数（对应 `getChassisData()`） |
| **phosphor-logging** | 管理事件日志、SEL | bmcweb 的 LogServices 资源从它取日志（对应 `requestRoutesSystemsLogServiceCollection()` 等，`redfish-core/src/redfish.cpp:126`） |
| **ObjectMapper** | 根据接口名查「哪个服务提供哪个 D-Bus 对象」 | bmcweb 用 `getSubTreePaths()` 等查对象路径（`src/dbus_utility.cpp:93-120`） |
| **User.Manager / PAM** | 用户账号、角色、密码校验 | bmcweb 认证鉴权的数据源（`include/dbus_privileges.hpp:183-184`、`features/webui_login/login_routes.hpp:83`） |

一句话记住分工：**phosphor-* 负责产生和维护硬件状态，bmcweb 负责把这些状态翻译成 Redfish/REST 给外部看，并把外部的操作翻译回 D-Bus 写操作。** 它不拥有任何硬件，只是 D-Bus 世界和 HTTP 世界之间那层「翻译 + 门面」。

机制层怎么转（异步回调、trie 路由、D-Bus 封装、编译期裁剪等）见配套的《Bmcweb机制讲解.md》。
