#include "BMP.h"

BMP::BMP(uint32_t width,uint32_t height){
    this->file.bfType=19778;
    this->file.bfSize=4;
    this->file.bfReserved1=0;
    this->file.bfReserved2=54;
    this->file.bfOffBits=977534976;//不知道这个值有什么含义，反正就是这样....
    this->info.biSize=40;
    this->info.biWidth=width;
    this->info.biHeight=height;
    this->info.biPlanes=1;
    this->info.biBitCount=24;
    this->info.biCompression=0;
    //result->info.biSizeImage=
    this->info.biXPelsPerMeter=144;
    this->info.biYPelsPerMeter=144;
    this->info.biClrUsed=0;
    this->info.biClrImportant=0;

    this->stride=(((this->info.biWidth*this->info.biBitCount)+31)>>5)<<2;
    this->info.biSizeImage=this->stride*this->info.biHeight;
    this->content=(byte*)malloc(this->info.biSizeImage);//分配内存
}
BMP::BMP(){
}
BMP::~BMP(){
    delete(this->content);
    this->content=NULL;
}
BMP BMP::FromFile(char*path){
    FILE*fp=fopen(path,"rb+");
    BMP result;
    fread(&result.file,1,14,fp);
    fread(&result.info,1,40,fp);
    result.content=(byte*)malloc(result.info.biSizeImage);//分配内存
    fread(result.content,result.info.biSizeImage,1,fp);
    fclose(fp);
    result.stride=(((result.info.biWidth*result.info.biBitCount)+31)>>5)<<2;

    return result;
}
void BMP::Save(char*path){
    FILE*fp=fopen(path,"wb+");
    fwrite(&this->file,1,14,fp);
    fwrite(&this->info,1,40,fp);
    fwrite(this->content,1,this->info.biSizeImage,fp);
    fclose(fp);
}

BMP BMP::Clone(){
    BMP result;
    memcpy(&result,this,sizeof(BMP));
    return result;
}


float*CreateGaussianKernelFloat(int radius){
    float sigma=radius/2;
    float weightsSum=0;
    int weightsLen=radius*2+1;
    float *weights=new float[weightsLen*weightsLen];

    //printf("GaussianKernel:\n");
    for(int v=-radius;v<=radius;v++){
        for(int u=-radius;u<=radius;u++){
            int i=u+radius,j=v+radius;
            weights[j*weightsLen+i]=1/(6.28*sigma*sigma)*exp((-u*u-v*v)/(2*sigma*sigma));
            weightsSum+=weights[j*weightsLen+i];
        }
    }
    for(int u=-radius;u<=radius;u++){ //row
        for(int v=-radius;v<=radius;v++){ //collum
            int i=u+radius,j=v+radius;
            weights[j*weightsLen+i]/=weightsSum;
            //printf("%f ",weights[j*weightsLen+i]);
        }
        //printf("\n");
    }
    return weights;
}
// 最原始高斯模糊
void BMP::GaussianBlurFloat(int radius){
    float *weights=CreateGaussianKernelFloat(radius);

    byte *newContent=new byte[this->info.biSizeImage]; //分配内存
    for(int j=0;j<info.biHeight;j++){ // 遍历像素(二重循环)
        for(int i=0;i<info.biWidth;i++){
            float rSum=0,gSum=0,bSum=0;
            int wi = 0;
            for(int v=-radius;v<=radius;v++){ // 卷积操作(二重循环)
                for(int u=-radius;u<=radius;u++){
                    if(u+i<0||u+i>=info.biWidth||v+j<0||v+j>=info.biHeight){ //防止越界
                        continue;
                    }
                    int base = ((j+v)*stride) + (u+i)*3;
                    
                    bSum += content[base+0] * weights[wi];
                    gSum += content[base+1] * weights[wi];
                    rSum += content[base+2] * weights[wi];
                    wi++;
                }
            }
            int base = (j*stride) + i*3;
            newContent[base+0]=bSum;
            newContent[base+1]=gSum;
            newContent[base+2]=rSum;
 
        }
    }
    delete weights;
    delete this->content;
    this->content=newContent;
}



