
#pragma once
#include "CoreMinimal.h"
#include "soci/soci.h"
#include "SociSetting.generated.h"

//https://soci.sourceforge.net/doc/release/4.0
///相关文档
UENUM()
enum class ESocil_SQLType :uint8
{
	ENone,
	EOdbc,//微软
	EMySql,//mysql
	EDB2,
	EFirebird,
	Oracle,
	PostgreSQL,
	SQLite3,
	Max
};
ENUM_CLASS_FLAGS(ESocil_SQLType);

UENUM()
enum class FSoci_ErrorType:uint8
{
	ENone,
	EQuery,
	EInsert,
	EDelete,
	EUpdate,
	EUnKnowns,
};


USTRUCT()
struct FSoci_Error
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FSoci_ErrorType ErrorType;
	UPROPERTY()
	FString   ErrorMsg;
	FSoci_Error():ErrorType(FSoci_ErrorType::ENone)
	{
		
	}
	bool HasError()
	{
		return ErrorType != FSoci_ErrorType::ENone;
	}
};


//数据库连接状态
UENUM()
enum class ESQLSubsys_State
{
	ENone,//最开始状态
	EConnecting,//连接中
	EConnected,//连接上
	ERetry,//失败
	EFail,//失败
};
ENUM_CLASS_FLAGS(ESQLSubsys_State);



USTRUCT()
struct FSociDefinition
{
	GENERATED_BODY()
	UPROPERTY( meta=(DisplayName = "定义名"))
	FName DefName;
	UPROPERTY( meta=(DisplayName = "连接字符串"))
	FString ConString;
	UPROPERTY()
	bool HasLog;
	UPROPERTY()
	ESocil_SQLType SQLType;

	
	
	FSociDefinition():
	DefName(NAME_None),
	HasLog(false),
	SQLType(ESocil_SQLType::ENone)
	{
		
	}
};

/*
USTRUCT()
struct FSQL_Float : public FSQLDataType
{
	GENERATED_BODY()
public:
	
};*/
/*
template<typename T>
struct SQL_Function
{
public:
	TUniquePtr<T> pt;
	TUniqueFunction<T> fun;
};*/
//tick刷新间隔
#define SQLSubSys_Tick_Rate 5.f
//重试次数
#define SQLSubSys_Retry_Nub 5
//超时时间
#define SQLSubSys_TimeOut 60.f
/*
UCLASS(Config=Game,Defaultconfig)
class TC_SQL_API USociSetting : public UObject
{
	GENERATED_BODY()
public:
	//定义群
	UPROPERTY(Config,transient)
	TArray<FSociDefinition> SociDefinitions;
};
/*

#define SOCI_DECLARE_ROW(UeType) \
namespace soci\
 {\
	template<>\
	struct type_conversion<UeType>\
	{\
		typedef values base_type;\
		static void from_base(values const & v, indicator  ind , UeType & p)\
		{\
			for (TFieldIterator<FProperty>  it(UeType::StaticStruct()); it;++it )\
			{\
				auto key=it->GetNameCPP();\
				if (it->IsA(FStrProperty::StaticClass()) )\
				{\
					FString str = UTF8_TO_TCHAR(v.get<std::string>(TCHAR_TO_UTF8(*key)).c_str());\
					it->SetValue_InContainer(&p,&str);\
				}\
				else if (it->IsA(FIntProperty::StaticClass()))\
				{\
					int32   i32 = v.get<int32>(TCHAR_TO_UTF8(*key));\
					it->SetValue_InContainer(&p,&i32);\
				}\
			}\
		}\
		static void to_base(const FPerson & p, values & v, indicator & ind)\
		{\
			for (TFieldIterator<FProperty>  it(UeType::StaticStruct()); it;++it )\
			{\
				auto key=it->GetNameCPP();\
				if (it->IsA(FStrProperty::StaticClass()) )\
				{\
					FString str ;\
					it->GetValue_InContainer(&p,&str);\
					v.set(TCHAR_TO_UTF8(*key), std::string( TCHAR_TO_UTF8(*str)) );\
				}\
				else if (it->IsA(FIntProperty::StaticClass()))\
				{\
					int32   i32;\
					it->GetValue_InContainer(&p,&i32);\
					v.set(TCHAR_TO_UTF8(*key),i32);\
				}\
			}\
			ind = i_ok;\
		}\
	};\
}*/