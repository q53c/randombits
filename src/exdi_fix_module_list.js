"use strict";

// Iterate nt!PsLoadedModuleList by raw memory reads (x64).
// Prints: entry, DllBase, SizeOfImage, FullDllName, and a reload template line.
//
// Usage:
//   .scriptload C:\\temp\\exdi_fix_module_list.js
//   dx Debugger.State.Scripts.exdi_fix_module_list.Contents.fix()

function I64(x) { return host.Int64(x); }

function add(addr, off) { return I64(addr).add(I64(off)); }

function eq(a, b) { return I64(a).compareTo(I64(b)) === 0; }

function hex(a) { a = I64(a); return "0x" + a.toString(16); }

function poi(x) { return host.memory.readMemoryValues(I64(x), 1, 8)[0]; }

function readUnicodeString(address) {
    // 1. 将地址转换为指向 UNICODE_STRING 的对象
    // 注意：如果是内核调试，模块名通常为 "nt"，用户态一般为 "ntdll"
    var ustr = host.createTypedObject(I64(address), "nt", "_UNICODE_STRING");

    // 2. 检查 Buffer 是否为空
    if (ustr.Buffer.isNull) {
        return "(null)";
    }

    // 3. 使用 readWideString 读取 Buffer 处的内容
    // UNICODE_STRING.Length 以字节为单位，readWideString 需要字符数，所以除以 2
    var charCount = ustr.Length / 2;
    var result = host.memory.readWideString(ustr.Buffer, charCount);

    return result;
}

function fix(options) {
    const LOG = host.diagnostics.debugLog
    const _KLDR_DATA_TABLE_ENTRY = host.getModuleType("nt", "_KLDR_DATA_TABLE_ENTRY");
    const PsLoadedModuleList = host.getModuleSymbolAddress("nt", "PsLoadedModuleList");
    
    LOG("PsLoadedModuleList: " + hex(PsLoadedModuleList) + "\n")
    LOG("  _KLDR_DATA_TABLE_ENTRY.DllBase = " + hex(_KLDR_DATA_TABLE_ENTRY.fields.DllBase.offset) + "\n")
    LOG("  _KLDR_DATA_TABLE_ENTRY.BaseDllName = " + hex(_KLDR_DATA_TABLE_ENTRY.fields.BaseDllName.offset) + "\n")
    
    const O_DllBase = _KLDR_DATA_TABLE_ENTRY.fields.DllBase.offset
    const O_BaseDllName = _KLDR_DATA_TABLE_ENTRY.fields.BaseDllName.offset
    const O_SizeOfImage = _KLDR_DATA_TABLE_ENTRY.fields.SizeOfImage.offset
    
    var cur = poi(PsLoadedModuleList);
    while(hex(cur) != hex(PsLoadedModuleList)){
      var kldr_dt_entry = host.createTypedObject(I64(cur), "nt", "_KLDR_DATA_TABLE_ENTRY");
      var basename_noquote = String(kldr_dt_entry.BaseDllName).replace(/"/g, "")
      LOG("[EXDI_LM_FIX] .reload " + basename_noquote + "=" + kldr_dt_entry.DllBase.address.toString(16) + "," + hex(kldr_dt_entry.SizeOfImage) + "\n")
      // LOG(hex(cur) + " ## .reload  "+ readUnicodeString(add(cur, 0x58)) "=" +  hex(poi(cur.add(O_DllBase))) + ",todo" + "\n")
      
      cur = poi(cur)
    }
}

function initializeScript() {
    return [
        new host.apiVersionSupport(1, 7),
        new host.functionAlias(fix, "fix")
        
    ];
}