int*CreateGaussianKernelInt(int radius){
    float *weights_float=CreateGaussianKernelFloat(radius);
    int weightsLen=radius*2+1;
    int *weights_int = new int[weightsLen*weightsLen];
    for(int j=0;j<weightsLen*weightsLen;j++){
        weights_int[j] = (int)round(weights_float[j]*32768);
    }
    return weights_int;
}

// 用定点代替浮点的高斯模糊
void BMP::GaussianBlurInt(int radius){
    int *weights=CreateGaussianKernelInt(radius);
    byte*newContent=new byte[this->info.biSizeImage]; //分配内存
    for(int j=0;j<info.biHeight;j++){
        for(int i=0;i<info.biWidth;i++){
            int rSum=0,gSum=0,bSum=0;
            int wi = 0;
            for(int v=-radius;v<=radius;v++){
                for(int u=-radius;u<=radius;u++){
                    if(u+i<0||u+i>=info.biWidth||v+j<0||v+j>=info.biHeight){
                        continue;
                    }
                    int base = ((j+v)*stride) + (u+i)*3;
                    
                    bSum += content[base+0] * weights[wi];
                    gSum += content[base+1] * weights[wi];
                    rSum += content[base+2] * weights[wi];
                    wi++;
                }
            }
            int base = (j*stride) + i*3;
            newContent[base+0]=bSum>>15;
            newContent[base+1]=gSum>>15;
            newContent[base+2]=rSum>>15;
        }
    }
    delete weights;
    delete this->content;
    this->content=newContent;
}

// 均值模糊最原始版本
void BMP::MeanBlur(int radius){
    byte *newContent=new byte[this->info.biSizeImage]; //分配内存
    int len = radius*2+1;
    int div = len*len;
    for(int j=0;j<info.biHeight;j++){
        for(int i=0;i<info.biWidth;i++){
            int rSum=0,gSum=0,bSum=0;
            for(int v=-radius;v<=radius;v++){
                for(int u=-radius;u<=radius;u++){
                    if(u+i<0||u+i>=info.biWidth||v+j<0||v+j>=info.biHeight){
                        continue;
                    }
                    int base = ((j+v)*stride) + (u+i)*3;
                    
                    bSum += content[base+0];
                    gSum += content[base+1];
                    rSum += content[base+2];
                }
            }
            int base = (j*stride) + i*3;
            newContent[base+0]=bSum/div;
            newContent[base+1]=gSum/div;
            newContent[base+2]=rSum/div;
        }
    }
    delete this->content;
    this->content=newContent;
}



