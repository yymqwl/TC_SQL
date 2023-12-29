// Fill out your copyright notice in the Description page of Project Settings.


#include "SociLog.h"


DEFINE_LOG_CATEGORY(TC_SQL_Log);



void Ue_Soci_Log_Impl::start_query(std::string const& query)
{
	TC_SQL_LOG(TEXT("%s"),  UTF8_TO_TCHAR(query.c_str()));
}

Ue_Soci_Log_Impl* Ue_Soci_Log_Impl::do_clone() const
{
	return new Ue_Soci_Log_Impl();
}

