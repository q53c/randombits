# Windows 内核回调与通知接口参考文档

本文档汇总了 Windows 内核中可供驱动程序注册的各类回调（Callback）与通知（Notify Routine）接口。这些接口允许驱动在特定系统事件、硬件中断或对象操作发生时获得执行控制权，且不属于 PatchGuard (KPP) 禁止的 Hook 行为。

## 1. 处理器与异常处理类 (Processor & Exception)

此类接口涉及 CPU 底层异常、中断分发及错误报告，运行环境通常具有极高的 IRQL。

| 接口名称                                | 功能描述                       | 备注                                                         |
| :-------------------------------------- | :----------------------------- | :----------------------------------------------------------- |
| **`KeRegisterNmiCallback`**             | 注册非屏蔽中断 (NMI) 回调。    | 当 CPU 收到 NMI 信号时触发，常用于死锁诊断或硬件错误分析。   |
| **`KeRegisterBoundCallback`**           | 注册 `BOUND` 指令异常回调。    | 拦截用户态执行 `BOUND` 指令触发的异常 (Bound Range Exceeded)。 |
| **`KeRegisterBugCheckCallback`**        | 注册系统蓝屏 (BugCheck) 回调。 | 系统崩溃时触发，用于写入辅助转储数据。运行在 `HIGH_LEVEL` IRQL。 |
| **`KeRegisterBugCheckReasonCallback`**  | 注册带原因的蓝屏回调。         | `KeRegisterBugCheckCallback` 的扩展版本，提供更多崩溃上下文。 |
| **`KeRegisterProcessorChangeCallback`** | 注册处理器变更回调。           | 当系统中有新的处理器 (CPU Core) 被动态添加时触发。           |

## 2. 进程、线程与映像监控类 (Process, Thread & Image)

此类接口属于内核微过滤机制的一部分，广泛应用于安全软件，用于监控系统执行流。

| 接口名称                                | 功能描述                | 备注                                                         |
| :-------------------------------------- | :---------------------- | :----------------------------------------------------------- |
| **`PsSetCreateProcessNotifyRoutineEx`** | 注册进程创建/退出通知。 | 支持在进程创建初期进行拦截 (通过返回错误码)。(包含 `Ex2` 版本) |
| **`PsSetCreateThreadNotifyRoutine`**    | 注册线程创建/退出通知。 | 监控线程的生命周期。(包含 `Ex` 版本)                         |
| **`PsSetLoadImageNotifyRoutine`**       | 注册映像加载通知。      | 当驱动 (.sys)、DLL 或可执行文件映射到内存时触发。(包含 `Ex` 版本) |

## 3. I/O 系统与文件系统类 (I/O & File System)

此类接口涉及 I/O 管理器、驱动加载顺序及文件系统挂载事件。

| 接口名称                                       | 功能描述                     | 备注                                                         |
| :--------------------------------------------- | :--------------------------- | :----------------------------------------------------------- |
| **`IoConnectInterrupt` / `Ex`**                | 注册中断服务例程 (ISR)。     | 将驱动程序 ISR 连接到指定的硬件中断向量。需配合设备对象使用。 |
| **`IoRegisterFsRegistrationChange`**           | 注册文件系统变动通知。       | 当文件系统 (如 NTFS) 激活或注销，或新卷挂载时触发。常用于 Minifilter 附加。 |
| **`IoRegisterBootDriverReinitialization`**     | 注册 Boot 驱动重初始化回调。 | 允许在系统启动早期加载的驱动，在所有设备枚举完成后再次被调用。 |
| **`IoRegisterShutdownNotification`**           | 注册关机/重启通知。          | 在系统关闭文件系统之前触发。                                 |
| **`IoRegisterLastChanceShutdownNotification`** | 注册“最后机会”关机通知。     | 在文件系统刷新并关闭**之后**触发。此时不可进行文件写操作。   |
| **`IoRegisterPlugPlayNotification`**           | 注册即插即用 (PnP) 通知。    | 监控设备接口类的变化 (插入/拔出) 或硬件配置变更。            |

## 4. 对象与注册表监控类 (Object & Registry)

此类接口用于监控内核对象句柄操作及注册表读写行为。

| 接口名称                   | 功能描述             | 备注                                                         |
| :------------------------- | :------------------- | :----------------------------------------------------------- |
| **`CmRegisterCallbackEx`** | 注册注册表操作回调。 | 监控、拦截或修改注册表的创建、读取、写入等操作。             |
| **`ObRegisterCallbacks`**  | 注册对象操作回调。   | 监控对进程 (Process)、线程 (Thread) 和桌面 (Desktop) 对象的句柄创建或复制操作。 |

## 5. 通用系统事件与电源类 (Executive & Power)

基于 `ExRegisterCallback` 的通用回调对象机制及电源管理接口。

| 接口名称                             | 功能描述               | 备注                                                         |
| :----------------------------------- | :--------------------- | :----------------------------------------------------------- |
| **`ExRegisterCallback`**             | 通用回调注册接口。     | 需指定回调对象 (Callback Object)。常见回调对象如下：<br>1. `\Callback\SetSystemTime` (系统时间变更)<br>2. `\Callback\PowerState` (电源状态变更)<br>3. `\Callback\ProcessorAdd` (CPU添加)<br>4. `\Callback\Shutdown` (关机) |
| **`PoRegisterPowerSettingCallback`** | 注册电源设置变更回调。 | 监控具体的电源策略变化 (如屏幕超时、盖子状态等)。            |

## 6. 特殊/底层/未完全文档化接口 (Reserved & HAL)

此类接口通常用于硬件抽象层 (HAL) 或特定的硬件错误架构 (WHEA)，部分属于保留接口或已废弃机制，但在技术上可实现注册。

| 接口名称                                       | 功能描述             | 备注                                                         |
| :--------------------------------------------- | :------------------- | :----------------------------------------------------------- |
| **`HalSetSystemInformation`**                  | 设置 HAL 系统信息。  | 配合 `HalProfileSourceInterruptHandler` 参数，可注册 PMI (性能监控中断) 处理程序。**注意：** 属内部保留接口，存在兼容性风险。 |
| **`WheaRegisterInUsePageOfflineNotification`** | 注册内存页离线通知。 | WHEA 架构的一部分，当系统因 ECC 错误决定停用某物理内存页时触发。 |
| **`HalRegisterErrataCallbacks`**               | 注册硬件勘误回调。   | 用于处理特定 CPU 硬件缺陷 (Errata) 的底层回调。              |