void BMP::MeanBlurBranch(int radius){
    byte *newContent=new byte[this->info.biSizeImage]; //分配内存
    int len = radius*2+1;
    int div = len*len;

    // j=0~radius-1
    for(int j=0;j<radius;j++){
        for(int i=0;i<info.biWidth;i++){
            int rSum=0,gSum=0,bSum=0;
            for(int v=-radius;v<=radius;v++){
                for(int u=-radius;u<=radius;u++){
                    if(u+i<0||u+i>=info.biWidth||v+j<0||v+j>=info.biHeight){
                        continue;
                    }
                    int base = ((j+v)*stride) + (u+i)*3;
                    
                    bSum += content[base+0];
                    gSum += content[base+1];
                    rSum += content[base+2];
                }
            }
            int base = (j*stride) + i*3;
            newContent[base+0]=bSum/div;
            newContent[base+1]=gSum/div;
            newContent[base+2]=rSum/div;
        }
    }
    // j=H-radius~H-1
    for(int j=info.biHeight-radius;j<info.biHeight;j++){
        for(int i=0;i<info.biWidth;i++){
            int rSum=0,gSum=0,bSum=0;
            for(int v=-radius;v<=radius;v++){
                for(int u=-radius;u<=radius;u++){
                    if(u+i<0||u+i>=info.biWidth||v+j<0||v+j>=info.biHeight){
                        continue;
                    }
                    int base = ((j+v)*stride) + (u+i)*3;
                    
                    bSum += content[base+0];
                    gSum += content[base+1];
                    rSum += content[base+2];
                }
            }
            int base = (j*stride) + i*3;
            newContent[base+0]=bSum/div;
            newContent[base+1]=gSum/div;
            newContent[base+2]=rSum/div;
        }
    }

    // i=0~radius-1
    for(int j=0;j<info.biHeight;j++){
        for(int i=0;i<radius;i++){
            int rSum=0,gSum=0,bSum=0;
            for(int v=-radius;v<=radius;v++){
                for(int u=-radius;u<=radius;u++){
                    if(u+i<0||u+i>=info.biWidth||v+j<0||v+j>=info.biHeight){
                        continue;
                    }
                    int base = ((j+v)*stride) + (u+i)*3;
                    
                    bSum += content[base+0];
                    gSum += content[base+1];
                    rSum += content[base+2];
                }
            }
            int base = (j*stride) + i*3;
            newContent[base+0]=bSum/div;
            newContent[base+1]=gSum/div;
            newContent[base+2]=rSum/div;
        }
    }


    // i=W-radius~W-1
    for(int j=0;j<info.biHeight;j++){
        for(int i=info.biWidth-radius;i<info.biWidth;i++){
            int rSum=0,gSum=0,bSum=0;
            for(int v=-radius;v<=radius;v++){
                for(int u=-radius;u<=radius;u++){
                    if(u+i<0||u+i>=info.biWidth||v+j<0||v+j>=info.biHeight){
                        continue;
                    }
                    int base = ((j+v)*stride) + (u+i)*3;
                    
                    bSum += content[base+0];
                    gSum += content[base+1];
                    rSum += content[base+2];
                }
            }
            int base = (j*stride) + i*3;
            newContent[base+0]=bSum/div;
            newContent[base+1]=gSum/div;
            newContent[base+2]=rSum/div;
        }
    }

    // main loop
    for(int j=radius;j<info.biHeight-radius;j++){
        for(int i=radius;i<info.biWidth-radius;i++){
            int rSum=0,gSum=0,bSum=0;
            for(int v=-radius;v<=radius;v++){
                for(int u=-radius;u<=radius;u++){
                    int base = ((j+v)*stride) + (u+i)*3;
                    
                    bSum += content[base+0];
                    gSum += content[base+1];
                    rSum += content[base+2];
                }
            }
            int base = (j*stride) + i*3;
            newContent[base+0]=bSum/div;
            newContent[base+1]=gSum/div;
            newContent[base+2]=rSum/div;
        }
    }

    delete this->content;
    this->content=newContent;
}



