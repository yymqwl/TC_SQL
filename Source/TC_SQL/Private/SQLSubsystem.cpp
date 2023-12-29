// Fill out your copyright notice in the Description page of Project Settings.
#pragma once


#include "SQLSubsystem.h"
#include "SociLog.h"
#include "SociSubsystem.h"


//#include "SociSubsystem.h"

//它允许你将生成的文件(xxx.Generated.cpp)内联到模块CPP文件中，这样能减少需要解析的头文件数量，从而缩短编译时间。
//#include UE_INLINE_GENERATED_CPP_BY_NAME(SQLSubsystem)

/*
UASQLSubsystem::UASQLSubsystem(const FSociDefinition* sociDefinition)
	:Ptr_SociDefinition( sociDefinition)
{
	
}*/

bool USQLSubsystem::IsConnected()
{
	if (PSociDefinition == nullptr)
	{
		return false;
	}
	return SQLSubsys_State == ESQLSubsys_State::EConnected;
}

ESocil_SQLType USQLSubsystem::Get_SQLType()
{
	return PSociDefinition->SQLType;
}

void USQLSubsystem::Initialize(const FSociDefinition* sociDefinition)
{
	check(sociDefinition);
	PSociDefinition = sociDefinition;
	SQLSubsys_State = ESQLSubsys_State::ENone;
	IRetry = SQLSubSys_Retry_Nub;
	TC_SQL_LOG(TEXT("Initialize:%d_%s"),PSociDefinition->SQLType,*PSociDefinition->DefName.ToString());

	GetSociSubsystem()->GetGameInstance()->GetTimerManager().SetTimer(TH_Tick,this,&USQLSubsystem::Tick,SQLSubSys_Tick_Rate,true,0);

	
	//FFunctionGraphTask::CreateAndDispatchWhenReady()
	/*
	TPromise<bool> Promise;
	// 返回一个尚未兑现的未来
	TFuture<bool> Future = Promise.GetFuture();
	Future.WaitFor(FTimespan::FromSeconds(5));
	*/
}

void USQLSubsystem::Deinitialize()
{
	TC_SQL_LOG(TEXT("Deinitialize:%d_%s"),PSociDefinition->SQLType, *PSociDefinition->DefName.ToString());
	PSociDefinition = nullptr;
	GetSociSubsystem()->GetGameInstance()->GetTimerManager().ClearTimer(TH_Tick);
}
void USQLSubsystem::UpdateActiveTime()
{
	LastActiveTime = FPlatformTime::Seconds();
}
/*
const soci::backend_factory& UASQLSubsystem::Get_backend_factory()
{
	return  soci::odbc;
	//return backend_factory();
}*/

const soci::backend_factory& USQLSubsystem::Get_backend_factory()
{

	/*
	if (PSociDefinition->SQLType ==  ESocil_SQLType::EDB2)
	{
		return  soci::odbc;
	}*/
	return  soci::odbc;
	//return backend_factory();
}

void USQLSubsystem::Open()
{
	if(SQLSubsys_State != ESQLSubsys_State::ENone)
	{
		TC_SQL_LOG(TEXT("Open Error: %d"),SQLSubsys_State);
		return ;
	}
	//auto sql = (ANSICHAR*)StringCast<UTF8CHAR>(*PSociDefinition->ConString).Get();
	//std::string(TCHAR_TO_UTF8(*PSociDefinition->ConString));
	//(ANSICHAR*)FTCHARToUTF8((const TCHAR*)str).Get()
	{
		SQLSubsys_State = ESQLSubsys_State::EConnecting;
	}
	
	FFunctionGraphTask::CreateAndDispatchWhenReady([this]()
	{
		TC_SQL_LOG(TEXT("Connecting:"));
		try
		{
			{
				FScopeLock SL(&SQL_CS);
				Sql_Session.open(Get_backend_factory(),  TCHAR_TO_UTF8(*PSociDefinition->ConString));
			}
			SQLSubsys_State = ESQLSubsys_State::EConnected;//直接算连上
			TC_SQL_LOG(TEXT("Connected:"));
			UpdateActiveTime();
		}
		catch (soci::soci_error const& e)
		{
			SQLSubsys_State = ESQLSubsys_State::ENone;
			TC_SQL_LOG(TEXT("Connect Error: %s"),e.what());
		}
	});
}
void USQLSubsystem::Close()
{
	if (SQLSubsys_State != ESQLSubsys_State::EConnected )
	{
		TC_SQL_LOG(TEXT("Open Close: %d"),SQLSubsys_State);
		return;
	}
	FFunctionGraphTask::CreateAndDispatchWhenReady([this]()
	{
		
		try
		{
			FScopeLock SL(&SQL_CS);
			Sql_Session.close();
		}
		catch (soci::soci_error const& e)
		{
			TC_SQL_LOG(TEXT("Close Error: %s"),e.what());
		}
		SQLSubsys_State = ESQLSubsys_State::ENone;//直接算连上
	});
}



