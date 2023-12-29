// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "CoreMinimal.h"
//using Error = ELogVerbosity::Type;

TC_SQL_API DECLARE_LOG_CATEGORY_EXTERN(TC_SQL_Log,Log,All);

//日志
#define TC_SQL_LOG(Format, ...) \
	UE_LOG(TC_SQL_Log,Log,Format,##__VA_ARGS__)

//错误日志
#define TC_SQL_ERROR(Format, ...) \
	UE_LOG(TC_SQL_Log,Error,Format,##__VA_ARGS__)



#include <soci/soci.h>

class Ue_Soci_Log_Impl : public soci::logger_impl
{
public:
	virtual void start_query(std::string const& query) override;
private:
	virtual Ue_Soci_Log_Impl* do_clone() const override;
};