void BMP::MeanBlurPointer(int radius){
    byte *newContent=new byte[this->info.biSizeImage]; //分配内存
    int len = radius*2+1;
    int div = len*len;

    // j=0~radius-1
    for(int j=0;j<radius;j++){
        for(int i=0;i<info.biWidth;i++){
            int rSum=0,gSum=0,bSum=0;
            for(int v=-radius;v<=radius;v++){
                for(int u=-radius;u<=radius;u++){
                    if(u+i<0||u+i>=info.biWidth||v+j<0||v+j>=info.biHeight){
                        continue;
                    }
                    int base = ((j+v)*stride) + (u+i)*3;
                    
                    bSum += content[base+0];
                    gSum += content[base+1];
                    rSum += content[base+2];
                }
            }
            int base = (j*stride) + i*3;
            newContent[base+0]=bSum/div;
            newContent[base+1]=gSum/div;
            newContent[base+2]=rSum/div;
        }
    }
    // j=H-radius~H-1
    for(int j=info.biHeight-radius;j<info.biHeight;j++){
        for(int i=0;i<info.biWidth;i++){
            int rSum=0,gSum=0,bSum=0;
            for(int v=-radius;v<=radius;v++){
                for(int u=-radius;u<=radius;u++){
                    if(u+i<0||u+i>=info.biWidth||v+j<0||v+j>=info.biHeight){
                        continue;
                    }
                    int base = ((j+v)*stride) + (u+i)*3;
                    
                    bSum += content[base+0];
                    gSum += content[base+1];
                    rSum += content[base+2];
                }
            }
            int base = (j*stride) + i*3;
            newContent[base+0]=bSum/div;
            newContent[base+1]=gSum/div;
            newContent[base+2]=rSum/div;
        }
    }

    // i=0~radius-1
    for(int j=0;j<info.biHeight;j++){
        for(int i=0;i<radius;i++){
            int rSum=0,gSum=0,bSum=0;
            for(int v=-radius;v<=radius;v++){
                for(int u=-radius;u<=radius;u++){
                    if(u+i<0||u+i>=info.biWidth||v+j<0||v+j>=info.biHeight){
                        continue;
                    }
                    int base = ((j+v)*stride) + (u+i)*3;
                    
                    bSum += content[base+0];
                    gSum += content[base+1];
                    rSum += content[base+2];
                }
            }
            int base = (j*stride) + i*3;
            newContent[base+0]=bSum/div;
            newContent[base+1]=gSum/div;
            newContent[base+2]=rSum/div;
        }
    }


    // i=W-radius~W-1
    for(int j=0;j<info.biHeight;j++){
        for(int i=info.biWidth-radius;i<info.biWidth;i++){
            int rSum=0,gSum=0,bSum=0;
            for(int v=-radius;v<=radius;v++){
                for(int u=-radius;u<=radius;u++){
                    if(u+i<0||u+i>=info.biWidth||v+j<0||v+j>=info.biHeight){
                        continue;
                    }
                    int base = ((j+v)*stride) + (u+i)*3;
                    
                    bSum += content[base+0];
                    gSum += content[base+1];
                    rSum += content[base+2];
                }
            }
            int base = (j*stride) + i*3;
            newContent[base+0]=bSum/div;
            newContent[base+1]=gSum/div;
            newContent[base+2]=rSum/div;
        }
    }


    byte *oc_ptr;
    byte *nc_ptr;
    byte *t_ptr;
    int gap = stride-3*len;
    // main loop
    for(int j=radius;j<info.biHeight-radius;j++){
        oc_ptr = &content[j*stride+radius*3];
        nc_ptr = &newContent[j*stride+radius*3];
        for(int i=radius;i<info.biWidth-radius;i++){
            int rSum=0,gSum=0,bSum=0;
            t_ptr = oc_ptr - stride*radius - 3*radius;
            for(int v=-radius;v<=radius;v++){
                for(int u=-radius;u<=radius;u++){
                    // t_ptr = &content[(j+v)*stride+(u+i)*3];
                    bSum += *(t_ptr++);
                    gSum += *(t_ptr++);
                    rSum += *(t_ptr++);
                }
                t_ptr += gap;
            }
            *(nc_ptr++)=bSum/div;
            *(nc_ptr++)=gSum/div;
            *(nc_ptr++)=rSum/div;
            oc_ptr+=3;
        }
    }

    delete this->content;
    this->content=newContent;
}


void BMP::EnableBlur(bool row,bool col){
    this->row = row;
    this->col = col;
}

