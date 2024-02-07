#ifndef __BMP_H__
#define __BMP_H__

#include "BMPStructer.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>


class BMP{
public:
    BITMAPFILEHEADER file;
    BITMAPINFOHEADER info;
    uint32_t stride;
    byte*content;
    BMP(uint32_t width,uint32_t height);
    BMP();
    ~BMP();
    static BMP FromFile(char*path);
    void Save(char*path);
    BMP Clone();

    void GaussianBlurFloat(int radius);
    void GaussianBlurInt(int radius);

    void MeanBlur(int radius);
    void MeanBlurBranch(int radius);
    void MeanBlurPointer(int radius);

    bool row=true;
    bool col=true;
    void EnableBlur(bool row,bool col);
    void MeanBlur1Dim(int radius);
    void MeanBlur1DimFlatPP(int radius);
private:


};

#endif // __BMP_H__
