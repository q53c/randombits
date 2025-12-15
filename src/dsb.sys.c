// self defence tricks learned from qzhddr
//   1. safeboot persistence
//   2. registry callback to protect service
//   3. fs filter to protect .sys file
FLT_REGISTRATION Registration =
{
  104u,
  514u,
  0u,
  NULL,
  &unk_140003000,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
}; // idb
uintptr_t _security_cookie = 47936899621426uLL;
__int64 qword_1400030D0 = -47936899621427LL; // weak
struct _UNICODE_STRING UnicodeString = { 0u, 0u, NULL }; // idb
struct _UNICODE_STRING StringOut_ = { 0u, 0u, NULL }; // idb
__int64 qword_140003100 = 0LL; // weak
LARGE_INTEGER Cookie = { { 0u, 0 } }; // idb
PFLT_FILTER Filter = NULL; // idb
char byte_140003118 = '\0'; // weak


//----- (0000000140001000) ----------------------------------------------------
__int64 __fastcall FltOperationCallback(PFLT_CALLBACK_DATA CallbackData, __int64 a2)
{
  PFLT_IO_PARAMETER_BLOCK Iopb; // rax
  UCHAR MajorFunction; // cl
  bool v6; // zf
  PFLT_FILE_NAME_INFORMATION FileNameInformation_1; // rax
  __int64 v8; // rdx
  UNICODE_STRING Name; // [rsp+20h] [rbp-18h] BYREF
  unsigned __int8 IsDirectory; // [rsp+48h] [rbp+10h] BYREF
  PFLT_FILE_NAME_INFORMATION FileNameInformation; // [rsp+58h] [rbp+20h] BYREF

  if ( FltIsDirectory(*(PFILE_OBJECT *)(a2 + 32), *(PFLT_INSTANCE *)(a2 + 24), &IsDirectory) >= 0 && IsDirectory )
    return 1LL;
  Iopb = CallbackData->Iopb;
  MajorFunction = Iopb->MajorFunction;
  if ( MajorFunction )
  {
    if ( MajorFunction == IRP_MJ_SET_INFORMATION )
    {
      switch ( Iopb->Parameters.SetFileInformation.FileInformationClass )
      {
        case 0xA:
        case 0xD:
        case 0x38:
        case 0x40:
        case 0x41:
        case 0x42:
          break;
        default:
          return 1LL;
      }
    }
  }
  else if ( (Iopb->Parameters.Create.Options & 0x1000) == 0 )
  {
    return 1LL;
  }
  v6 = *(_QWORD *)(a2 + 32) == 0LL;
  FileNameInformation = 0LL;
  if ( v6 )
  {
    DbgPrint("[ERROR] FltObjects->FileObject is NULL!\n");
  }
  else if ( FltGetFileNameInformation(CallbackData, 0x101u, &FileNameInformation) < 0 )
  {
    DbgPrint("[ERROR] Failed to get file name information!\n");
  }
  else
  {
    FltParseFileNameInformation(FileNameInformation);
    if ( qword_140003100 == *(_QWORD *)(*(_QWORD *)(a2 + 32) + 24LL) )
    {
      FileNameInformation_1 = FileNameInformation;
      CallbackData->IoStatus.Status = -1073741790;
      CallbackData->IoStatus.Information = 0LL;
      v8 = *(_QWORD *)(*(_QWORD *)(a2 + 32) + 24LL);
      Name = FileNameInformation_1->Name;
      DbgPrint("[DENIED] (%p) %wZ\n", v8, &Name);
      return 4LL;
    }
  }
  return 1LL;
}
//----- (00000001400011E0) ----------------------------------------------------
NTSTATUS InitRegistryForFlt()
{
  NTSTATUS v0; // eax
  void *KeyHandle_1; // rbx
  NTSTATUS v2; // eax
  HANDLE KeyHandle__1; // rbx
  NTSTATUS v4; // eax
  NTSTATUS result; // eax
  struct _UNICODE_STRING DestinationString; // [rsp+40h] [rbp-19h] BYREF
  struct _UNICODE_STRING ValueName; // [rsp+50h] [rbp-9h] BYREF
  _OBJECT_ATTRIBUTES ObjectAttributes; // [rsp+60h] [rbp+7h] BYREF
  int Data; // [rsp+C0h] [rbp+67h] BYREF
  HANDLE Handle; // [rsp+C8h] [rbp+6Fh] BYREF
  HANDLE KeyHandle_; // [rsp+D0h] [rbp+77h] BYREF
  void *KeyHandle; // [rsp+D8h] [rbp+7Fh] BYREF

  KeyHandle = 0LL;
  RtlInitUnicodeString(&DestinationString, UnicodeString.Buffer);
  ObjectAttributes.ObjectName = &DestinationString;
  ObjectAttributes.Length = 48;
  ObjectAttributes.RootDirectory = 0LL;
  ObjectAttributes.Attributes = 576;
  *(_OWORD *)&ObjectAttributes.SecurityDescriptor = 0LL;
  v0 = ZwCreateKey(&KeyHandle, 0x2001Fu, &ObjectAttributes, 0, 0LL, 0, 0LL);
  if ( v0 < 0 )
    DbgPrint("[-] Failed to open key(%wZ): %x\n", &DestinationString, (unsigned int)v0);
  KeyHandle_1 = KeyHandle;
  KeyHandle_ = 0LL;
  RtlInitUnicodeString(&DestinationString, L"Instances");
  ObjectAttributes.ObjectName = &DestinationString;
  ObjectAttributes.Length = 48;
  ObjectAttributes.RootDirectory = KeyHandle_1;
  ObjectAttributes.Attributes = 576;
  *(_OWORD *)&ObjectAttributes.SecurityDescriptor = 0LL;
  v2 = ZwCreateKey(&KeyHandle_, 0x2001Fu, &ObjectAttributes, 0, 0LL, 0, 0LL);
  if ( v2 < 0 )
    DbgPrint("[-] Failed to open key(%wZ): %x\n", &DestinationString, (unsigned int)v2);
  RtlInitUnicodeString(&DestinationString, L"DefaultInstance");
  ZwSetValueKey(KeyHandle_, &DestinationString, 0, 1u, (PVOID)L"inst0", 0xCu);
  KeyHandle__1 = KeyHandle_;
  Handle = 0LL;
  RtlInitUnicodeString(&DestinationString, L"inst0");
  ObjectAttributes.ObjectName = &DestinationString;
  ObjectAttributes.Length = 48;
  ObjectAttributes.RootDirectory = KeyHandle__1;
  ObjectAttributes.Attributes = 576;
  *(_OWORD *)&ObjectAttributes.SecurityDescriptor = 0LL;
  v4 = ZwCreateKey(&Handle, 0x2001Fu, &ObjectAttributes, 0, 0LL, 0, 0LL);
  if ( v4 < 0 )
    DbgPrint("[-] Failed to open key(%wZ): %x\n", &DestinationString, (unsigned int)v4);
  RtlInitUnicodeString(&DestinationString, L"12306");
  RtlInitUnicodeString(&ValueName, L"Altitude");
  ZwSetValueKey(Handle, &ValueName, 0, 1u, DestinationString.Buffer, DestinationString.Length);
  Data = 0;
  RtlInitUnicodeString(&ValueName, L"Flags");
  result = ZwSetValueKey(Handle, &ValueName, 0, 4u, &Data, 4u);
  if ( Handle )
    result = ZwClose(Handle);
  if ( KeyHandle_ )
    result = ZwClose(KeyHandle_);
  if ( KeyHandle )
    return ZwClose(KeyHandle);
  return result;
}

