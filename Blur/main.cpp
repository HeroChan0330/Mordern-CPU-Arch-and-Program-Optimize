#include <iostream>
#include "BMP.h"
#include <time.h>

using namespace std;



int main()
{
    printf("Begin\n");
    uint32_t t=clock();

    BMP bmp1=BMP::FromFile("1080P.bmp");
    //高斯模糊（浮点）
    t=clock();
    bmp1.GaussianBlurFloat(10);
    cout<<"Time cost(GaussianBlurFloat):"<<clock()-t<<"ms"<<endl;
    bmp1.Save("Out_Gaussian_Float.bmp");


    BMP bmp2=BMP::FromFile("1080P.bmp");
    //高斯模糊（整形）
    t=clock();
    bmp2.GaussianBlurInt(10);
    cout<<"Time cost(GaussianBlurInt):"<<clock()-t<<"ms"<<endl;
    bmp2.Save("Out_Gaussian_Int.bmp");

    BMP bmp3=BMP::FromFile("1080P.bmp");
    //均值模糊
    t=clock();
    for(int n=0;n<3;n++)
        bmp3.MeanBlur(4);
    cout<<"Time cost(MeanBlur):"<<clock()-t<<"ms"<<endl;
    bmp3.Save("Out_Mean.bmp");

    BMP bmp4=BMP::FromFile("1080P.bmp");
    //均值模糊+分支
    t=clock();
    for(int n=0;n<3;n++)
        bmp4.MeanBlurBranch(4);
    cout<<"Time cost(MeanBlurBranch):"<<clock()-t<<"ms"<<endl;
    bmp4.Save("Out_Mean_Branch.bmp");

    BMP bmp5=BMP::FromFile("1080P.bmp");
    //均值模糊+指针
    t=clock();
    for(int n=0;n<3;n++)
        bmp5.MeanBlurPointer(4);
    cout<<"Time cost(MeanBlurPointer):"<<clock()-t<<"ms"<<endl;
    bmp5.Save("Out_Mean_Pointer.bmp");

    // TODO: 行模糊+列模糊优化
    BMP bmp6=BMP::FromFile("1080P.bmp");
    // 行模糊+列模糊
    t=clock();
    for(int n=0;n<3;n++)
        bmp6.MeanBlur1Dim(20);
    cout<<"Time cost(MeanBlur1Dim):"<<clock()-t<<"ms"<<endl;
    bmp6.Save("Out_Mean_1Dim.bmp");

    // ----------------------------------------------------------------
    // 循环100次放大运行时间

    BMP bmp7=BMP::FromFile("1080P.bmp");
    // 行模糊+列模糊
    t=clock();
    bmp7.EnableBlur(true,true);
    for(int n=0;n<100;n++)
        bmp7.MeanBlur1Dim(4);
    cout<<"Time cost(MeanBlur1Dim_X100):"<<clock()-t<<"ms"<<endl;
    bmp7.Save("Out_Mean_1Dim_X100.bmp");


    BMP bmp8=BMP::FromFile("1080P.bmp");
    // 行模糊+列模糊
    t=clock();
    bmp8.EnableBlur(true,true);
    for(int n=0;n<100;n++)
        bmp8.MeanBlur1DimFlatPP(4);
    cout<<"Time cost(MeanBlur1DimFlatPP_X100):"<<clock()-t<<"ms"<<endl;
    bmp8.Save("Out_Mean_1Dim_FlatPP_X100.bmp");


    BMP bmp9=BMP::FromFile("1080P.bmp");
    //均值模糊+指针
    t=clock();
    bmp9.EnableBlur(true,false);
    for(int n=0;n<100;n++)
        bmp9.MeanBlur1DimFlatPP(4);
    
    cout<<"Time cost(MeanBlur1DimFlatPP_ROW_X100):"<<clock()-t<<"ms"<<endl;
    bmp9.Save("Out_Mean_1Dim_FlatPP_ROW_X100.bmp");


    BMP bmp10=BMP::FromFile("1080P.bmp");
    //均值模糊+指针
    t=clock();
    bmp10.EnableBlur(false,true);
    for(int n=0;n<100;n++)
        bmp10.MeanBlur1DimFlatPP(4);
    cout<<"Time cost(MeanBlur1DimFlatPP_COL_X100):"<<clock()-t<<"ms"<<endl;
    bmp10.Save("Out_Mean_1Dim_FlatPP_COL_X100.bmp");


    printf("Finished\n");
    return 0;
}
