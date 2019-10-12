// Fill out your copyright notice in the Description page of Project Settings.


#include "BaiDuMapApiData.h"
#include "Engine.h"
#include "python.h"

TArray<FVector2D> UBaiDuMapApiData::OutPythonMapAPIData(FVector StartActor, FVector EndActor, FString Origin, FString Destination, int& OutDistance, int& OutAllTime, int Number/* = 25*/, EBaiDuAPIType::Type InType/* = EBaiDuAPIType::Walking*/, bool IsIgnoreSamllLen /*= false*/)
{
	TArray<FVector2D> UE4Data;
	//TArray<Vector2D_Double> BaiduData;
	TArray<FVector2D_Double> BaiduDataXY;
	TArray<LevelArray> LevelBaiData;
	TArray<int> BaiduDataTimeAndDistance;

	std::string OldOrigin = TCHAR_TO_UTF8(*Origin);
	std::string OldDestination = TCHAR_TO_UTF8(*Destination);
	std::size_t posA = OldOrigin.find(",");
	std::string NewOrigin = OldOrigin.substr(posA + 1, OldOrigin.size()) + "," + OldOrigin.substr(0, posA);
	posA = OldDestination.find(",");
	std::string NewDestination = OldDestination.substr(posA + 1, OldDestination.size()) + "," + OldDestination.substr(0, posA);


	PyhonData PyData;
	PyData.Origin = NewOrigin.c_str();
	PyData.Destination = NewDestination.c_str();

	CToPython(PyData.Path, PyData.Type, PyData.Origin, PyData.Destination, BaiDuMapAPIType(InType));
	
	std::string file = TCHAR_TO_UTF8(*FPaths::ProjectPluginsDir()) + std::string("PythonPlugins/Source/ThirdParty/PythonThirdParty/Resources/walking.txt");
	//std::string file = std::string("F:/test/test/Plugins/PythonPlugins/PythonPlugins/Source/ThirdParty/PythonThirdParty/Resources/walking.txt");
	std::ifstream infile;
	infile.open(file.data());   //将文件流对象与文件连接起来 

	unsigned int AID = 0;

	if (infile.is_open())
	{

		std::string s;
		while (getline(infile, s))
		{
			std::string s1, s2, s3;
			//getline(infile, s);  //读取一行输入
			std::size_t pos = s.find(",");
			if (s._Equal("------"))
			{
				LevelArray A;
				AID += 1;
				A.ID = AID;
				LevelBaiData.Add(A);
			}
			if (pos != std::string::npos)
			{
				s1 = s.substr(0, pos);
				s2 = s.substr(pos + 1, s.size());

				FVector2D_Double v2lf = FVector2D_Double(atof(s1.c_str()), atof(s2.c_str()));
				//BaiduData.Add(v2lf);
				FVector2D_Double A = IBToXY(v2lf);
				//BaiduDataXY.Add(A);
				LevelBaiData[LevelBaiData.Num() - 1].Array.Add(A);
			}
			std::size_t pos_1 = s.find(":");
			if (pos_1 != std::string::npos)
			{
				s3 = s.substr(pos_1 + 1, s.size());
				BaiduDataTimeAndDistance.Add(atoi(s3.c_str()));
			}
		}
		infile.close();
	}
	FVector Delate = EndActor - StartActor;

	//double MaxDistance = (LevelBaiData[LevelBaiData.Num() - 1].Array[LevelBaiData[LevelBaiData.Num() - 1].Array.Num() - 1] - LevelBaiData[0].Array[0]).Size();

	
	//double Scale = MaxDistance / FVector2D(Delate.X, Delate.Y).Size();
	BaiduDataXY = UpDataLevelArrayData(LevelBaiData, Number, IsIgnoreSamllLen, Number <= 0);

	FVector2D_Double StartEndVector2D = (BaiduDataXY[BaiduDataXY.Num() - 1] - BaiduDataXY[0]); //米勒投影坐标向量

	double Scale = StartEndVector2D.Size() / FVector2D(Delate.X, Delate.Y).Size();
	
	for (auto &Item : BaiduDataXY)
	{
		FVector2D_Double p = (Item - BaiduDataXY[0]) / Scale;
		/*
		p.X = DistanceLngLat(BaiduData[0].X, Item.X, BaiduData[0].Y, BaiduData[0].Y) / Scale;
		p.Y = DistanceLngLat(BaiduData[0].X, BaiduData[0].X, BaiduData[0].Y, Item.Y) / Scale;
		*/
		UE4Data.Add(FVector2D(p.X, p.Y));
		
	}
	FVector2D Scale2D = FVector2D(Delate.X, Delate.Y) / (UE4Data[UE4Data.Num() - 1] - UE4Data[0]);

	TArray<FVector2D> UE4NewData;
	for (auto &Item : UE4Data)
	{
		UE4NewData.Add(Item * Scale2D);
	}
	OutDistance = BaiduDataTimeAndDistance[1];
	OutAllTime = BaiduDataTimeAndDistance[0];
	//Py_Initialize();
	infile.close();
	return UE4NewData;
}

