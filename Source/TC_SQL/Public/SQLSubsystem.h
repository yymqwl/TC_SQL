// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "soci/soci.h"
#include "soci/odbc/soci-odbc.h"//放第一个不然会有歧义
#include "CoreMinimal.h"
#include "SociLog.h"
#include "SociSetting.h"
#include "SQLTableWrapper.h"
#include "SQLSubsystem.generated.h"

class USociSubsystem;
/**
 * 依附于USociSubsystem系统之下
 *
*/

//DECLARE_DELEGATE_OneParam()

UCLASS()
class TC_SQL_API USQLSubsystem : public UObject
{
	GENERATED_BODY()
public:
	USQLSubsystem(){};
	//UASQLSubsystem(const FSociDefinition* sociDefinition);

	virtual void Initialize(const FSociDefinition* sociDefinition);

	/** Implement this for deinitialization of instances of the system */
	virtual void Deinitialize();

	UFUNCTION()
	virtual void Open();
	UFUNCTION()
	virtual  bool IsConnected();
	UFUNCTION()
	virtual void Close();

	/*
	template<typename T>
	FString GetInsertSQLStr()
	{
		//check(str_tb.IsEmpty());
	    FString	str_tb = T::StaticStruct()->GetName();
		FString str1 = FString::Format(TEXT("insert into {0}("),{*str_tb});
		FString str2 = FString(TEXT("values("));
		for (TFieldIterator<FProperty>  it(T::StaticStruct()); it;++it )
		{
			auto key=it->GetNameCPP();
			str1 += FString::Format(TEXT("{0},"),{*key});
			str2 += FString::Format(TEXT(":{0},"),{*key}); //TEXT(":")+TCHAR_TO_UTF8(*key)+""
		}
		str1.LeftInline(str1.Len()-1);
		str2.LeftInline(str2.Len()-1);
		
		FString str = FString::Format(TEXT("{0}) {1})"),{*str1,*str2});
		//FString::Left()
		return str;
	}*/

