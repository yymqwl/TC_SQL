#pragma once

#include "soci/soci.h"
//#include "soci/soci-backend.h"
#include "SociSetting.h"
#include "CoreMinimal.h"
#include "SQLTableWrapper.h"
#include "Person.generated.h"


USTRUCT()
struct FPerson : public FSQLRow
{
	GENERATED_BODY();
public:
	UPROPERTY()
	int Id;
	UPROPERTY()
	FString Name;

};
/*
struct Person
{
	int Id;
	std::string Name;
public:
	Person()
	{
		GF_LOG(TEXT("Person"));
	}
	~Person()
	{
		GF_LOG(TEXT("~Person"));
	}
};*/

/*
SOCI_DECLARE_ROW(FPerson)


namespace soci
{
	template<>
	struct type_conversion<FPerson>
	{
		typedef  values base_type;
		static void from_base(values const & v, indicator  ind , FPerson & p)
		{
			
			//auto struct =FPerson::StaticStruct();
			for (TFieldIterator<FProperty>  it(FPerson::StaticStruct()); it;++it )
			{
				auto key=it->GetNameCPP();
				if (it->IsA(FStrProperty::StaticClass()) )//字符串
				{
					FString str = UTF8_TO_TCHAR(v.get<std::string>(TCHAR_TO_UTF8(*key)).c_str());
					it->SetValue_InContainer(&p,&str);
					//it->SetValue_InContainer(&p,v.get<int>( TCHAR_TO_UTF8(*key) ));
				}
				else if (it->IsA(FIntProperty::StaticClass()))//int
				{
					int32   i32 = v.get<int32>(TCHAR_TO_UTF8(*key));
					it->SetValue_InContainer(&p,&i32);
				}
			}
		}

		static void to_base(const FPerson & p, values & v, indicator & ind)
		{
			for (TFieldIterator<FProperty>  it(FPerson::StaticStruct()); it;++it )
			{
				auto key=it->GetNameCPP();
				if (it->IsA(FStrProperty::StaticClass()) )//字符串
				{
					FString str ;
					it->GetValue_InContainer(&p,&str);
					v.set(TCHAR_TO_UTF8(*key), std::string( TCHAR_TO_UTF8(*str)) );
				}
				else if (it->IsA(FIntProperty::StaticClass()))//int
				{
					int32   i32;
					it->GetValue_InContainer(&p,&i32);
					v.set(TCHAR_TO_UTF8(*key),i32);
				}
			}
			ind = i_ok;
		}
	};
}


struct Person
{
	int Id;
	std::string Name;
public:
	Person()
	{
		GF_LOG(TEXT("Person"));
	}
	~Person()
	{
		GF_LOG(TEXT("~Person"));
	}
};
namespace soci
{
	template<>
	struct type_conversion<Person>
	{
		typedef values base_type;

		static void from_base(values const & v, indicator  ind , Person & p)
		{
			p.Id = v.get<int>("Id");
			p.Name = v.get<std::string>("Name");
		}

		static void to_base(const Person & p, values & v, indicator & ind)
		{
			v.set("Id", p.Id);
			v.set("Name", p.Name);
			ind = i_ok;
		}
	};
}*/