FVector2D_Double UBaiDuMapApiData::IBToXY(FVector2D_Double IB)
{

	double L = 6378000 * PI * 2;//地球周长
	double W = L;// 平面展开后，x轴等于周长 
	double H = L / 2;// y轴约等于周长一半  
	double mill = 2.3;// 米勒投影中的一个常数，范围大约在正负2.3之间  
	double x = IB.X * PI / 180;// 将经度从度数转换为弧度  
	double y = IB.Y * PI / 180;// 将纬度从度数转换为弧度  
	
	y = 1.25 * log(tan(0.25 * PI + 0.4 * y));// 米勒投影的转换 

	// 弧度转为实际距离  
	x = (W / 2) + (W / (2 * PI)) * x;
	y = (H / 2) - (H / (2 * mill)) * y;

	return FVector2D_Double(x,y);
}

double UBaiDuMapApiData::DistanceLngLat(double longitudeA, double longitudeB, double latitudeA, double latitudeB)
{
	double LngA = longitudeA * PI / 180;
	double LngB = longitudeB * PI / 180;
	double LatA = latitudeA * PI / 180;
	double LatB = latitudeB * PI / 180;

	double DeltalLong = LngB - LngA;
	double RadianAB = acos(sin(LatA) * sin(LatB) + cos(LatA) * cos(LatB) * cos(DeltalLong));

	return (RadianAB * 6378000);
}

const char* UBaiDuMapApiData::BaiDuMapAPIType(EBaiDuAPIType::Type InType)
{
	const char* API;
	switch (InType)
	{
	case EBaiDuAPIType::Walking:
		API = "http://api.map.baidu.com/directionlite/v1/walking?";
		break;
	case EBaiDuAPIType::Transit:
		API = "http://api.map.baidu.com/direction/v2/transit?";
		break;
	case EBaiDuAPIType::Riding:
		API = "http://api.map.baidu.com/direction/v2/riding?";
		break;
	case EBaiDuAPIType::Driving:
		API = "http://api.map.baidu.com/direction/v2/driving?";
		break;
	default:
		API = "http://api.map.baidu.com/directionlite/v1/walking?";
		break;
	}
	return API;
}

