#pragma once


//
// Softcam Sender API
//

#define SOFTCAM_API __cdecl

extern "C"
{
    using scCamera = void*;

    using scBitmap = void*;

    /*
    创建摄像头
    @param index, 摄像头索引 0-3
    @param width，摄像头宽度
    @param height， 摄像头高度
    @param framerate, 帧率
    @return 摄像头句柄，调用其他接口时传入
    */
    scCamera SOFTCAM_API scCreateCamera(int index, int width, int height, float framerate = 60.0f);

    /*
    删除摄像头
    @param camera，摄像头句柄，scCreateCamera返回的值
    */
    void SOFTCAM_API scDeleteCamera(scCamera camera);

    /*
    等待应用使用摄像头
    @param camera，摄像头句柄，scCreateCamera返回的值
    @param timeout，等待超时，单位秒，0.0f表示永久等待
    @return true在超时前有应用连接，false在超时前没有应用连接
    */
    bool SOFTCAM_API scWaitForConnection(scCamera camera, float timeout = 0.0f);

    /*
    发送图片帧
    @param camera，摄像头句柄，scCreateCamera返回的值
    @param image_data，图片数据，BGRA格式
    @param length, image_data的长度，摄像头宽*摄像头高*4
    */
    void SOFTCAM_API scSendFrame(scCamera camera, const void* image_data, int length);

    /*
    发送图片帧，与scSendFrame相同，提供一个图片句柄HBITMAP作为参数
    @param camera，摄像头句柄，scCreateCamera返回的值
    @param bitmap, 图片句柄HBITMAP
    */
    void SOFTCAM_API scSendBitmapFrame(scCamera camera, scBitmap bitmap);    

    /*
    获取摄像头安装状态
    @param index, 摄像头索引 0-3
    @param bInstalled, 调用成功后有效，true表示摄像头已经安装, false表示摄像头未安装
    @return true调用成功, false调用失败
    */
    bool SOFTCAM_API scGetInstallationStatus(int index, bool& bInstalled);

    /*
    获取使用摄像头的应用列表
    @param index, 摄像头索引 0-3
    @param pids，存放使用摄像头的应用进程
    @param size, pids数组大小
    @return 使用摄像头的应用数，应用进程id存放在pids里面，返回值不会超过输入参数size
    */
    int SOFTCAM_API scGetCameraUsers(int index, unsigned int* pids, unsigned int size);
}
