// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BaiDuMapApiData.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
namespace EBaiDuAPIType
{
	enum Type
	{
		//步行
		Walking, 
		//公交
		Transit, 
		//骑行
		Riding, 
		//驾车
		Driving  
	};
}

struct FVector2D_Double
{
	double X;
	double Y;

	FVector2D_Double operator-(const FVector2D_Double& Other) const
	{
		return FVector2D_Double(X - Other.X, Y - Other.Y);
	}

	FVector2D_Double operator+(const FVector2D_Double& Other) const
	{
		return FVector2D_Double(X + Other.X, Y + Other.Y);
	}

	FVector2D_Double operator/(const FVector2D_Double& Other) const
	{
		return FVector2D_Double(X * Other.X, Y * Other.Y);
	}

	FVector2D_Double operator*(const float& Other) const
	{
		return FVector2D_Double(X * Other, Y * Other);
	}

	FVector2D_Double operator/(const float& Other) const
	{
		return FVector2D_Double(X / Other, Y / Other);
	}

	FVector2D_Double operator/(const FVector2D& Other) const
	{
		return FVector2D_Double(X / Other.X, Y / Other.Y);
	}

	FVector2D_Double operator*(const FVector2D& Other) const
	{
		return FVector2D_Double(X * Other.X, Y * Other.Y);
	}

	FVector2D_Double()
		:X(0.f), Y(0.f)
	{}

	FVector2D_Double(double X,double Y)
		:X(X),Y(Y)
	{}

	double Size() const
	{
		return sqrt(X * X + Y * Y);
	}
};
struct PyhonData
{
	std::string Path = std::string("PythonPlugins/Source/ThirdParty/PythonThirdParty/Resources");
	const char* Type = "BaiduWalkAPI";
	const char* Origin;
	const char* Destination;
};
struct LevelArray
{
	unsigned int ID = 0;
	TArray<FVector2D_Double> Array;
};


UCLASS()
class PYTHONPLUGINS_API UBaiDuMapApiData : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/* 
		< 根据经纬度获取UE4坐标值
		< StartActor:UE4初始Actor位置值
		< EndActor:UE4结束位置Actor位置值
		< Origin:百度地图初始位置经纬度 例如: 130.456102，38.184562
		< Destination:百度地图结束位置经纬度 例如: 145.256312，36.164552
		< OutDistance:输出实际地图路程值 单位:米(m)
		< OutAllTime:输出百度地图给出的步行事件 单位:秒(s)
		< Number:需要输出多少层数据 若值小于等于0，则按照百度地图给出的最小分段作为输出的最小数据层
		< InType:设置路途为步行、骑车等类型
		< IsIgnoreSamllLen:是否忽略拐点长度小于分长度，若忽略则为true
	*/
	UFUNCTION(BlueprintCallable, Category = "PythonPlugins")
		static TArray<FVector2D>OutPythonMapAPIData(FVector StartActor, FVector EndActor, FString Origin, FString Destination, int& OutDistance, int& OutAllTime, int Number = 25, EBaiDuAPIType::Type InType = EBaiDuAPIType::Walking,bool IsIgnoreSamllLen = false);

	static FVector2D_Double IBToXY(FVector2D_Double IB);
	
	static double DistanceLngLat(double longitudeA,double longitudeB,double latitudeA,double latitudeB);

private:

	static const char* BaiDuMapAPIType(EBaiDuAPIType::Type InType);


protected:
	static void CToPython(std::string Path, const char* Type, const char* Origin, const char* Destination, const char* API);

	static TArray<FVector2D_Double> UpDataLevelArrayData(TArray<LevelArray> Value, int Number, bool IsIgnoreSamllLen, bool IsDataMinimum = false);

};