	//多线程查询,主线程回调
	template<typename T ,typename = typename TEnableIf<TIsDerivedFrom<T,FSQLRow>::Value>::Type>
	void Query(const FString& sql,TFunction<void(TSharedRef< SQLTableWrapper<T> >,TSharedRef<FSoci_Error> error)> fun)
	{
		TSharedRef<SQLTableWrapper<T>>  SqlTable =  MakeShareable(new SQLTableWrapper<T>());
		TSharedRef<FSoci_Error> p_error = MakeShareable(new FSoci_Error);
		//这里用引用传值大概率会报错,变量大部分都是局部
		auto get_thread = FFunctionGraphTask::CreateAndDispatchWhenReady([this,sql,SqlTable,p_error]()
		{
			try
			{
				auto str_sql = std::string( TCHAR_TO_UTF8(*sql));
				
				FScopeLock SL(&this->SQL_CS);
				
				soci::rowset<soci::row> rs=(this->Sql_Session.prepare << str_sql );
				//Sql_Session << str_sql, soci::into(r);
				
				//SOCI_LOG(TEXT("Query Size: %d"), r.size());
				for (auto it = rs.begin(); it != rs.end(); ++it)
				{
					const soci::row& r = (*it);
					
					TSharedRef<SQLRowWrapper<T>> p_row = MakeShareable(new SQLRowWrapper<T>);
					SqlTable->Rows.Add(p_row);

					
					for(std::size_t i = 0; i != r.size(); ++i)
					{
						const soci::column_properties& props = r.get_properties(i);
						//props.get_name();
						FName key = FName( UTF8_TO_TCHAR(props.get_name().c_str()));
						switch (props.get_data_type())
						{
							case soci::dt_string://
								{
									FString str= FString( UTF8_TO_TCHAR( r.get<std::string>(i).c_str()) );
									p_row->SetValue(key,&str);
									break;
								}
							case soci::dt_integer:
								{
									int32 i32 = r.get<int>(i);
									p_row->SetValue(key,&i32);
									break;
								}
							default:TC_SQL_ERROR(TEXT("Not Suppert Type %d"),(int32)props.get_data_type());break;
						}
					}
				}
				this->UpdateActiveTime();
				//SOCI_LOG(TEXT("query success"));
			}
			catch (std::runtime_error const& e)
			{
				p_error->ErrorType = FSoci_ErrorType::EQuery;
				p_error->ErrorMsg = UTF8_TO_TCHAR(e.what());
				TC_SQL_ERROR(TEXT("Query Error: %s"), *p_error->ErrorMsg);
			}
			catch (std::exception e)
			{
				p_error->ErrorType = FSoci_ErrorType::EUnKnowns;
				p_error->ErrorMsg = UTF8_TO_TCHAR(e.what());
				TC_SQL_ERROR(TEXT("Query Error:%s"),*p_error->ErrorMsg);
			}
		});
		
		FGraphEventArray Tasks;
		Tasks.Add(get_thread);
		FFunctionGraphTask::CreateAndDispatchWhenReady([this,fun,SqlTable,p_error]()
		{
			fun(SqlTable,p_error );
		},TStatId{},&Tasks,ENamedThreads::GameThread);
		
	}
	//"insert into person(Id, Name) values(:Id, :Name)"//标准输入
	//每次查一个数据，暂时不做批量操作
	template<typename T ,typename = typename TEnableIf<TIsDerivedFrom<T,FSQLRow>::Value>::Type>
	void Insert(TSharedRef<SQLTableWrapper<T>>  ptr_SqlTable,TFunction<void(TSharedRef<SQLTableWrapper<T>>,TSharedRef<FSoci_Error> error)> fun)
	{
		TSharedRef<FSoci_Error> p_error = MakeShareable(new FSoci_Error);
		auto get_thread = FFunctionGraphTask::CreateAndDispatchWhenReady([this,ptr_SqlTable,p_error]()
		{
			try
			{
				//auto sql = TCHAR_TO_UTF8(*sql_tb);
				FScopeLock SL(&this->SQL_CS);
				for (auto pt_row : ptr_SqlTable->Rows)
				{
					FString sql_str = ptr_SqlTable->GetInsertSQLString(pt_row);
					//SOCI_LOG(TEXT("%s"),*sql_str);
					//std::string sql_str = std::string( UTF8_TO_TCHAR(ptr_SqlTable->GetInsertSQLString()));
					this->Sql_Session << std::string( TCHAR_TO_UTF8( *sql_str ));//以utf8格式插入
				}
				this->UpdateActiveTime();
				//SOCI_LOG(TEXT("query success"));
			}
			catch (std::runtime_error const& e)
			{
				p_error->ErrorType = FSoci_ErrorType::EInsert;
				p_error->ErrorMsg = UTF8_TO_TCHAR(e.what());
				TC_SQL_ERROR(TEXT("Insert Error: %s"), *p_error->ErrorMsg);
			}
			catch (std::exception e)
			{
				p_error->ErrorType = FSoci_ErrorType::EUnKnowns;
				p_error->ErrorMsg = UTF8_TO_TCHAR(e.what());
				TC_SQL_ERROR(TEXT("Insert Error:%s"),*p_error->ErrorMsg);
			}
		});
		FGraphEventArray Tasks;
		Tasks.Add(get_thread);
		FFunctionGraphTask::CreateAndDispatchWhenReady([this,fun,ptr_SqlTable,p_error]()
		{
			fun(ptr_SqlTable,p_error );
		},TStatId{},&Tasks,ENamedThreads::GameThread);
	}
	///删除功能
	template<typename T ,typename = typename TEnableIf<TIsDerivedFrom<T,FSQLRow>::Value>::Type>
	void Delete(TSharedRef<SQLTableWrapper<T>>  ptr_SqlTable,TFunction<void(TSharedRef<SQLTableWrapper<T>>,TSharedRef<FSoci_Error> error)> fun)
	{
		TSharedRef<FSoci_Error> p_error = MakeShareable(new FSoci_Error);
		auto get_thread = FFunctionGraphTask::CreateAndDispatchWhenReady([this,ptr_SqlTable,p_error]()
		{
			try
			{
				FScopeLock SL(&this->SQL_CS);
				for (auto pt_row : ptr_SqlTable->Rows)
				{
					FString sql_str = ptr_SqlTable->GetDeleteSQLString(pt_row);
					//SOCI_LOG(TEXT("delete %s"),*sql_str);
					this->Sql_Session << std::string( TCHAR_TO_UTF8( *sql_str ));//以utf8格式插入
				}
				this->UpdateActiveTime();
				//SOCI_LOG(TEXT("query success"));
			}
			catch (std::runtime_error const& e)
			{
				p_error->ErrorType = FSoci_ErrorType::EDelete;
				p_error->ErrorMsg = UTF8_TO_TCHAR(e.what());
				TC_SQL_ERROR(TEXT("Delete Error: %s"), *p_error->ErrorMsg);
			}
			catch (std::exception e)
			{
				p_error->ErrorType = FSoci_ErrorType::EUnKnowns;
				p_error->ErrorMsg = UTF8_TO_TCHAR(e.what());
				TC_SQL_ERROR(TEXT("Delete Error:%s"),*p_error->ErrorMsg);
			}
		});
		FGraphEventArray Tasks;
		Tasks.Add(get_thread);
		FFunctionGraphTask::CreateAndDispatchWhenReady([this,fun,ptr_SqlTable,p_error]()
		{
			fun(ptr_SqlTable,p_error );
		},TStatId{},&Tasks,ENamedThreads::GameThread);
	}
	
