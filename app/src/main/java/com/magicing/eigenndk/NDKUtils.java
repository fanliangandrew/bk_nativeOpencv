package com.magicing.eigenndk;

/**
 * Created by Administrator on 2017/2/10.
 */

public class NDKUtils {

   // public native String invokeCmethod();

    public static native int[] gray(int[] buf, int w, int h);
//    public static native int[] detectDriving(int[] buf,int w,int h);
    public static native ImageData detectDriving(int[] buf,int w,int h);
    public static native int testString(int[] buf,int w,int h);
    public static native ImageData preDetect(int[] buf,int w, int h);

    static {
        System.loadLibrary("OpenCV");//导入生成的链接库文件
    }

}