void BMP::MeanBlur1Dim(int radius){
    byte *newContent=new byte[this->info.biSizeImage]; //分配内存
    byte *dst = newContent;
    byte *src = content;
    int len = 2*radius + 1;
    // 逐行模糊
    if(row){
        for(int j=0;j<info.biHeight;j++){
            byte *src_ptr_l = &src[j*stride];
            byte *src_ptr_r = &src[j*stride];
            byte* dst_ptr = &dst[j*stride];

            int rSum = 0,gSum = 0,bSum = 0;
            for(int u=0;u<=radius;u++){
                bSum += *(src_ptr_r++);
                gSum += *(src_ptr_r++);
                rSum += *(src_ptr_r++);
            }
            *(dst_ptr++) = bSum / len;
            *(dst_ptr++) = gSum / len;
            *(dst_ptr++) = rSum / len;

            for(int i=1;i<=radius;i++){
                bSum += *(src_ptr_r++);
                gSum += *(src_ptr_r++);
                rSum += *(src_ptr_r++);
                *(dst_ptr++) = bSum / len;
                *(dst_ptr++) = gSum / len;
                *(dst_ptr++) = rSum / len;
            }
            
            for(int i=radius+1;i<info.biWidth-radius;i++){
                bSum += *(src_ptr_r++) - *(src_ptr_l++);
                gSum += *(src_ptr_r++) - *(src_ptr_l++);
                rSum += *(src_ptr_r++) - *(src_ptr_l++);
                *(dst_ptr++) = bSum / len;
                *(dst_ptr++) = gSum / len;
                *(dst_ptr++) = rSum / len;
            }

            for(int i=info.biWidth-radius;i<info.biWidth;i++){
                bSum -= *(src_ptr_l++);
                gSum -= *(src_ptr_l++);
                rSum -= *(src_ptr_l++);
                *(dst_ptr++) = bSum / len;
                *(dst_ptr++) = gSum / len;
                *(dst_ptr++) = rSum / len;
            }
        }
    }
    
    if(col){
        if(row){
            dst = content;
            src = newContent;
        }
        // 逐列模糊
        for(int i=0;i<info.biWidth;i++){
            byte *src_ptr_l = &src[i*3];
            byte *src_ptr_r = &src[i*3];
            byte *dst_ptr = &dst[i*3];
            int rSum = 0,gSum = 0,bSum = 0;
            for(int v=0;v<=radius;v++){
                bSum += *(src_ptr_r);
                gSum += *(src_ptr_r+1);
                rSum += *(src_ptr_r+2);
                src_ptr_r+=stride;
            }
            *(dst_ptr) = bSum / len;
            *(dst_ptr+1) = gSum / len;
            *(dst_ptr+2) = rSum / len;
            dst_ptr+=stride;

            for(int j=1;j<=radius;j++){
                bSum += *(src_ptr_r);
                gSum += *(src_ptr_r+1);
                rSum += *(src_ptr_r+2);
                src_ptr_r+=stride;

                *(dst_ptr) = bSum / len;
                *(dst_ptr+1) = gSum / len;
                *(dst_ptr+2) = rSum / len;
                dst_ptr+=stride;
            }
            
            for(int j=radius+1;j<info.biHeight-radius;j++){
                bSum += *(src_ptr_r) - *(src_ptr_l);
                gSum += *(src_ptr_r+1) - *(src_ptr_l+1);
                rSum += *(src_ptr_r+2) - *(src_ptr_l+2);
                src_ptr_r+=stride; src_ptr_l+=stride;

                *(dst_ptr) = bSum / len;
                *(dst_ptr+1) = gSum / len;
                *(dst_ptr+2) = rSum / len;
                dst_ptr+=stride;
            }

            for(int i=info.biWidth-radius;i<info.biWidth;i++){
                bSum -= *(src_ptr_l);
                gSum -= *(src_ptr_l+1);
                rSum -= *(src_ptr_l+2);
                src_ptr_l+=stride;

                *(dst_ptr) = bSum / len;
                *(dst_ptr+1) = gSum / len;
                *(dst_ptr+2) = rSum / len;
                dst_ptr+=stride;
            }
        }
    }
    this->content = dst;
    delete src;
    // delete this->content;
    // this->content=newContent;
}