void UBaiDuMapApiData::CToPython(std::string Path, const char* Type, const char* Origin, const char* Destination, const char* API)
{
	
	Py_Initialize();
	
	std::string chdir_cmd = std::string("sys.path.append(\"") + TCHAR_TO_UTF8(*FPaths::ProjectPluginsDir()) + Path + "\")";
	const char* cstr_cmd = chdir_cmd.c_str();
	PyRun_SimpleString("import sys");
	PyRun_SimpleString(cstr_cmd);

	PyObject* moduleName = PyString_FromString(Type);

	PyObject* pModule = PyImport_Import(moduleName);
	if (!pModule) // 加载模块失败
	{
		UE_LOG(LogTemp, Warning, TEXT("[ERROR] Python get module failed."));
		return;
	}

	// 加载函数
	PyObject* pv = PyObject_GetAttrString(pModule, "OutBaiduAPILngAndLat");
	if (!pv || !PyCallable_Check(pv))  // 验证是否加载成功
	{
		UE_LOG(LogTemp, Warning, TEXT("[ERROR] Can't find funftion (OutBaiduAPILngAndLat)"));
		return ;
	}

	FString UE4Path = FPaths::ConvertRelativePathToFull(FPaths::ProjectPluginsDir());
	FString UE4TextPath;
	TArray<TCHAR> UE4PathArray = UE4Path.GetCharArray();
	for (auto & Item : UE4PathArray)
	{
		FString A;
		if (FString(1,&Item).Equals("/"))
		{
			A = "\\";
		}
		else
		{
			A = FString(1, &Item);
		}
		UE4TextPath += A;
	}
	std::string textPath = TCHAR_TO_UTF8(*UE4TextPath) + std::string("PythonPlugins\\Source\\ThirdParty\\PythonThirdParty\\Resources\\walking.txt");

	std::string file = TCHAR_TO_UTF8(*FPaths::ProjectPluginsDir()) + std::string("PythonPlugins/Source/ThirdParty/PythonThirdParty/Resources/MyBaiDuAk.txt");

	std::ifstream infile;
	infile.open(file.data());   //将文件流对象与文件连接起来
	std::string s;
	getline(infile, s);

	// 设置参数 或使用
	/*
	PyObject* args = PyTuple_New(2);   // 2个参数
	PyObject* arg1 = PyUnicodeUCS2_FromString(Origin);    // 参数一
	PyObject* arg2 = PyUnicodeUCS2_FromString(Destination);    // 参数二
	PyTuple_SetItem(args, 0, arg1);
	PyTuple_SetItem(args, 1, arg2);
	*/
	PyObject* args = Py_BuildValue("(sssss)", API, s.c_str(), textPath.c_str(), Origin, Destination);
	// 调用函数
	PyObject* pRet = PyObject_CallObject(pv, args);
	
	
	Py_Finalize();     //释放资源
	
	return ;
	

}

TArray<FVector2D_Double> UBaiDuMapApiData::UpDataLevelArrayData(TArray<LevelArray> Value, int Number, bool IsIgnoreSamllLen, bool IsDataMinimum /*= false*/)
{
	double AllPath = 0; //路径
	double MinTurningLen = -1; //最小拐弯长度
	double MinNumberLen = 0; //最小分段长度
	TArray<FVector2D_Double> OutValue;
	for (auto& Item : Value)
	{
		FVector2D_Double A = Item.Array[Item.Array.Num() - 1] - Item.Array[0];
		double Length = A.Size();
		AllPath += Length;
		if (MinTurningLen < 0)
		{
			MinTurningLen = Length;
		}
		MinTurningLen = std::fmin(MinTurningLen, Length);
	}
	MinNumberLen = IsDataMinimum ?  MinTurningLen : AllPath / Number;
	//MinNumberLen = std::fmin(AllPath / Number, MinTurningLen);
	//MinNumberLen = AllPath / Number;
	for (auto& Item : Value)
	{
		FVector2D_Double A = Item.Array[Item.Array.Num() - 1] - Item.Array[0];
		double ItemLen = A.Size();
		int NumValue = IsIgnoreSamllLen ? std::floor(ItemLen / MinNumberLen) : std::fmax(std::floor(ItemLen / MinNumberLen), 1);
		for (int i = 0; i < NumValue; i++)
		{
			FVector2D_Double B = Item.Array[0] + (A/ NumValue)*i;
			OutValue.Add(B);
		}
	}
	OutValue.Add(Value[Value.Num() - 1].Array[Value[Value.Num() - 1].Array.Num() - 1]);
	return OutValue;
}