	///修改功能
	template<typename T ,typename = typename TEnableIf<TIsDerivedFrom<T,FSQLRow>::Value>::Type>
	void Update(TSharedRef<SQLTableWrapper<T>>  ptr_SqlTable,TFunction<void(TSharedRef<SQLTableWrapper<T>>,TSharedRef<FSoci_Error> error)> fun)
	{
		TSharedRef<FSoci_Error> p_error = MakeShareable(new FSoci_Error);
		auto get_thread = FFunctionGraphTask::CreateAndDispatchWhenReady([this,ptr_SqlTable,p_error]()
		{
			try
			{
				FScopeLock SL(&this->SQL_CS);
				for (auto pt_row : ptr_SqlTable->Rows)
				{
					FString sql_str = ptr_SqlTable->GetUpdateSQLString(pt_row);
					//SOCI_LOG(TEXT("update %s"),*sql_str);
					this->Sql_Session << std::string( TCHAR_TO_UTF8( *sql_str ));//以utf8格式插入
				}
				this->UpdateActiveTime();
				//SOCI_LOG(TEXT("query success"));
			}
			catch (std::runtime_error const& e)
			{
				p_error->ErrorType = FSoci_ErrorType::EDelete;
				p_error->ErrorMsg = UTF8_TO_TCHAR(e.what());
				TC_SQL_ERROR(TEXT("Update Error: %s"), *p_error->ErrorMsg);
			}
			catch (std::exception e)
			{
				p_error->ErrorType = FSoci_ErrorType::EUnKnowns;
				p_error->ErrorMsg = UTF8_TO_TCHAR(e.what());
				TC_SQL_ERROR(TEXT("Update Error:%s"),*p_error->ErrorMsg);
			}
		});
		FGraphEventArray Tasks;
		Tasks.Add(get_thread);
		FFunctionGraphTask::CreateAndDispatchWhenReady([this,fun,ptr_SqlTable,p_error]()
		{
			fun(ptr_SqlTable,p_error );
		},TStatId{},&Tasks,ENamedThreads::GameThread);
	}

	void Test_1();
	//线程执行,主线程执行
	virtual FGraphEventRef Async_Operate(TUniqueFunction<void()>&& thread_fun,TUniqueFunction<void()>&& game_fun);
	
	
	
	virtual void Tick();

	void UpdateActiveTime();
	
	virtual void Ping_SQL();//定时Ping 防止断开连接
	//TFuture<bool> GetTime();

	
	
	//////////////需要重载的数据
	ESocil_SQLType Get_SQLType();
	std::string Get_PingSQL();//得到ping sql语句
	USociSubsystem* GetSociSubsystem() const;
private:
	const soci::backend_factory& Get_backend_factory();
	////
protected:
	//bool bConnected;//是否连接
	//UPROPERTY()
	const FSociDefinition* PSociDefinition;//定义名称
	ESQLSubsys_State  SQLSubsys_State;//线程安全
	FCriticalSection SQL_CS;//线程同步锁
	FTimerHandle TH_Tick;
	int8  IRetry;//重试次数
	double LastActiveTime;
	
public:
	soci::session Sql_Session;//回话
};
