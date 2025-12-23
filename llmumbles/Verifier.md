要关闭 Windows 的驱动程序验证程序 (Driver Verifier)，最直接的方法是使用命令提示符（管理员权限）输入 `verifier /reset`，然后重启电脑；若无法启动系统，则需通过高级启动进入恢复环境，使用注册表编辑器删除 `HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Memory Management` 下的 `VerifyDrivers` 和 `VerifyDriverLevel` 键值。 

------

方法一：使用命令提示符 (推荐，若系统能正常启动)

1. **以管理员身份打开命令提示符**:
   - 在任务栏搜索框输入 "cmd"。
   - 右键点击“命令提示符”，选择“以管理员身份运行”。
2. **输入命令关闭验证**:
   - 在命令行中输入：`verifier /reset`
   - 按 Enter 键执行。
3. **重启电脑**: 命令会提示你在下次重启时禁用验证，输入 `Y` 确认并重启。 

方法二：通过安全模式或恢复环境 (如果系统无法启动)

如果系统因驱动程序验证导致蓝屏无法启动，可以尝试：

1. **进入 Windows 恢复环境 (WinRE)**：
   - 在登录界面按住 `Shift` 键，同时点击“电源” > “重启”。
   - 或者，连续强制关机/重启两次，系统会进入恢复环境。
2. **选择“疑难解答”** > **“高级选项”** > **“命令提示符”**。
3. **加载注册表并删除键值** (此步骤需要对注册表有一定了解):
   - 输入 `regedit` 打开注册表编辑器。
   - 选择 `HKEY_LOCAL_MACHINE`。
   - 点击“文件” > “加载 Hive”，选择目标系统盘的 `\Windows\System32\config\SYSTEM` 文件。
   - 加载后会出现在 `HKEY_LOCAL_MACHINE` 下，给它起个名字（例如 "SYSTEM"）。
   - 导航至 `HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Session Manager\Memory Management`。
   - 找到并删除 `VerifyDrivers` 和 `VerifyDriverLevel` 两个键值。
   - 选中你加载的 "SYSTEM" 分支，点击“文件” > “卸载 Hive”。
4. **重启电脑**。 

**小提示**: `verifier` 命令是 Windows 内置的强大的驱动程序调试工具，用于帮助开发者查找驱动问题，一般用户不需开启。`verifier /reset` 命令是将其恢复到默认关闭状态。 
