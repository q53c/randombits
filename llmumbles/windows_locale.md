# Windows 语言环境修复指南

你的问题很可能是由于语言设置、区域设置和系统 Locale 不匹配导致的。下面是系统的修复步骤：

## 1. 检查当前配置状态

首先在 PowerShell（管理员权限）中运行以下命令查看当前配置：

```powershell
Get-WinSystemLocale
Get-WinUserLanguageList
Get-Culture
Get-WinUILanguageOverride
```

## 2. 重置为简体中文环境

### 方法一：通过 PowerShell 修复（推荐）

以管理员身份运行 PowerShell，依次执行：

```powershell
# 设置系统区域设置（非 Unicode 程序语言）
Set-WinSystemLocale -SystemLocale zh-CN

# 设置用户语言列表
Set-WinUserLanguageList -LanguageList zh-CN -Force

# 设置 UI 语言
Set-WinUILanguageOverride -Language zh-CN

# 设置当前用户的区域格式
Set-Culture -CultureInfo zh-CN
```

执行后**重启电脑**。

### 方法二：通过图形界面修复

1. **打开区域设置**
   - 按 `Win + R`，输入 `intl.cpl`，回车

2. **管理选项卡**
   - 点击"更改系统区域设置"
   - 选择"中文(简体，中国)"
   - ✅ 勾选"Beta: 使用 Unicode UTF-8 提供全球语言支持"（如果之前勾选了）
   - 点击确定

3. **复制设置**
   - 在"管理"选项卡，点击"复制设置"
   - ✅ 勾选"欢迎屏幕和系统账户"
   - ✅ 勾选"新用户账户"
   - 点击确定

4. **格式选项卡**
   - 确保格式设置为"中文(简体，中国)"

5. **重启电脑**

## 3. 修复字体显示问题

如果界面显示问号，可能是字体缺失：

```powershell
# 检查字体注册表
Get-ItemProperty "HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts" | Select-Object "Microsoft YaHei*"
```

### 重新安装系统字体

1. 打开 `C:\Windows\Fonts`
2. 确认以下字体存在：
   - Microsoft YaHei (微软雅黑)
   - SimSun (宋体)
   - SimHei (黑体)

3. 如果缺失，从正常的 Windows 系统复制，或运行：

```powershell
# 运行系统文件检查
sfc /scannow
```

## 4. 清除语言缓存

```powershell
# 停止相关服务
Stop-Service -Name "BITS" -Force
Stop-Service -Name "wuauserv" -Force

# 清除缓存
Remove-Item -Path "$env:LOCALAPPDATA\Microsoft\Windows\INetCache\*" -Recurse -Force -ErrorAction SilentlyContinue

# 重启服务
Start-Service -Name "BITS"
Start-Service -Name "wuauserv"
```

## 5. 检查 Windows 更新

确保语言包正确安装：

1. 设置 → 时间和语言 → 语言和区域
2. 点击"中文(简体，中国)"旁边的 `...` → 语言选项
3. 下载所有可选功能（特别是"语言显示"）

## 6. 如果问题依然存在

### 高级修复：重置区域设置注册表

**⚠️ 修改注册表有风险，请先备份！**

```powershell
# 备份注册表
reg export "HKEY_CURRENT_USER\Control Panel\International" "$env:USERPROFILE\Desktop\intl_backup.reg"

# 重置为默认中文值
reg add "HKEY_CURRENT_USER\Control Panel\International" /v Locale /t REG_SZ /d 00000804 /f
reg add "HKEY_CURRENT_USER\Control Panel\International" /v LocaleName /t REG_SZ /d zh-CN /f
```

重启电脑后查看效果。

---

## 常见问题诊断

- **问号显示**：通常是系统 Locale（非 Unicode 程序语言）未设为中文
- **部分 UI 英文**：UI 语言覆盖设置问题
- **日期/时间格式异常**：Culture 设置问题

请先尝试方法一，执行完后重启。如果还有问题，告诉我具体哪些地方显示问号，我可以提供更针对性的解决方案。