void USQLSubsystem::Tick()
{
	if (SQLSubsys_State == ESQLSubsys_State::EFail)//失败重试
	{
		if (IRetry>0 )
		{
			--IRetry;
			SQLSubsys_State = ESQLSubsys_State::ENone;
			TC_SQL_LOG(TEXT("Retry Nub: %d"),IRetry);
		}
		else
		{
			TC_SQL_LOG(TEXT("Retry Nub Not enough"));
		}
	}
	else if (SQLSubsys_State == ESQLSubsys_State::ENone )
	{
		this->Open();
	}
	else if (SQLSubsys_State == ESQLSubsys_State::EConnecting)
	{
		TC_SQL_LOG(TEXT("%s Connecting"),*PSociDefinition->DefName.ToString());
	}
	else if (SQLSubsys_State == ESQLSubsys_State::EConnected )
	{
		if (IsConnected())
		{
			auto delta = FPlatformTime::Seconds()-LastActiveTime;
			if (delta > SQLSubSys_TimeOut)//防止超时
			{
				Ping_SQL();
			}
		}
	}
}


std::string USQLSubsystem::Get_PingSQL()
{
	std::string sql = "";
	if (Get_SQLType() == ESocil_SQLType::EOdbc)
	{
		sql = "select getdate()";
	}
	return sql;
}

void USQLSubsystem::Ping_SQL()
{
	if (SQLSubsys_State != ESQLSubsys_State::EConnected )
	{
		TC_SQL_LOG(TEXT("Open Ping: %d"),SQLSubsys_State);
		return;
	}
	FFunctionGraphTask::CreateAndDispatchWhenReady([this]()
	{
		try
		{
			{
				FScopeLock SL(&SQL_CS);
				Sql_Session << Get_PingSQL() ;//,into(r);
			}
			this->UpdateActiveTime();
			TC_SQL_LOG(TEXT("Ping_SQL Success"));
		}
		catch (std::runtime_error const& e)
		{
			TC_SQL_LOG(TEXT("Ping Error: %s"),e.what());
		}
	});
}


FGraphEventRef USQLSubsystem::Async_Operate(TUniqueFunction<void()>&& thread_fun,TUniqueFunction<void()>&& game_fun)
{
	//FFunctionGraphTask::CreateAndDispatchWhenReady()
	auto get_thread = FFunctionGraphTask::CreateAndDispatchWhenReady(MoveTemp(thread_fun));
	//auto ger  = FFunctionGraphTask::CreateAndDispatchWhenReady(MoveTemp(thread_fun));
	// 同时创建多个任务
	FGraphEventArray Tasks;
	Tasks.Add(get_thread);
	return FFunctionGraphTask::CreateAndDispatchWhenReady(MoveTemp(game_fun),TStatId{},&Tasks,ENamedThreads::GameThread);
}



/*
TFuture<bool> UASQLSubsystem::GetTime()
{
	TPromise<bool> promise;
	promise.SetValue(true);
	return promise.GetFuture();
}*/


/*
template <typename T>
void USQLSubsystem::Query(FString sql, TUniqueFunction<void(T)> fun)
{
	fun.Callable(nullptr);
}*/

void USQLSubsystem::Test_1( )
{
//	Sql_Session << sql ,
//			soci::use(*data);
}




USociSubsystem* USQLSubsystem::GetSociSubsystem() const
{
	return Cast<USociSubsystem>(GetOuter());
	
}