//----- (0000000140001450) ----------------------------------------------------
__int64 __fastcall RegCallback(PVOID CallbackContext, PVOID Argument1, void **Argument2)
{
  unsigned int v3; // ebx
  void *Object; // rdx
  PCUNICODE_STRING ObjectName; // [rsp+48h] [rbp+18h] BYREF

  v3 = 0;
  switch ( (int)Argument1 )
  {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 38:
      Object = *Argument2;
      goto LABEL_4;
    case 26:
      Object = Argument2[1];
LABEL_4:
      ObjectName = 0LL;
      if ( CmCallbackGetKeyObjectID(&Cookie, Object, 0LL, &ObjectName) >= 0
        && ObjectName->Length >= UnicodeString.Length
        && !RtlCompareUnicodeStrings(
              ObjectName->Buffer,
              (unsigned __int64)UnicodeString.Length >> 1,
              UnicodeString.Buffer,
              (unsigned __int64)UnicodeString.Length >> 1,
              1u) )
      {
        DbgPrint("[-] Protected registry path: %wZ\n", ObjectName);
        v3 = -1073741790;
      }
      break;
    default:
      return v3;
  }
  return v3;
}

//----- (0000000140001540) ----------------------------------------------------
int __fastcall DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
  struct_DriverSection *DriverSection; // r8
  NTSTATUS v4; // eax
  NTSTATUS v5; // eax
  int v6; // ebx
  NTSTATUS v7; // eax
  int result; // eax
  struct _UNICODE_STRING DestinationString; // [rsp+40h] [rbp-9h] BYREF
  UNICODE_STRING Altitude; // [rsp+50h] [rbp+7h] BYREF
  struct _OBJECT_ATTRIBUTES ObjectAttributes; // [rsp+60h] [rbp+17h] BYREF
  int Data; // [rsp+B0h] [rbp+67h] BYREF
  void *KeyHandle; // [rsp+C0h] [rbp+77h] BYREF

  RtlDuplicateUnicodeString(2u, RegistryPath, &UnicodeString);
  DriverSection = (struct_DriverSection *)DriverObject->DriverSection;
  qword_140003100 = *(_QWORD *)((*(_QWORD *)(*(_QWORD *)(DriverSection->qword70 + 40LL) + 64LL) & 0xFFFFFFFFFFFFFFF0uLL)
                              + 0x18);
  RtlDuplicateUnicodeString(2u, &DriverSection->unicode_string48, &StringOut_);
  ObjectAttributes.Length = 48;
  KeyHandle = 0LL;
  ObjectAttributes.RootDirectory = 0LL;
  ObjectAttributes.Attributes = 576;
  ObjectAttributes.ObjectName = &UnicodeString;
  *(_OWORD *)&ObjectAttributes.SecurityDescriptor = 0LL;
  v4 = ZwCreateKey(&KeyHandle, 0x2001Fu, &ObjectAttributes, 0, 0LL, 0, 0LL);
  if ( v4 < 0 )
    DbgPrint("[-] Failed to open key(%wZ): %x\n", &UnicodeString, (unsigned int)v4);
  Data = 1;
  RtlInitUnicodeString(&DestinationString, L"Start");
  ZwSetValueKey(KeyHandle, &DestinationString, 0, 4u, &Data, 4u);
  Data = 2;
  RtlInitUnicodeString(&DestinationString, L"Type");
  ZwSetValueKey(KeyHandle, &DestinationString, 0, 4u, &Data, 4u);
  RtlInitUnicodeString(&DestinationString, L"Group");
  ZwSetValueKey(KeyHandle, &DestinationString, 0, 1u, L"System Bus Extender", 0x28u);
  if ( KeyHandle )
    ZwClose(KeyHandle);
  InitRegistryForFlt();
  RtlInitUnicodeString(&Altitude, L"12306");
  v5 = CmRegisterCallbackEx((PEX_CALLBACK_FUNCTION)RegCallback, &Altitude, DriverObject, 0LL, &Cookie, 0LL);
  v6 = v5;
  if ( v5 >= 0 )
  {
    v7 = FltRegisterFilter(DriverObject, &Registration, &Filter);
    v6 = v7;
    if ( v7 >= 0 )
    {
      result = FltStartFiltering(Filter);
      v6 = result;
      if ( result >= 0 )
      {
        byte_140003118 = 1;
        return result;
      }
      DbgPrint("[-] FltStartFiltering failed: %08x\n", (unsigned int)result);
    }
    else
    {
      DbgPrint("[-] FltRegisterFilter failed: %08x\n", (unsigned int)v7);
    }
  }
  else
  {
    DbgPrint("[-] Register callback failed: %08x\n", (unsigned int)v5);
  }
  if ( Filter )
  {
    FltUnregisterFilter(Filter);
    Filter = 0LL;
  }
  if ( Cookie.QuadPart )
  {
    CmUnRegisterCallback(Cookie);
    Cookie.QuadPart = 0LL;
  }
  RtlFreeUnicodeString(&UnicodeString);
  return v6;
}