void BMP::MeanBlur1DimFlatPP(int radius){
    byte *newContent=new byte[this->info.biSizeImage]; //分配内存
    byte *dst = newContent;
    byte *src = content;
    int len = 2*radius + 1;
    if(row){
        // 逐行模糊
        for(int j=0;j<info.biHeight;j++){
            byte *src_ptr_l = &src[j*stride];
            byte *src_ptr_r = &src[j*stride];
            byte* dst_ptr = &dst[j*stride];

            int rSum = 0,gSum = 0,bSum = 0;
            for(int u=0;u<=radius;u++){
                bSum += *(src_ptr_r);
                gSum += *(src_ptr_r+1);
                rSum += *(src_ptr_r+2);
                src_ptr_r+=3;
            }
            *(dst_ptr) = bSum / len;
            *(dst_ptr+1) = gSum / len;
            *(dst_ptr+2) = rSum / len;
            dst_ptr+=3;

            for(int i=1;i<=radius;i++){
                bSum += *(src_ptr_r);
                gSum += *(src_ptr_r+1);
                rSum += *(src_ptr_r+2);
                src_ptr_r+=3;
                *(dst_ptr) = bSum / len;
                *(dst_ptr+1) = gSum / len;
                *(dst_ptr+2) = rSum / len;
                dst_ptr+=3;
            }
            
            for(int i=radius+1;i<info.biWidth-radius;i++){
                bSum += *(src_ptr_r) - *(src_ptr_l);
                gSum += *(src_ptr_r+1) - *(src_ptr_l+1);
                rSum += *(src_ptr_r+2) - *(src_ptr_l+2);
                src_ptr_r+=3;src_ptr_l+=3;
                *(dst_ptr) = bSum / len;
                *(dst_ptr+1) = gSum / len;
                *(dst_ptr+2) = rSum / len;
                dst_ptr+=3;
            }

            for(int i=info.biWidth-radius;i<info.biWidth;i++){
                bSum -= *(src_ptr_l);
                gSum -= *(src_ptr_l+1);
                rSum -= *(src_ptr_l+2);
                src_ptr_l+=3;
                *(dst_ptr) = bSum / len;
                *(dst_ptr+1) = gSum / len;
                *(dst_ptr+2) = rSum / len;
                dst_ptr+=3;
            }
        }
    }

    if(col){
        if(row){
            dst = content;
            src = newContent;
        }
        // 逐列模糊
        for(int i=0;i<info.biWidth;i++){
            byte *src_ptr_l = &src[i*3];
            byte *src_ptr_r = &src[i*3];
            byte *dst_ptr = &dst[i*3];
            int rSum = 0,gSum = 0,bSum = 0;
            for(int v=0;v<=radius;v++){
                bSum += *(src_ptr_r);
                gSum += *(src_ptr_r+1);
                rSum += *(src_ptr_r+2);
                src_ptr_r+=stride;
            }
            *(dst_ptr) = bSum / len;
            *(dst_ptr+1) = gSum / len;
            *(dst_ptr+2) = rSum / len;
            dst_ptr+=stride;

            for(int j=1;j<=radius;j++){
                bSum += *(src_ptr_r);
                gSum += *(src_ptr_r+1);
                rSum += *(src_ptr_r+2);
                src_ptr_r+=stride;

                *(dst_ptr) = bSum / len;
                *(dst_ptr+1) = gSum / len;
                *(dst_ptr+2) = rSum / len;
                dst_ptr+=stride;
            }
            
            for(int j=radius+1;j<info.biHeight-radius;j++){
                bSum += *(src_ptr_r) - *(src_ptr_l);
                gSum += *(src_ptr_r+1) - *(src_ptr_l+1);
                rSum += *(src_ptr_r+2) - *(src_ptr_l+2);
                src_ptr_r+=stride; src_ptr_l+=stride;

                *(dst_ptr) = bSum / len;
                *(dst_ptr+1) = gSum / len;
                *(dst_ptr+2) = rSum / len;
                dst_ptr+=stride;
            }

            for(int i=info.biWidth-radius;i<info.biWidth;i++){
                bSum -= *(src_ptr_l);
                gSum -= *(src_ptr_l+1);
                rSum -= *(src_ptr_l+2);
                src_ptr_l+=stride;

                *(dst_ptr) = bSum / len;
                *(dst_ptr+1) = gSum / len;
                *(dst_ptr+2) = rSum / len;
                dst_ptr+=stride;
            }
        }
    }
    this->content = dst;
    delete src;
}
